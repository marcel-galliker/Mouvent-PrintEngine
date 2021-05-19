"""
    Simulate head controller.    
    Need a network.cfg configuration
"""
import asyncio, functools
import logging
import struct
import os
import sys

import network

import message_mgr
Message = message_mgr.Message
# all messages we can convert and their corresponding C struct from .h file
message_mgr.msgtypes.update({
    "CMD_HEAD_STAT": None, # ignore
    "SET_GET_INK_DEF": None,
    "CMD_HEAD_FLUID_CTRL_MODE": None,

    "REP_HEAD_STAT":"SHeadBoardStat",
    "CMD_SET_DENSITY_VAL": "SDensityValuesMsg",
    "CMD_SET_DISABLED_JETS": "SDisabledJetsMsg",
    "CMD_PING": "",
    "REP_PING": "",
    "CMD_FPGA_IMAGE": "SFpgaImageCmd",
    "REP_FPGA_IMAGE": "",
    "REP_HEAD_BOARD_CFG":
        "UINT32		resetCnt;",
    "CMD_HEAD_BOARD_CFG": "SHeadBoardCfg",
    "CMD_GET_BLOCK_USED": "SBlockUsedCmd",
    "REP_GET_BLOCK_USED": # special variable struct
    """
        UINT32		aliveCnt[2];
        UINT32		blockOutIdx;	// number of the last block used
        UINT32		blkNo;			// BYTE-index of first used bits
        UINT8		headNo;
        UINT8		id;
        UINT16		blkCnt;				// number of bytes
        UINT32		used[*];// maximum size
    """,
    "EVT_PRINT_DONE": "SPrintDoneMsg",
    "CMD_PRINT_ABORT": "",
})

def crc8(data, lenght = 2):
    "crc on array of short int as in rx_crc.c" 
    crc = 0xFF
    for v in data:
        if v<0:
            v = v + 2**(lenght*8)
        for b in reversed(v.to_bytes(lenght, "big")):
            crc ^= b
            for _ in range(8): 
                if (crc & 0x80)!=0:
                    crc <<= 1
                    crc ^= 0x1D
                else:
                    crc <<= 1
    return (~crc)&0xFF


class UDPProtocol:
    "fpga: receive UDP data"
    def __init__(self, board):
        self.board = board
        self.count = 0

    def connection_made(self, transport):
        logging.info(f'Connection from UDP to board {self.board.no}')
        self.transport = transport
        network.all_transports.append(transport)

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


