using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Services;
using System;
using System.Diagnostics;
using System.Text;

namespace RX_DigiPrint.Models
{
    public class LogItem
    {       
        #region Creator
        public LogItem()
        {
            init_item();
        }

        private void init_item()
        {
            StringBuilder str = new StringBuilder(100);
            str.AppendFormat("{0:D2}:{1:D2}:{2:D2}", DateTime.Now.Hour, DateTime.Now.Minute, DateTime.Now.Second);
            TimeStr = str.ToString();

            StackTrace st = new StackTrace(true);
            StackFrame sf = st.GetFrame(1);
            File    = sf.GetFileName();
            Line    =(uint)sf.GetFileLineNumber();      
            LogType = ELogType.eErrLog;
            DevType = EDeviceType.dev_gui;
            DevNo       =-1;
        }

        public LogItem(TcpIp.SErrorMsg err)
        {
            LogType  = err.logType;
            FileTime = err.time;
            Error    = err.errNo;
            Message  = ErrorMessage.Compose(err);
            File     = err.file;
            Line     = err.line;
            DevType  = err.devType;
            DevNo    = err.devNo;
        }

        //--- LogItem -------------------------------------------------
        public LogItem(string format, params object[] args)
        {
            init_item();
            FileTime = DateTime.Now.ToFileTime();
            StringBuilder str = new StringBuilder(100);
            try
            {
                str.AppendFormat(format, args);
                Message = str.ToString();
            }
            catch
            {
            }
        }

        #endregion

        //--- Prpperty TimeStr ----------------------------------------
        #region Properties
        private string _TimeStr;
        public string TimeStr
        {
            get { return _TimeStr; }
            set { _TimeStr = value; }
        }

        //--- Property FileTime ------------------------------------
        private Int64 _FileTime;
        public Int64 FileTime
        {
            get { return _FileTime; }
            set 
            {
                try
                {
                    _FileTime = value;
                    StringBuilder str = new StringBuilder(100);
                    DateTime t = DateTime.FromFileTime(_FileTime);
                    str.AppendFormat("{0:D2}.{1:D2}.{2:D2}   {3:D2}:{4:D2}:{5:D2}.{6:D3}", t.Day, Rx.MonthName[t.Month], t.Year, t.Hour, t.Minute, t.Second, t.Millisecond);
                    TimeStr = str.ToString(); 
                }
                catch (Exception)
                {
                }
            }
        }

        //--- Property LogType -------------------------------------
        private ELogType _LogType;
        public ELogType LogType
        {
            get { return _LogType; }
            set 
            { 
                _LogType = value;
                switch (_LogType)
                {
                case ELogType.eErrWarn:   LogTypeStr = "WARN"; break;
                case ELogType.eErrCont:  
                case ELogType.eErrStop:  
                case ELogType.eErrAbort:  LogTypeStr = "ERROR"; break;
                default:                  LogTypeStr = "LOG"; break;
                }
            }
        }

        //--- Property LogTypeStr ------------------------------------- 
        private string _LogTypeStr;
        public string LogTypeStr
        {
            get { return _LogTypeStr; }
            set { _LogTypeStr = value; }
        }

        //--- Property DevType ------------------------------------------
        private EDeviceType _DevType;
        public EDeviceType DevType
        {
            get { return _DevType; }
            set { _DevType = value; }
        }
        
        //--- Property DevNo ---------------------------------------------
        private int _DevNo;
        public int DevNo
        {
            get { return _DevNo; }
            set { _DevNo = value; }
        }

        //--- Property DeviceStr ----------------------------------------
        public string DeviceStr
        {
            get 
            { 
                string noStr="";
                if (DevNo>=0) noStr = string.Format(" {0}", _DevNo+1);

                switch(_DevType)
                {
                    case EDeviceType.dev_gui:       return "GUI";
                    case EDeviceType.dev_main:      return "Main Ctrl";
                    case EDeviceType.dev_plc:       return "Plc";
                    case EDeviceType.dev_enc:       if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_DP803)
                                                    { 
                                                        if (_DevNo==0) return "Encoder Recto";
                                                        return "Encoder Verso";
                                                    }
                                                    return "Encoder";
                    case EDeviceType.dev_fluid:     return string.Format("Fluid{0}", noStr);
                    case EDeviceType.dev_stepper:   return string.Format("Stepper{0}", noStr);
                    case EDeviceType.dev_head:      try
                                                    {
                                                        int head0 = (_DevNo*(int)TcpIp.HEAD_CNT)%RxGlobals.PrintSystem.HeadCnt;
                                                        if (RxGlobals.PrintSystem.HeadCnt==TcpIp.HEAD_CNT) 
                                                            return string.Format("Cluster {0}", InkType.ColorNameShort(RxGlobals.InkSupply.List[_DevNo].InkType.ColorCode), _DevNo+1, _DevNo+4);
                                                        if (RxGlobals.PrintSystem.HeadCnt> TcpIp.HEAD_CNT)
                                                            return string.Format("Cluster {0}-{1}..{2}", InkType.ColorNameShort(RxGlobals.InkSupply.List[_DevNo].InkType.ColorCode), head0+1, head0+4);
                                                    }
                                                    catch(Exception){};
                                                    return string.Format("Cluster{0}", noStr);
                    case EDeviceType.dev_spool:     return string.Format("Spooler{0}", noStr);
                    default:                        return "";
                }
            }
        }           
        
        //--- Property  Error ---------------------------------------
        private UInt32 _Error;
        public UInt32 Error
        {
            get { return _Error; }
            set { _Error = value; }
        }

        //--- Prpperty Message -------------------------------------
        private string _Message;
        public string Message
        {
            get { return _Message;  }
            set { _Message = value; }
        }

        //--- Property File -----------------------------------------
        private string _File;
        public string File
        {
            get { return _File; }
            set 
            {   
                if (value!=null)
                {
                    int pos=value.LastIndexOfAny(new char[]{'\\', '/'});
                    if (pos>=0) _File=value.Remove(0, pos+1);
                    else _File=value;
                }
                return;
            }
        }

        //--- Property Line  ---------------------------------------
        private uint _Line;
        public uint Line
        {
            get { return _Line; }
            set { _Line = value; }
        }


        //--- Property Alternate ---------------------------------------
        private bool _Alternate;
        public bool Alternate
        {
            get { return _Alternate; }
            set { _Alternate=value; }
        }
        

        //--- SendToBluetooth ----------------------------------------
        public void SendToBluetooth()
        {
            RxBtDef.SErrorMsg msg = new RxBtDef.SErrorMsg();
            {
                msg.device  = DeviceStr;
                msg.message = Message;
                msg.type    = (RxBtDef.ELogType)LogType;
            }
            RxGlobals.Bluetooth.SendMsg(RxBtDef.BT_EVT_GET_EVT, ref msg);
        }
        #endregion
    }
}
