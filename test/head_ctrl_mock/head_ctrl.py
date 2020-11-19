"""
    Simulate head controller.    
    Need a network.cfg configuration
"""
import asyncio, functools
import logging
import struct
import os
import xml.etree.ElementTree as ET

from message_mgr import Message

# only work with hack on IP: -localsubnet
SUBNET = "127.168.200."

def mac_addr(s):
    "transform a mac address string to an corresponding int"
    return int("".join(reversed(s.split("-"))), 16)

# all connections to close properly at the end
_transports = []

class UPDBoot:
    "rx_boot: receive data from main control to init the network"
    def __init__(self, heads):
        self.heads = heads
    def connection_made(self, transport):
        logging.info('Connection from UDP to boot')
        self.transport = transport
        _transports.append(transport)

    def connection_lost(self, exc):
        logging.error('Connection lost to boot')

    def datagram_received(self, data, addr):
        "receive the broadcast message for rx_boot"
        msg = Message(header="i")
        msg.unpack(data)
        logging.debug(f'Received message {msg.msgtype} of {len(data)} bytes on boot from addr {addr}')
        if msg.msgtype == "CMD_BOOT_INFO_REQ" or msg.msgtype == "CMD_BOOT_PING":
            # both messages respond the same struct
            reptype = msg.msgtype.replace("CMD", "REP").replace("_REQ","")
            # respond always for all boards
            for head in self.heads:
                no = int(head["DevNo"])
                msg = Message(reptype, "I")
                msg.deviceTypeStr = b"Head"
                msg.serialNo=head["SerialNo"].encode()
                # note that network.cfg should be well configured (macAddr) to allow registration
                msg.macAddr = mac_addr(head["MacAddr"])
                msg.deviceType = 10
                msg.deviceNo = no
                msg.ipAddr = head["ipaddress"].encode()
                msg.connected = 1 if head["ipaddress"] else 0
                msg.platform = 0
                msg.rfsPort = 7011
                msg.ports = (7011,0,0,0,0,0,0,0)
                self.transport.sendto(msg.pack(), (SUBNET + "1", 7004))
        elif msg.msgtype == "CMD_BOOT_ADDR_SET": # change the IP
            # retreive the head from its mac address
            head = [h for h in self.heads if mac_addr(h["MacAddr"]) ==  msg.macAddr][0]
            # setup the good ip address
            head["ipaddress"] = msg.ipAddr.split(b"\0", 1)[0].decode()
            logging.info(f"CMD_BOOT_ADDR_SET: board {head['DevNo']} change ip: {head['ipaddress']}")
        else:
            logging.error(f"Boot message unknown x{msg.msgtype:08x}")


class UDPProtocol:
    "fpga: receive UDP data"
    def __init__(self, board):
        self.board = board
        self.count = 0

    def connection_made(self, transport):
        logging.info(f'Connection from UDP to board {self.board.no}')
        self.transport = transport
        _transports.append(transport)

    def connection_lost(self, exc):
        logging.error(f'UDP Connection lost to board {self.board.no}')

    def datagram_received(self, data, addr):
        "receive UDP blocks"
        msg = struct.unpack("=I%ds" % (len(data) - 4), data)
        logging.debug(f"Received udp message {msg[0]} of size {len(data)}")
        if msg[0] != 0xffffffff:
            if len(msg[1]) == 1440:
                self.board.add_block(msg[0], msg[1])
            else:
                logging.error(f"UDP Bad block {msg[0]} size: {len(data)}")
            self.count += 1
            if self.count % 1000 == 0:
                logging.info(f"UDP {len(self.board.blocks)} blocks/{self.count} on board {self.board.no}")


class NewTCPProtocol:
    "TCP protocol factory to save board number"
    def __init__(self, board):
        self.board = board
    def __call__(self):
        return TCPProtocol(self.board)