class TCPProtocol(network.AbstractTCPProtocol):
    "rx_head_ctrl: manage messages"
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

    def mgt_CMD_SET_DENSITY_VAL(self, msg):
        logging.info(f"set density on board {self.board.no} head {msg.head % 4}")
        self.board.config["density"][msg.head % 4] = msg.value
        self.board.config["voltage"][msg.head % 4] = msg.voltage
        logging.info(f"{msg.voltage}V {msg.value}")

    def mgt_CMD_SET_DISABLED_JETS(self, msg):
        logging.info(f"set disabled jets on board {self.board.no} head {msg.head % 4}")
        self.board.config["disabled_jets"][msg.head % 4] = msg.disabledJets
        logging.info(f"{msg.disabledJets}")

    def mgt_CMD_HEAD_STAT(self, msg):
        msg = Message("REP_HEAD_STAT")
        msg.boardNo = self.board.no
        msg.headCnt = 4
        msg.head = ({}, {}, {}, {})
        for i in range(4):
                msg.head[i]["ctrlMode"] = 0x006 # ctrl_readyToPrint
                msg.head[i]["clusterNo"] = self.board.no + i + 1
                if self.board.config["disabled_jets"][i] is not None:
                    msg.head[i]["disabledJets"] = self.board.config["disabled_jets"][i]
                    msg.head[i]["disabledJetsCRC"] = crc8(self.board.config["disabled_jets"][i])
                if self.board.config["voltage"][i] is not None:
                    msg.head[i]["densityValue"] = self.board.config["density"][i]
                    msg.head[i]["densityValueCRC"] = crc8(self.board.config["density"][i])
                    msg.head[i]["voltage"] = self.board.config["voltage"][i]
                    msg.head[i]["voltageCRC"] = crc8((self.board.config["voltage"][i],),1) # unsigned char
        self.transport.write(msg.pack())

    def mgt_CMD_GET_BLOCK_USED(self, msg):
        # send response
        msg.msgtype = "REP_GET_BLOCK_USED"
        msg.blockOutIdx = self.board.blockOutIdx[msg.headNo] 
        if rec_blocks:        
            msg.used = self.board.used.flags(msg.blkNo, msg.blkCnt, self.board.blk_end[msg.headNo], self.board.blkNo0[msg.headNo]) 
        else:
            msg.used = (msg.blkCnt+31)//32 * (0xffffffff, )
        msg.blkCnt = len(msg.used) * 32
        logging.debug(f"{self.board.used.count} blocks used {msg.blkNo} for {msg.blkCnt} blocks on board {self.board.no} head {msg.headNo}")
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
    def free(self, no):
        "free one blocks"
        self._flags[no // 32] &= ~(1 << (no % 32))
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
        # note: no need to round up as all blk cnt are already rounded by spooler!
        if blkNo + blkCnt < end:
            self._spare(blkNo + blkCnt)
            return self._flags[blkNo//32:(blkNo+blkCnt)//32]
        else:
            self._spare(end)
            return self._flags[blkNo//32:end//32]+self._flags[blk0//32:(blkNo + blkCnt - end)//32]


def save_image(board, fpga_images):
    "save all images printed by a board"
    no = board.no
    blocks = board.blocks
    blk_end = board.blk_end
    blk_No0 = board.blkNo0
    for fpga in fpga_images:
        filename = f"printed/fake id{fpga.id} c{fpga.copy} p{fpga.page} s{fpga.scan} b{no} h{fpga.head}.bmp"
        logging.info(f"Creating '{filename}' ({fpga.widthPx}x{fpga.lengthPx})...")
        try:
            img = bytearray()
            end = blk_end[fpga.head]
            b0 = blk_No0[fpga.head]
            for i in range(fpga.blkNo,fpga.blkNo+fpga.blkCnt):
                n0 =  i if i < end else b0+i-end
                img.extend(blocks[n0])
                if fpga.clearBlockUsed:
                    del blocks[n0]
                    board.used.free(n0)
            board.blockOutIdx[fpga.head] = fpga.blkNo+fpga.blkCnt
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
                    start = fpga.jetPx0
                bmp = bmp.crop((start, 0, fpga.widthPx , fpga.lengthPx))
            bmp.save(filename)
        except Exception as e:
            logging.error(f"{e.__class__.__name__}: {e} while saving '{filename}'")


class Board(network.AbstractBoard):
    "board information regarding fpga and state"
    def __init__(self, d):
        super().__init__(d)
        self.activate = 4 * [False]
        self.reset()
        self.config = {"disabled_jets": 4 * [None], "density": 4 * [None], "voltage": 4 * [None]}

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
        self.blockOutIdx = 4 * [2**32-1]
        self.blocks = {}
        self.fpga_images = {}
        self.used = UsedFlags()
        self.pd = 0

async def startup(loop, board):
    "startup board opening UDP port"
    os.makedirs("printed", exist_ok=True)
    udp_port = 7012
    udp_ip = network.SUBNET + str(50 + board.no)
    await loop.create_datagram_endpoint(
        lambda: UDPProtocol(board),
        local_addr=(udp_ip, udp_port))


rec_blocks = True
async def process(loop, boards):
    "process periodically all head and save image if needed"
    global rec_blocks
    await asyncio.sleep(0.5) # simulate time to print each copy
    # check the end of the printing
    for board in boards:
        if board.fpga_images:
            img=board.fpga_images[sorted(board.fpga_images.keys())[0]]
            if len(img) == len([x for x in board.activate if x]) and not board.abort:
                # if done, async save bmp as it could be a long process
                if rec_blocks:
                    fn = functools.partial(save_image, *(board, img))
                    await loop.run_in_executor(None, fn)
                board.print_done(img[0])
    

simulation = None
def run():
    "run only head mock in simulation"
    global simulation
    simulation = network.Network()
    simulation.register("Head", sys.modules[__name__])
    simulation.run()


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--log", help='log level (default INFO)', default="INFO", choices=['DEBUG', 'INFO', 'WARN', 'ERROR'])
    parser.add_argument("--src", help='root source folder where to find header')
    parser.add_argument("--nobmp", help='do not save bmp in printed folder', action="store_false")
    args = parser.parse_args()
    numeric_level = getattr(logging, args.log.upper(), None)

    if args.src:
        Message.src_path = args.src

    if not isinstance(numeric_level, int):
        raise ValueError('Invalid log level: %s' % args.log)
    logging.basicConfig(format='%(asctime)s %(levelname)s:%(message)s',
                        level=numeric_level)

    rec_blocks = args.nobmp
    try:
        run()
    except KeyboardInterrupt:
        # ignore Ctrl+C silently
        pass

