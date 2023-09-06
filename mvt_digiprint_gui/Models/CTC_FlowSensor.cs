using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace RX_DigiPrint.Models
{

	// https://sensirion.com/de/produkte/katalog/SLQ-QT500/
	// 
	public class CTC_FlowSensor
    {
        [DllImport("ShdlcDriver.dll", EntryPoint = "OpenPort", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 _DLL_OpenPort(byte aPortType, string aPortConfig, out Int32 aPortHandle);

        public static UInt32 OpenPort(byte aPortType, string aPortConfig, out Int32 aPortHandle)
        {
            try
            {
                UInt32 ret=_DLL_OpenPort(aPortType, aPortConfig, out Int32 port);
                aPortHandle=port;
                return ret;
            }
            catch(Exception)
            {
            }
            aPortHandle=0;
            return 0x485; //ERROR_DLL_NOT_FOUND;
        }

        [DllImport("ShdlcDriver.dll", EntryPoint = "ClosePort", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 ClosePort(Int32 aPortHandle);

        [DllImport("ShdlcDriver.dll", EntryPoint = "DeviceReset", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 DeviceReset(Int32 aPortHandle, byte aSlaveAdr);

        [DllImport("ShdlcDriver.dll", EntryPoint = "GetDeviceAddress", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetDeviceAddress(Int32 aPortHandle, byte aSlaveAdr, out byte aAddress);

        [DllImport("ShdlcDriver.dll", EntryPoint = "SetDeviceAddress", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 SetDeviceAddress(Int32 aPortHandle, byte aBroadcastMode, byte aSlaveAdr, byte aAddress);



        // Import of commands in Sensor Cable Dll
        [DllImport("SensorCableDriver.dll", EntryPoint = "GetTermination", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetTermination(Int32 aPortHandle, byte aSlaveAdr, out byte aTermination);

        [DllImport("SensorCableDriver.dll", EntryPoint = "GetSensorVoltage", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetSensorVoltage(Int32 aPortHandle, byte aSlaveAdr, out byte aVoltageSetting);

        [DllImport("SensorCableDriver.dll", EntryPoint = "GetSensorType", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetSensorType(Int32 aPortHandle, byte aSlaveAdr, out byte aSensorType);

        [DllImport("SensorCableDriver.dll", EntryPoint = "GetSensorAddress", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetSensorAddress(Int32 aPortHandle, byte aSlaveAdr, out byte aSensorType);

        [DllImport("SensorCableDriver.dll", EntryPoint = "GetReplyDelay", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetReplyDelay(Int32 aPortHandle, byte aSlaveAdr, out Int16 aReplyDelay);

        [DllImport("SensorCableDriver.dll", EntryPoint = "GetI2cDelay", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetI2cDelay(Int32 aPortHandle, byte aSlaveAdr, out Int16 aI2cDelay);

        [DllImport("SensorCableDriver.dll", EntryPoint = "GetHeaterMode", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetHeaterMode(Int32 aPortHandle, byte aSlaveAdr, out byte aHeaterMode);

        [DllImport("SensorCableDriver.dll", EntryPoint = "SetHeaterMode", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 SetHeaterMode(Int32 aPortHandle, byte aSlaveAdr, byte aHeaterMode);


        [DllImport("SensorCableDriver.dll", EntryPoint = "GetSensorPartName", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetSensorPartName(Int32 aPortHandle, byte aSlaveAdr, StringBuilder aSensorPartName, UInt32 aStringMaxSize);

        [DllImport("SensorCableDriver.dll", EntryPoint = "GetMeasurementType", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetMeasurementType(Int32 aPortHandle, byte aSlaveAdr, out byte aMeasureType);

        [DllImport("SensorCableDriver.dll", EntryPoint = "GetResolution", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetResolution(Int32 aPortHandle, byte aSlaveAdr, out byte aResolution);

        [DllImport("SensorCableDriver.dll", EntryPoint = "GetCalibField", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetCalibField(Int32 aPortHandle, byte aSlaveAdr, out byte aCalibField);

        [DllImport("SensorCableDriver.dll", EntryPoint = "GetLinearization", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetLinearization(Int32 aPortHandle, byte aSlaveAdr, out byte aLinearization);

        [DllImport("SensorCableDriver.dll", EntryPoint = "GetSensorItemNumber", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetSensorItemNumber(Int32 aPortHandle, byte aSlaveAdr, StringBuilder aSensorPartName, UInt32 aStringMaxSize);

        [DllImport("SensorCableDriver.dll", EntryPoint = "GetOffset", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetOffset(Int32 aPortHandle, byte aSlaveAdr, out Int32 aOffset);

        [DllImport("SensorCableDriver.dll", EntryPoint = "GetFlowUnit", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetFlowUnit(Int32 aPortHandle, byte aSlaveAdr, out Int16 aFlowUnit);


        [DllImport("SensorCableDriver.dll", EntryPoint = "StartSingleMeasurement", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 StartSingleMeasurement(Int32 aPortHandle, byte aBroadcastMode, byte aSlaveAdr);

        [DllImport("SensorCableDriver.dll", EntryPoint = "GetSingleMeasurementSigned", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetSingleMeasurementSigned(Int32 aPortHandle, byte aSlaveAdr, out Int32 aMeasureResult);

        [DllImport("SensorCableDriver.dll", EntryPoint = "GetSingleMeasurementUnsigned", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetSingleMeasurementUnsigned(Int32 aPortHandle, byte aSlaveAdr, out UInt32 aMeasureResult);
        
        [DllImport("SensorCableDriver.dll", EntryPoint = "GetFlowUnitString", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetFlowUnitString(Int32 aPortHandle, byte aSlaveAdr, StringBuilder aUnitName, UInt32 aStringMaxSize);

        [DllImport("SensorCableDriver.dll", EntryPoint = "GetScaleFactor", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static extern UInt32 GetScaleFactor(Int32 aPortHandle, byte aSlaveAdr, out Int16 factor);

        public CTC_FlowSensor()
        {
        }

        public string ErrorStr(UInt32 err)
        {
            switch (err)
            {
                case 0: return null;
                case 1: return "Device reported an illegal data size";
                case 2: return "Command not accepted from device";
                case 3: return "No access right on device for this command";
                case 4: return "Parameter out of range (report from device)";

                case 32: return "command could not be executed because sensor is busy";
                case 33: return "Sensor gives no I2C acknowledge";
                case 34: return "CRC error while communication with sensor";
                case 35: return "Timeout of sensor while measurement";
                case 36: return "No measure is started";

                case 128: return "Fatal system error";
                case 129: return "In the Rx data stream, the start or stop byte (0x7E) is missing.";
                case 130: return "Too few bytes in Rx frame (frame content + checksum >= 5 bytes).";
                case 131: return "The transmitted data length information in the Rx frame does not match with the number of bytes received.";
                case 132: return "The port configuration string has an illegal format.";
                case 133: return "Could not open the COM port.";
                case 134: return "Could not close COM port.";
                case 135: return "Unknown communication type of communication port.";
                case 136: return "Incoming checksum was wrong.";
                case 137: return "The device command in the received frame is not the same as sent.";
                case 138: return "The returned number of Data is wrong for this command";
                case 139: return "Illegal broadcast mode";
                case 140: return "One of the given arguments has an illegal size.";
                case 141: return "The SerialPortOverlapped class reported an error.";
                case 142: return "Do not use the broadcast address when calling the transceive function.";
                case 143: return "The maximum number of open ports which could be handled by the DLL is reached.";
                case 144: return "The given port handle is not valid.";
                case 145: return "The requested functionality is not implemented yet.";
                case 146: return "An error occured while calling a windows API function.";
                case 147: return "A timeout occured while waiting for the RX data.";
                case 148: return "The function SerialPortOverlapped.WriteData() could not write all data.";
                case 149: return "The COM port is not open when trying to work with it (in SerialPortOverlapped)";
                default:  return string.Format("Error {0}", err);
            }
        }

        //--- Test --------------------------------
        public void Test()
        {
            Int32 xPortHandle=-1;
            UInt32 xError;
            byte slaveAddr=0;
            Int32 result;

            // open Port
            for (int i=1; i<10; i++)
            {
                string cfg=string.Format("COM{0}, 115200, ECHOOFF", i);
                xError = OpenPort(0, cfg, out xPortHandle);
                if (xError==0) break;
            }
            // Read SensorPartName from device at port xPortHandle and Address 0
            StringBuilder xSensorPartName = new StringBuilder(256);
            xError = GetSensorPartName(xPortHandle, slaveAddr, xSensorPartName, (UInt32)xSensorPartName.Capacity);
            Console.WriteLine("Sensor: {0}", xSensorPartName);

            for (int i=0; i<10; i++)
            {
                xError=StartSingleMeasurement(xPortHandle, 0, slaveAddr);
                xError=GetSingleMeasurementSigned(xPortHandle, slaveAddr, out result);
                Console.WriteLine("{0}: flow={1}, err=>>{2}<<", Environment.TickCount, result, ErrorStr(xError)); 
            }

            // close Port
            xError = ClosePort(xPortHandle);
        }
        
        private Int32 _PortHandle=-1;
        private Int16 _Factor;

        //--- StartMeasurement --------------------------------
        public bool StartMeasurement()
        {
            UInt32 err;
            StopMeasurement();
            StringBuilder str = new StringBuilder(256);

            // open Port
            for (int i=1; i<10; i++)
            {
                err=0;
                string cfg=string.Format("COM{0}, 115200, ECHOOFF", i);
                err=OpenPort(0, cfg, out _PortHandle);
                if (err==0) err = GetSensorPartName(_PortHandle, 0, str, (UInt32)str.Capacity);
                if (err==0) err = GetFlowUnitString(_PortHandle, 0, str, (UInt32)str.Capacity);
                if (err==0) err = GetScaleFactor(_PortHandle, 0, out _Factor);

                if (err==0) 
                {
                    Console.WriteLine("GetScaleFactor={0}", _Factor);
                    Console.WriteLine("GetFlowUnitString={0}", str);

                    err=  GetTermination(_PortHandle, 0, out byte aTermination);
                    Console.WriteLine("GetTermination={0}", aTermination);

                    err=  GetSensorVoltage(_PortHandle, 0, out byte aVoltageSetting);
                    Console.WriteLine("GetSensorVoltage={0}", aVoltageSetting);

                    err=  GetSensorType(_PortHandle, 0, out byte aSensorType);
                    Console.WriteLine("GetSensorType={0}", aSensorType);

                    err=  GetSensorAddress(_PortHandle, 0, out byte aSensorAddress);
                    Console.WriteLine("GetSensorAddress={0}", aSensorAddress);

                    err=  GetReplyDelay(_PortHandle, 0, out Int16 aReplyDelay);
                    Console.WriteLine("GetReplyDelay={0}", aReplyDelay);

                    err=  GetI2cDelay(_PortHandle, 0, out Int16 aI2cDelay);
                    Console.WriteLine("GetI2cDelay={0}", aI2cDelay);

                    err= GetHeaterMode(_PortHandle, 0, out byte aHeaterMode);
                    Console.WriteLine("GetHeaterMode={0}", aHeaterMode);


                    err= GetMeasurementType(_PortHandle, 0, out byte aMeasureType);
                    Console.WriteLine("GetMeasurementType={0}", aMeasureType);

                    err= GetResolution(_PortHandle, 0, out byte aResolution);
                    Console.WriteLine("GetResolution={0}", aResolution);

                    err= GetCalibField(_PortHandle, 0, out byte aCalibField);
                    Console.WriteLine("GetCalibField={0}", aCalibField);

                    err= GetLinearization(_PortHandle, 0, out byte aLinearization);
                    Console.WriteLine("GetLinearization={0}", aLinearization);

                    err= GetSensorItemNumber(_PortHandle, 0, str, (UInt32)str.Capacity);
                    Console.WriteLine("GetSensorItemNumber={0}", str);

                    err= GetOffset(_PortHandle, 0, out Int32 aOffset);
                    Console.WriteLine("GetOffset={0}", aOffset);

                    err= GetFlowUnit(_PortHandle, 0, out Int16 aFlowUnit);
                    Console.WriteLine("GetFlowUnit={0}", aFlowUnit);
                    
                    break;
                };
                _PortHandle=-1;
            }
            if (_PortHandle<0) return false;
            if (_Factor<0) _Factor=1;
            err=SetHeaterMode(_PortHandle, 0, 1);
            return true;
        }

        //--- MeasureFlow ----------------------------------
        public bool MeasureFlow(out int flow)
        {
            flow=0;
            if (_PortHandle<0)
                return false;

            UInt32 err;
            Int32 value=0;
            byte slaveAddr=0;
            err=StartSingleMeasurement(_PortHandle, 0, slaveAddr);
            if (err==0) 
            {
                Thread.Sleep(100);
                do
                {
                    err=GetSingleMeasurementSigned(_PortHandle, slaveAddr, out value);
                    Console.WriteLine("Sensor: err={0} vel={1}", err, value);
                } while(err==1376);
            }
            if (err!=0) 
                return false;
            flow = (value*60)/1000/_Factor;
            return true;
        }

        //--- StopMeasurement --------------------------------------
        public void StopMeasurement()
        {
            if (_PortHandle>=0) 
            {
               SetHeaterMode(_PortHandle, 0, 0);
                ClosePort(_PortHandle);
            }
            _PortHandle = -1;
        }
    }
}