class TCPProtocol:
    "rx_head_ctrl: manage messages"
    def __init__(self, board ):
        self.board = board
        self.transport = None

    def connection_made(self, transport):
        peername = transport.get_extra_info('peername')
        logging.info(f'Connection from {peername} to board {self.board.no}')
        if self.transport:
            raise Exception("already connected")
        self.transport = transport
        _transports.append(transport)


    def connection_lost(self, exc):
        logging.error(f'Connection lost to board {self.board.no}')

    def data_received(self, data):
        "manage data and call the mgt_XXX of the corresponding message"
        while len(data):
            msg = Message()        
            data = msg.unpack(data)
            logging.debug(f"Receive message {msg.msgtype}")
            if isinstance(msg.msgtype, str):
                message_manager = "mgt_"+msg.msgtype
                if hasattr(self, message_manager):
                    getattr(self, message_manager)(msg)
                else:
                    logging.debug(f'Ignore msg {msg.msgtype} ({msg.lenght} bytes) on TCP for board {self.board.no}')
            else:
                logging.error(f'Unknown msg x{msg.msgtype:06x} ({msg.lenght} bytes) on TCP for board {self.board.no}')

    def mgt_CMD_PING(self, msg):
        self.transport.write(Message("REP_PING").pack())

    def mgt_CMD_HEAD_BOARD_CFG(self, msg):
        # open socket
        self.board.connect_main(self.transport)
        if self.board.abort:
            self.board.reset()
        # active heads are "dev_on" (2)
        self.board.activate = [x['enabled']==2 for x in msg.head]
        # save blocks position for the board
        self.board.blk_end = [x["blkNo0"]+x["blkCnt"] for x in msg.head]
        self.board.blkNo0 = [x["blkNo0"] for x in msg.head]
        # send back the ResetCnt
        msg.msgtype = "REP_HEAD_BOARD_CFG"
        self.transport.write(msg.pack())
        logging.info(f"Reset counter {msg.resetCnt} for board {self.board.no}")

    def mgt_CMD_HEAD_STAT(self, msg):
        msg = Message("REP_HEAD_STAT")
        msg.boardNo = self.board.no
        msg.headCnt = 4
        msg.head = ({}, {}, {}, {})
        for i in range(4):
                msg.head[i]["ctrlMode"] = 0x006 # ctrl_readyToPrint
                msg.head[i]["clusterNo"] = self.board.no
                msg.head[i]["disabledJets"] = 32 * (-1, )
                #msg.head[i]["densityValue"] = 12 * [1000,]
                #msg.head[i]["densityValue"][3*i] = 10 
        self.transport.write(msg.pack())

    def mgt_CMD_GET_BLOCK_USED(self, msg):
        # send response
        msg.msgtype = "REP_GET_BLOCK_USED"
        msg.blockOutIdx = self.board.blk_end[msg.headNo]
        msg.blkCnt = ((msg.blkCnt + 31) // 32) * 32
        logging.debug(f"{self.board.used.count} blocks used {msg.blkNo} for {msg.blkCnt} blocks on board {self.board.no} head {msg.headNo}")
        msg.used = self.board.used.flags(msg.blkNo, msg.blkCnt, self.board.blk_end[msg.headNo], self.board.blkNo0[msg.headNo]) 
        self.transport.write(msg.pack())

    def mgt_CMD_FPGA_IMAGE(self, msg):
        logging.info(f"FPGA image on board {self.board.no} head {msg.head} Blk0 {msg.blkNo} BlkCnt {msg.blkCnt} (id:{msg.id} copy:{msg.copy} page:{msg.page} scan:{msg.scan})")
        self.transport.write(Message("REP_FPGA_IMAGE").pack())
        self.board.image(msg)

    def mgt_CMD_PRINT_ABORT(self, msg):
        logging.info(f"Abort print board {self.board.no}")
        self.board.abort = True

class UsedFlags(list):
    "table of bits to manage blocks used"
    def __init__(self):
        self.empty()
    def empty(self):
        "set all blocks as free"
        self._flags = []
        self.count = 0
    def _spare(self, no):
        "increase the list size according to last block used"
        missing = no // 32 - len(self._flags) + 1
        if missing > 0:
            self._flags.extend(missing * [0])
    def free(self, blkNo, blkCnt,  end, blk0):
        "free one blocks"
        if blkNo + blkCnt < end:
            self._flags[blkNo//32:blkNo//32+blkCnt//32] = blkCnt//32 * (0,)
        else:
            self._flags[blkNo//32:end//32] = (end//32 - blkNo//32) * (0,)
            self._flags[blk0//32:(blkNo + blkCnt - end)//32] = (blkNo//32 + blkCnt//32 - end//32)  * (0,)

    def use(self, no):
        "set a block as used"
        self._spare(no)
        self._flags[no // 32] |= (1 << (no % 32))
    def is_used(self, no):
        "check if a block is used"
        self._spare(no)
        return self._flags[no // 32] & (1 << (no % 32))
    def flags(self, blkNo, blkCnt, end, blk0):
        "return the table of bits according to blocks"
        if blkNo + blkCnt < end:
            self._spare(blkNo + blkCnt)
            return self._flags[blkNo//32:blkNo//32+blkCnt//32]
        else:
            self._spare(end)
            return self._flags[blkNo//32:end//32]+self._flags[blk0//32:(blkNo + blkCnt - end)//32]


def save_image(no, blocks, fpga_images, blk_end, blk_No0):
    "save all images printed by a board"
    for fpga in fpga_images:
        filename = f"printed/fake id{fpga.id} c{fpga.copy} p{fpga.page} s{fpga.scan} h{fpga.head} b{no}.bmp"
        logging.info(f"Creating '{filename}' ({fpga.widthPx}x{fpga.lengthPx})...")
        try:
            img = bytearray()
            end = blk_end[fpga.head]
            b0 = blk_No0[fpga.head]
            for i in range(fpga.blkNo,fpga.blkNo+fpga.blkCnt):
                if i < end: 
                    img.extend(blocks[i])
                else:
                    img.extend(blocks[b0+i-end])
            from PIL import Image
            # the width is align to 256 bits
            align = 256 // fpga.bitPerPixel # so it depends on the bit per pixel
            width = align * ((fpga.widthPx + align - 1) // align)
            # save the image with the good mode depending on bit per pixel and "Inverted" (white is 0x00)
            if fpga.bitPerPixel == 1:
                bmp = Image.frombytes("1", (width, fpga.lengthPx), bytes(img), "raw", "1;I")
            else:
                bmp = Image.frombytes("L", (width, fpga.lengthPx), bytes(img), "raw", f"L;{fpga.bitPerPixel}I")
            # if scan backward, mirror the image
            if fpga.flags & 1:
                bmp = bmp.transpose(Image.FLIP_TOP_BOTTOM)
            if fpga.jetPx0 or width != fpga.widthPx: # remove alignement to retreive printed image
                start = 0
                if fpga.jetPx0:
                    logging.info(f"'{filename}' has jetPx0={fpga.jetPx0}")
                    start = 8 // fpga.bitPerPixel - fpga.jetPx0
                bmp = bmp.crop((start, 0, fpga.widthPx , fpga.lengthPx))
            bmp.save(filename)
        except Exception as e:
            logging.error(f"{e.__class__.__name__}: {e} while saving '{filename}'")


class Board:
    "board information regarding fpga and state"
    def __init__(self, no):
        self.no = no
        self.activate = 4 * [False]
        self.reset()

    def add_block(self, no, block):
        "add a received block to the board"
        if not self.abort:
            self.blocks[no] = block
            self.used.use(no)

    def connect_main(self, transport):
        "set the connection socket to main ctrl (to send EVT_PRINT_DONE)"
        self.main_ctrl = transport

    def image(self, msg):
        "received a FPG_IMAGE message"
        pr = (msg.id, msg.page, msg.copy, msg.scan)
        self.fpga_images.setdefault(pr, [])
        self.fpga_images[pr].append(msg)

    def print_done(self, msg):
        "finish the print by clearing blocks and sending EVT_PRINT_DONE"
        for head in self.fpga_images[(msg.id, msg.page, msg.copy, msg.scan)]:
            if head.clearBlockUsed:
                self.used.free(head.blkNo, head.blkCnt, self.blk_end[head.head], self.blkNo0[head.head])
        # reuse the same data from the message
        self.pd += 1
        msg.pd = self.pd
        msg.boardNo = 0xff
        # to send a print done to main ctrl
        msg.msgtype = "EVT_PRINT_DONE"
        self.main_ctrl.write(msg.pack())
        del self.fpga_images[(msg.id, msg.page, msg.copy, msg.scan)]
        logging.warning(f"Done on board {self.no} for id{msg.id} s{msg.scan} c{msg.copy}")

    def reset(self):
        "reset the board by clearing everythings"
        logging.warning(f"reset board {self.no}")
        self.abort = False
        self.blocks = {}
        self.fpga_images = {}
        self.used = UsedFlags()
        self.pd = 0

# to end the infinite loop
simulate = True
async def main():
    "main program that listen to all UDP/TCP sockets according to network.cfg"
    os.makedirs("printed", exist_ok=True)
    loop = asyncio.get_running_loop()
    boards = []
    # start reading network.cfg to know what boards to simulate
    tree = ET.parse(r'D:\radex\user\network.cfg')
    root = tree.getroot()
    heads = [x.attrib for x in root.findall("item") if x.attrib['DevTypeStr'] == 'Head']
    # set a empty (bad) IP to receive a CMD_BOOT_ADDR_SET
    for head in heads:
        head["ipaddress"] = "" 

    tcp_port = 7011
    udp_port = 7012

    # read broadcast boot info
    await loop.create_datagram_endpoint(
            lambda: UPDBoot(heads),
            local_addr=(SUBNET+"255", 7005))

    # wait for board to "boot" to ensure we get the good ip
    while simulate and not all([head["ipaddress"] for head in heads]):
        await asyncio.sleep(1)

    if simulate:
        logging.info("all boards booted...")

        # open sockets for UDP/TCP of each board
        for head in heads:
            no = int(head["DevNo"])
            board = Board(no)
            boards.append(board)
            tcp_ip = head["ipaddress"]

            _transports.append(await loop.create_server(
                    NewTCPProtocol(board),
                    tcp_ip, tcp_port))
            logging.info(f"start {tcp_ip}:{tcp_port} for board {no}")

            udp_ip = SUBNET + str(50 + no)
            await loop.create_datagram_endpoint(
                lambda: UDPProtocol(board),
                local_addr=(udp_ip, udp_port))

        while simulate:
            await asyncio.sleep(3)
            # check the end of the printing
            for board in boards:
                for img in list(board.fpga_images.values()):
                    if len(img) == len([x for x in board.activate if x]) and not board.abort:
                        # if done, save bmp in another thread as it could be a long process
                        fn = functools.partial(save_image, *(board.no, board.blocks, img, board.blk_end, board.blkNo0))
                        loop.run_in_executor(None, fn)                            
                        board.print_done(img[0])
    

def run():
    "run a head ctrl asynchronously by calling main in asyncio loop"
    global _transports, simulate
    simulate = True
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    try:
        loop.run_until_complete(main())
    finally:
        for transport in _transports:
            transport.close()
        _transports= []
        loop.run_until_complete(loop.shutdown_asyncgens())
        loop.close()
    

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--log", help='log level (default INFO)', default="INFO", choices=['DEBUG', 'INFO', 'WARN', 'ERROR'])
    parser.add_argument("--src", help='root source folder where to find header')
    args = parser.parse_args()
    numeric_level = getattr(logging, args.log.upper(), None)

    if args.src:
        Message.src_path = args.src

    if not isinstance(numeric_level, int):
        raise ValueError('Invalid log level: %s' % args.log)
    logging.basicConfig(format='%(asctime)s %(levelname)s:%(message)s',
                        level=numeric_level)

    try:
        run()
    except KeyboardInterrupt:
        # ignore Ctrl+C silently
        pass

