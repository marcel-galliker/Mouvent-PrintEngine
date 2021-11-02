"""
    Simulate stepper    
    Need a network.cfg configuration
"""
import asyncio
import logging
logger = logging.getLogger("stepper")

import network
import message_mgr
Message = message_mgr.Message
# all messages we can convert and their corresponding C struct from .h file
message_mgr.msgtypes.update({
    "CMD_LIFT_STOP": "",
    "CMD_LIFT_REFERENCE": "",
    "CMD_LIFT_CALIBRATE": "",
    "CMD_LIFT_PRINT_POS": "",
    "CMD_LIFT_UP_POS": "",
    "CMD_LIFT_CAPPING_POS": "",
    "CMD_ERROR_RESET": "",
    "CMD_LIFT_VENT": None,

    "CMD_STEPPER_CFG": "SStepperCfg",
    "CMD_STEPPER_STAT": "",
    "REP_STEPPER_STAT": "SStepperStat",

})


class TCPProtocol(network.AbstractTCPProtocol):
    "rx_stepper_ctrl: manage messages"
    def mgt_CMD_ERROR_RESET(self, msg):
        self.board.reset()       
    def mgt_CMD_LIFT_STOP(self, msg):
        logger.debug("CMD_LIFT_STOP}")
    def mgt_CMD_LIFT_REFERENCE(self, msg):
        logger.debug("CMD_LIFT_REFERENCE")
    def mgt_CMD_LIFT_CALIBRATE(self, msg):
        logger.debug("CMD_LIFT_CALIBRATE")
    def mgt_CMD_LIFT_PRINT_POS(self, msg):
        logger.debug("CMD_LIFT_PRINT_POS")
    def mgt_CMD_LIFT_UP_POS(self, msg):
        logger.debug("CMD_LIFT_UP_POS")
    def mgt_CMD_LIFT_CAPPING_POS(self, msg):
        logger.debug("CMD_LIFT_CAPPING_POS")
    def mgt_CMD_STEPPER_STAT(self, msg):
        logger.debug("CMD_STEPPER_STAT")
    def mgt_CMD_STEPPER_CFG(self, msg):
        logger.debug("CMD_STEPPER_CFG")

    def stepper_stat(self):
        # info bit defintion
        ref_done	= 0x00000001
        moving		= 0x00000002
        uv_on		= 0x00000004
        uv_ready	= 0x00000008
        z_in_ref	= 0x00000010
        z_in_print	= 0x00000020
        z_in_cap	= 0x00000040
        z_in_up		= 0x00000080
        x_in_cap	= 0x00000100
        x_in_ref	= 0x00000200
        printing	= 0x00000400
        curing		= 0x00000800
        headUpInput_= 0x00040000
        headUpInput_= 0x00080000
        headUpInput_= 0x00100000
        headUpInput_= 0x00200000
        scannerEnabl= 0x01000000
        printhead_en= 0x04000000
        msg = Message("REP_STEPPER_STAT")
        msg.no = self.board.no
        msg.info = ref_done | z_in_print
        msg.cmdRunning = 0# message_mgr.defines["CMD_LIFT_REFERENCE"]
        self.transport.write(msg.pack())
    
    def data_received(self, data):
        super().data_received(data)
        self.stepper_stat()


class Board(network.AbstractBoard):
    "board information regarding state"
    def __init__(self, d):
        super().__init__(d)

    def reset(self):
        "reset the board by clearing everythings"
        self.error = 0

async def startup(loop, board):
    "startup a boad"
    pass

async def process(loop, boards):
    "process all boards periodically"
    pass


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
    logging.basicConfig(#format='%(asctime)s %(levelname)s:%(message)s',
                        level=numeric_level)

    simulation = network.Network()
    simulation.register("Stepper", sys.modules[__name__])
    try:
        simulation.run()
    except KeyboardInterrupt:
        # ignore Ctrl+C silently
        pass

