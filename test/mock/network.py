"""
    Network simualation by reading network.cgg
    All board to simulate should be register do a Network instance before run
"""
import logging
logger = logging.getLogger(__name__)
import asyncio
import xml.etree.ElementTree as ET
import message_mgr
Message = message_mgr.Message
# messages for boot
message_mgr.msgtypes.update({
    "CMD_RFS_START_PROCESS": None,
    "CMD_RFS_KILL_PROCESS": None,
    "CMD_RFS_MAKE_DIR" : None,
    "CMD_ERROR_RESET" : None,

    "CMD_BOOT_PING": "",
    "CMD_BOOT_INFO_REQ": "",
    "REP_BOOT_INFO":"SNetworkItem",
    "REP_BOOT_PING": "", # will be overwritten by REP_BOOT_INFO
    "CMD_BOOT_ADDR_SET":"SBootAddrSetCmd",
})

SUBNET = "127.168.200."
all_transports = []

tcp_port = {
    "Head": 7011,
    "Encoder": 7013,
    'Fluid': 7014,
    "Stepper": 7015,
}
def mac_addr(s):
    "transform a mac address string to an corresponding int"
    return int("".join(reversed(s.split("-"))), 16)

class UPDBoot:
    "rx_boot: receive data from main control to init the network"
    def __init__(self, boards):
        self.boards = boards
    def connection_made(self, transport):
        logger.info('Connection from UDP to boot')
        self.transport = transport
        all_transports.append(transport)

    def connection_lost(self, exc):
        logger.error('Connection lost to boot')

    def datagram_received(self, data, addr):
        "receive the broadcast message for rx_boot"
        msg = Message(header="i")
        msg.unpack(data)
        logger.debug(f'Received message {msg.msgtype} of {len(data)} bytes on boot from addr {addr}')
        if msg.msgtype == "CMD_BOOT_INFO_REQ" or msg.msgtype == "CMD_BOOT_PING":
            # both messages respond the same struct
            reptype = msg.msgtype.replace("CMD", "REP").replace("_REQ","")
            # respond always for all boards
            for board in self.boards:
                no = int(board["DevNo"])
                msg = Message(reptype, "I")
                msg.deviceTypeStr = board["DevTypeStr"].encode()
                msg.serialNo=board["SerialNo"].encode()
                # note that network.cfg should be well configured (macAddr) to allow registration
                msg.macAddr = mac_addr(board["MacAddr"])
                msg.deviceType = 9
                msg.deviceNo = no
                msg.ipAddr = board["ipaddress"].encode()
                msg.connected = 1 if board["ipaddress"] else 0
                msg.platform = 0
                msg.rfsPort = board["port"]
                msg.ports = (board["port"],0,0,0,0,0,0,0)
                self.transport.sendto(msg.pack(), (SUBNET + "1", 7004))
        elif msg.msgtype == "CMD_BOOT_ADDR_SET": # change the IP
            # retreive the head from its mac address
            board = [h for h in self.boards if mac_addr(h["MacAddr"]) ==  msg.macAddr][0]
            # setup the good ip address
            board["ipaddress"] = msg.ipAddr.split("\0", 1)[0] 
            logger.info(f"CMD_BOOT_ADDR_SET: board {board['DevTypeStr']} {board['DevNo']} change ip: {board['ipaddress']}")
        else:
            logger.error(f"Boot message unknown x{msg.msgtype:08x}")

class FactoryTCPProtocol:
    "TCP protocol factory to save board number"
    def __init__(self, board, protocolClass):
        self.board = board
        self.protocolClass = protocolClass
    def __call__(self):
        return self.protocolClass(self.board)

class AbstractTCPProtocol:
    "manage messages"
    def __init__(self, board):
        self.board = board
        self.transport = None

    def connection_made(self, transport):
        peername = transport.get_extra_info('peername')
        logger.info(f'Connection from {peername} to board {self.board.type}:{self.board.no}')
        if self.transport:
            raise Exception("already connected")
        self.transport = transport
        all_transports.append(transport)


    def connection_lost(self, exc):
        logger.error(f'Connection lost to board {self.board.type}:{self.board.no}')

    def data_received(self, data):
        "manage data and call the mgt_XXX of the corresponding message"
        while len(data):
            msg = Message()        
            data = msg.unpack(data)
            logger.debug(f"Receive message {msg.msgtype}")
            if isinstance(msg.msgtype, str):
                message_manager = "mgt_"+msg.msgtype
                if hasattr(self, message_manager):
                    getattr(self, message_manager)(msg)
                else:
                    logger.debug(f'Ignore msg {msg.msgtype} ({msg.lenght} bytes) on TCP for board {self.board.type}:{self.board.no}')
            else:
                logger.error(f'Unknown msg x{msg.msgtype:06x} ({msg.lenght} bytes) on TCP for board {self.board.type}:{self.board.no}')

