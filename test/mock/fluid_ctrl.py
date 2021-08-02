"""
    Simulate fluid controller.    
    Need a network.cfg configuration
"""
import asyncio
import logging

import network
import message_mgr
Message = message_mgr.Message
# all messages we can convert and their corresponding C struct from .h file
message_mgr.msgtypes.update({
    "CMD_FLUID_IS_CFG": "SInkSupplyCfg",
	"CMD_FLUID_CFG": "SFluidBoardCfg",									
	"CMD_FLUID_STAT": None,
	"CMD_FLUID_CTRL_MODE": "SFluidCtrlCmd",
    "CMD_FLUID_DEGASSER": "int degas;",
	"CMD_SET_PURGE_PAR": "SPurgePar",
	"CMD_SCALES_SET_CFG": "SScalesCfgMsg",
	"CMD_SCALES_GET_CFG": "",
	"CMD_SCALES_TARA": "INT32 no;",	
	"CMD_SCALES_CALIBRATE": "SValue",	
	"CMD_SCALES_STAT": "",	
    "REP_SCALES_GET_CFG": "SScalesCfgMsg",
    "REP_SCALES_STAT": "SScalesStatMsg",
    "REP_FLUID_STAT": "SFluidBoardStat",

})


class TCPProtocol(network.AbstractTCPProtocol):
    "rx_fluid_ctrl: manage messages"
    def mgt_CMD_ERROR_RESET(self, msg):
        self.board.reset()

    def mgt_CMD_FLUID_IS_CFG(self, msg):
        logging.info("CMD_FLUID_IS_CFG")
        # nois_set_is_cfg((SInkSupplyCfg",
        
    def mgt_CMD_FLUID_CFG(self, msg):
        logging.info("CMD_FLUID_CFG")
                                                
    def mgt_CMD_FLUID_STAT(self, msg):
        msg = Message("REP_FLUID_STAT")
        msg.data(self.board.config["REP_FLUID_STAT"])
        self.transport.write(msg.pack())

        if "REP_SCALES_STAT" in self.board.config:
            msg = Message("REP_SCALES_STAT")
            msg.data(self.board.config["REP_SCALES_STAT"])
            self.transport.write(msg.pack())

        
    def mgt_CMD_FLUID_CTRL_MODE(self, msg):
        logging.info(f"CMD_FLUID_CTRL_MODE {msg.ctrlMode}")
        ctrl_conv = {
            message_mgr.enums.get("ctrl_shutdown"): message_mgr.enums["ctrl_off"],
            message_mgr.enums.get("ctrl_prepareToPrint"): message_mgr.enums["ctrl_readyToPrint"],
        }
        self.board.config["REP_FLUID_STAT"]["stat"][msg.no]["ctrlMode"] = ctrl_conv.get(msg.ctrlMode, msg.ctrlMode)
        #_do_fluid_ctrlMode(socket, (SFluidCtrlCmd*)msg);
        
    def mgt_CMD_FLUID_DEGASSER(self, msg):
        logging.info("CMD_FLUID_DEGASSER")
        #nios_set_degasser((int",

    def mgt_CMD_SET_PURGE_PAR(self, msg):
        logging.info("CMD_SET_PURGE_PAR")
        #_do_set_purge_par(socket, (SPurgePar",

    def mgt_CMD_SCALES_SET_CFG(self, msg):
        logging.info("CMD_SCALES_SET_CFG")
        #_do_scales_set_cfg(socket, (SScalesCfgMsg*)msg);

    def mgt_CMD_SCALES_GET_CFG(self, msg):
        logging.info("CMD_SCALES_GET_CFG")
        #_do_scales_get_cfg(socket);

    def mgt_CMD_SCALES_TARA(self, msg):
        logging.info("CMD_SCALES_TARA")
        #_do_scales_tara(socket, (INT32",

    def mgt_CMD_SCALES_CALIBRATE(self, msg):
        logging.info("CMD_SCALES_CALIBRATE")
        # _do_scales_calib(socket, (SValue",

    def mgt_CMD_SCALES_STAT(self, msg):
        msg = Message("REP_SCALES_STAT")
        msg.val = 18 * [1000, ]
        self.transport.write(msg.pack())

class Board(network.AbstractBoard):
    "board information regarding fpga and state"
    def __init__(self, d):
        super().__init__(d)
        self.error = 0
        self.config = {}
        self.config["REP_FLUID_STAT"] = Message("REP_FLUID_STAT").empty() 
        if self.no==0:
            self.config["REP_SCALES_STAT"] = Message("REP_SCALES_STAT").empty()
            self.config["REP_SCALES_STAT"]["val"] = 18*[10000,]

        for i in range(4):
            self.config["REP_FLUID_STAT"]["stat"][i]["canisterLevel"] = 10000
            self.config["REP_FLUID_STAT"]["stat"][i]["ctrlMode"] = message_mgr.enums["ctrl_readyToPrint"]
            self.config["REP_FLUID_STAT"]["stat"][i]["condTemp"] = 30000
            self.config["REP_FLUID_STAT"]["stat"][i]["temp"] = 45000

    def reset(self):
        "reset the board by clearing everythings"
        self.error = 0


async def startup(loop, board):
    "startup a boad"
    pass

async def process(loop, boards):
    "process all boards periodically"
    await asyncio.sleep(0.5)

if __name__ == "__main__":
    import argparse, sys
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

    simulation = network.Network()
    simulation.register("Fluid", sys.modules[__name__])
    try:
        simulation.run()
    except KeyboardInterrupt:
        # ignore Ctrl+C silently
        pass

