from opcua import ua, Server
import logging
import threading
logger = logging.getLogger(__name__)

class Siemens(object):
    """
    Mock for the Siemens PLC
    """
    def datachange_notification(self, node, val, data):
        "Subscription Handler. To receive events from server for a subscription"
        logger.info(f"Python: New data change event {node} {val}")
        if str(node).endswith("boReqStart") and val == True:
            if self.boIsReadyToPrint.get_value():
                logger.info("Restart...")
                self.boReqPrintOn.set_value(True)
                self.boMachineIsStopped.set_value(False)
                self.boMachineIsRunning.set_value(True)
            else:
                logger.info("Prepare to print")
                self.boReqPrepareToPrint.set_value(True)
        elif str(node).endswith("boIsReadyToPrint") and val == True:
            logger.info("Print !")
            self.boReqPrintOn.set_value(True)
            self.boMachineIsStopped.set_value(False)
            self.boMachineIsRunning.set_value(True)
        elif str(node).endswith("boIsPrinting") and val == False:
            logger.info("Stopped")
            self.boReqPrepareToPrint.set_value(False)
            self.boReqPrintOn.set_value(False)
        elif str(node).endswith("boReqPause") and val == True:
            logger.info("Pause")
            self.boMachineIsStopped.set_value(True)
            self.boMachineIsRunning.set_value(False)
        elif str(node).endswith("boReqStop") and val == True:
            logger.info("Stop")
            self.boMachineIsStopped.set_value(True)
            self.boMachineIsRunning.set_value(False)
        elif str(node).endswith("boReqPowerOnUvLamps"):
            logger.warning(f"boReqPowerOnUvLamps change to {val}")
            if val == True:
                self.inUvLampStatus.set_value(ua.Variant(1, ua.VariantType.Int16))
                self.inUvLampCoolingCountdown.set_value(ua.Variant(10, ua.VariantType.Int16))
                threading.Timer(1, self.countdown).start()
            else:
                if self.inUvLampStatus.get_value() == 2:
                    self.inUvLampStatus.set_value(ua.Variant(3, ua.VariantType.Int16))
                    self.inUvLampCoolingCountdown.set_value(ua.Variant(10, ua.VariantType.Int16))
                    threading.Timer(1, self.countdown).start()           

    def countdown(self):
        c = self.inUvLampCoolingCountdown.get_value()
        self.inUvLampCoolingCountdown.set_value(ua.Variant(max(c - 1, 0), ua.VariantType.Int16))
        if c >= 1:
            threading.Timer(1, self.countdown).start()
        else:
            if self.inUvLampStatus.get_value() == 1:
                self.inUvLampStatus.set_value(ua.Variant(2, ua.VariantType.Int16))
            else:
                self.inUvLampStatus.set_value(ua.Variant(0, ua.VariantType.Int16))



    @staticmethod
    def add_node_string(parent, idx, name, *args):
        "add a opcua node to a parent with a string nodeId based on the path"
        if parent.nodeid.NamespaceIndex != idx: # special case for first node
            parentname=""
        else:
            parentname=parent.nodeid.Identifier+"." # prefix of the name based on parent name
        
        id = f"ns={idx};s={parentname}{name};" # NodeId
        # create folder of variable node
        if len(args) == 0:
            return parent.add_folder(id, name)
        else:
            var = parent.add_variable(id, name, *args)
            var.set_writable()
            return var

    def __init__(self):
        # create OPCUA server to simulate Siemens PLC
        self.server = Server()
        self.server.set_endpoint("opc.tcp://0.0.0.0:55105")

        # create all usefull nodes
        idx = 6
        prefix = Siemens.add_node_string(self.server.nodes.objects, idx, "S71500ET200MP station_1")
        prefix = Siemens.add_node_string(prefix, idx, "PLC_1")
        prefix = Siemens.add_node_string(prefix, idx, "DPU_DB")

        dpu_to_plc = Siemens.add_node_string(prefix, idx, "DPU_to_PLC")

        inHoursCluster = Siemens.add_node_string(dpu_to_plc, idx, "inHoursCluster ", ua.Variant(21*[0], ua.VariantType.Int16))
        self.inInkLevels = Siemens.add_node_string(dpu_to_plc, idx, "inInkLevels", ua.Variant(7*[-1], ua.VariantType.Int16))
        self.inLifeMonitor = Siemens.add_node_string(dpu_to_plc, idx, "inLifeMonitor", 10, ua.VariantType.Int16)
        self.boReqPrepareToPrintAck = Siemens.add_node_string(dpu_to_plc, idx,"boReqPrepareToPrintAck",False,ua.VariantType.Boolean)
        self.boIsReadyToPrint = Siemens.add_node_string(dpu_to_plc, idx,"boIsReadyToPrint",False,ua.VariantType.Boolean)
        self.boReqPrintOnAck = Siemens.add_node_string(dpu_to_plc, idx,"boReqPrintOnAck",False,ua.VariantType.Boolean)
        self.boIsPrinting = Siemens.add_node_string(dpu_to_plc, idx,"boIsPrinting",False,ua.VariantType.Boolean)
        self.boReqStart = Siemens.add_node_string(dpu_to_plc, idx,"boReqStart",False,ua.VariantType.Boolean)
        self.boReqStop = Siemens.add_node_string(dpu_to_plc, idx,"boReqStop",False,ua.VariantType.Boolean)
        self.boReqPause = Siemens.add_node_string(dpu_to_plc, idx,"boReqPause",False,ua.VariantType.Int16)
        self.inFormatLength = Siemens.add_node_string(dpu_to_plc, idx,"inFormatLength",0, ua.VariantType.Boolean)
        self.boReqPowerOnUvLamps = Siemens.add_node_string(dpu_to_plc, idx,"boReqPowerOnUvLamps",False,ua.VariantType.Boolean)
        self.reMachineSpeedSetpoint = Siemens.add_node_string(dpu_to_plc, idx, "reMachineSpeedSetpoint", 0.0,  ua.VariantType.Float)


        plc_to_dpu = Siemens.add_node_string(prefix, idx, "PLC_to_DPU")
        self.inLateralRegister = Siemens.add_node_string(plc_to_dpu, idx, "inLateralRegister", 0, ua.VariantType.Int16)
        self.boReqPrepareToPrint = Siemens.add_node_string(plc_to_dpu, idx,"boReqPrepareToPrint",False,ua.VariantType.Boolean)
        self.boReqPrintOn = Siemens.add_node_string(plc_to_dpu, idx,"boReqPrintOn",False,ua.VariantType.Boolean)
        self.boReqStartAck = Siemens.add_node_string(plc_to_dpu, idx,"boReqStartAck",False,ua.VariantType.Boolean)
        self.boMachineIsRunning = Siemens.add_node_string(plc_to_dpu, idx,"boMachineIsRunning",False,ua.VariantType.Boolean)
        self.boReqStopAck = Siemens.add_node_string(plc_to_dpu, idx,"boReqStopAck",False,ua.VariantType.Boolean)
        self.boMachineIsStopped = Siemens.add_node_string(plc_to_dpu, idx,"boMachineIsStopped",False,ua.VariantType.Boolean)
        self.boReqPauseAck = Siemens.add_node_string(plc_to_dpu, idx,"boReqPauseAck",False,ua.VariantType.Boolean)
        self.boSequencePauseIsRunning = Siemens.add_node_string(plc_to_dpu, idx,"boSequencePauseIsRunning",False,ua.VariantType.Boolean)
        self.inUvLampStatus = Siemens.add_node_string(plc_to_dpu, idx, "inUvLampStatus", 0,  ua.VariantType.Int16)
        self.inUvLampCoolingCountdown = Siemens.add_node_string(plc_to_dpu, idx, "inUvLampCoolingCountdown", 0,  ua.VariantType.Int16)
        self.reMachineActualSpeed = Siemens.add_node_string(plc_to_dpu, idx, "reMachineActualSpeed", 50.0,  ua.VariantType.Float)

        self.server.start()

        # enable subscriscription to nodes on server side
        sub = self.server.create_subscription(500, self)
        sub.subscribe_data_change(self.boReqStart)
        sub.subscribe_data_change(self.boIsReadyToPrint)
        sub.subscribe_data_change(self.boIsPrinting)
        sub.subscribe_data_change(self.boReqStart)
        sub.subscribe_data_change(self.boReqStop)
        sub.subscribe_data_change(self.boReqPause)
        sub.subscribe_data_change(self.boReqPowerOnUvLamps)

    def stop(self):
        self.server.stop()



server = Siemens()