class AbstractBoard(dict):
    "Base class for board (use to read network.cfg, should inherit from dict)"
    def __init__(self, d):
        super().__init__(d)
        self.no = int(d["DevNo"])
        self.type = d["DevTypeStr"]
        self.transport = None
        self.config = {} # rest api config of the board


class Network:
    def __init__(self):
        self.simulate = True # to end the infinite loop
        self.boards = {}
        self.protocols = {}
        self.app = None

    async def boot(self, boards):
        "wait for boot of given boards"
        loop = asyncio.get_running_loop()
        # set a empty (bad) IP to receive a CMD_BOOT_ADDR_SET
        for board in boards:
            board["ipaddress"] = ""
            board["port"] = tcp_port[board["DevTypeStr"]]

        # read broadcast boot info
        await loop.create_datagram_endpoint(
                lambda: UPDBoot(boards),
                local_addr=(SUBNET+"255", 7005))

        # wait for board to "boot" to ensure we get the good ip
        while self.simulate and not all([board["ipaddress"] for board in boards]):
            await asyncio.sleep(1)
        logger.info("all boards booted...")

    def register(self, dev, protocol):
        "register a module for a device, module should have TCPProtocol, Board, startup and process"
        self.protocols[dev] = protocol
        self.boards[dev] = []

    async def start_boards(self):
        "main program that listen to all UDP/TCP sockets according to network.cfg"
        loop = asyncio.get_running_loop()
        # start reading network.cfg to know what boards to simulate
        tree = ET.parse(r'D:\radex\user\network.cfg')
        root = tree.getroot()
        devs = [x.attrib for x in root.findall("item") if x.attrib['DevTypeStr'] in self.protocols.keys()]
        if not devs:
            raise ValueError(f"no {', '.join(self.protocols.keys())} in network.cfg")
        devs.sort(key=lambda d:int(d["DevNo"])) # sort by dev no to ensure the first item in list is DevNo=0

        # create all the boards
        allboards = []
        for d in devs:
            dev = d["DevTypeStr"]
            no = int(d["DevNo"])
            board = self.protocols[dev].Board(d)
            self.boards[dev].append(board)
            allboards.append(board)

        # create the web server for boards
        import rest
        self.app = rest.create_app(self.boards)
        self.handler = self.app.make_handler()
        server = loop.create_server(self.handler, '0.0.0.0', 8080)
        (self.srv, _) = await asyncio.gather(server, self.app.startup())

        # wait for boot message from main to open socket on the good ip address
        await self.boot(allboards)

        if self.simulate:
            # open sockets for UDP/TCP of each board
            for board in allboards:
                tcp_ip = board["ipaddress"]
                dev = board["DevTypeStr"]
                all_transports.append(await loop.create_server(
                        FactoryTCPProtocol(board,self.protocols[dev].TCPProtocol),
                        tcp_ip, tcp_port[dev]))
                logger.info(f"start {tcp_ip}:{tcp_port[dev]} for board {dev} {board.no}")

                await self.protocols[dev].startup(loop, board)


            while self.simulate:
                for dev in self.protocols.keys():
                    await self.protocols[dev].process(loop, self.boards[dev]) 
    
    def run(self):
        "run ctrl asynchronously by calling start_boards in asyncio loop"
        global all_transports
        self.simulate = True
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)

        try:
            loop.run_until_complete(self.start_boards())
        finally:
            # clean exit
            for transport in all_transports:
                transport.close()
            all_transports= []
            # for http server
            if self.app:
                self.srv.close()
                loop.run_until_complete(self.srv.wait_closed())
                loop.run_until_complete(self.app.shutdown())
                loop.run_until_complete(self.handler.shutdown())
                loop.run_until_complete(self.app.cleanup())

            loop.run_until_complete(loop.shutdown_asyncgens())
            loop.close()