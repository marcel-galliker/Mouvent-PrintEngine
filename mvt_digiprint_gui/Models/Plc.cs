using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using RxRexrothGui.Models;
using RxRexrothGui.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

namespace RX_DigiPrint.Models
{
    public class Plc :RxBindable
    {
        public Action   LogComplete;
        int             _MaxLogItems=50;

        List<List<string>>    _Variables=new List<List<string>>();    // buffer of "STA_*" variables

        //--- Property UnitID ---------------------------------------
        private string _UnitID = "Application.GUI_00_001_Main";
        public string UnitID
        {
            get { return _UnitID; }
        }
        
        //--- Property Info ---------------------------------------
        private RxRexroth.SSystemInfo _Info;
        public RxRexroth.SSystemInfo Info
        {
            get { return _Info; }
            set { SetProperty(ref _Info, value);}
        }

        //--- Property Log ---------------------------------------
        private CPlcLogCollection _Log = new CPlcLogCollection();
        public CPlcLogCollection Log
        {
            get { return _Log; }
            set { SetProperty(ref _Log, value); }
        }

        //--- AddLogItem ----------------------------------------------------
        public void AddLogItem(CPlcLogItem item)
        {
            Console.WriteLine("Plc.AddLogItem(no={0}, test={1})", item.No, item.No<0);
            while (Log.Count>=_MaxLogItems) Log.RemoveAt(Log.Count-1);
            if (item.No<=0) Log.Insert(0, item);
            else            Log.Add(item);
        }

        //--- Property Connected ---------------------------------------
        private bool _Connected;
        public bool Connected
        {
            get
            {
                if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LH702) _Connected = true;
                return _Connected;
            }
            set
            {
                if (value != _Connected)
                {
                    _Connected = value;
                    if (_Connected) RxBindable.Invoke(_RequestLog);
                    foreach (List<string> list in _Variables)
                    {
                        OnPropertyChanged(list[0]);
                    }
                    OnPropertyChanged();
                }
            }
        }

        //--- RequestVar ------------------------------------------------------
        private string[] _varList = new string[256];
        private int      _varListLen=0;
        private DateTime _lastTime = DateTime.Now;
        public void RequestVar(string var)
        {
            int i;
            string[] list = var.Split('\n');
            if (list.Length>1 && list[0].Equals(_UnitID))
            {
                for (i=0; i<list.Length; i++)
                {
                    if (!list[i].Equals("") && !_varList.Contains(list[i]))
                    {
                        _varList[_varListLen++] = list[i];
                    }
                }
                TimeSpan diff=DateTime.Now-_lastTime;
                if (diff.Milliseconds>100 && _varListLen>0)
                {
                    _lastTime=DateTime.Now;
                    new Thread(()=>
                    {
                        Thread.Sleep(10);
                        StringBuilder str = new StringBuilder(2048);
                        for (int n=0; n<_varListLen; n++)
                        {
                            str.Append(_varList[n]+'\n');
                            _varList[n] = "";
                        }
                        _varListLen=0;
                        RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_GET_VAR, str.ToString());
                    }).Start();
                }
            }
        }

        //--- RequestAllPar ---------------------------------------------------
        public void RequestAllPar()
        {
            OnPropertyChanged("ALL_PLC_PAR");
        }

        //--- _RequestLog -----------------------------------------------
        private void _RequestLog()
        {
            Log.Clear();
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_GET_LOG);
        }

        //--- UpdateVar -------------------------------------------------
        public void UpdateVar(string msg)
        {
            string[] msglist = msg.Split('\n');

          //  Console.WriteLine("PLC Variables: {0}", msg);

            if (msglist!=null && msglist.Count()>1)
            {
              //  Console.WriteLine("UpdateVar str[0]=>>{0}<< str[1]=>>{1}<<", msglist[0], msglist[1]);
               _updateVar(_Variables, msglist);
                OnPropertyChanged(msglist[0]);
            }
        }

        //--- _updateVar --------------------------------------------------------
        private void _updateVar(List<List<string>>buffer, string[] list)
        {
            int i;
            bool found;
            for(i=0, found=false; i<buffer.Count && !found; i++)
            {
                if (buffer[i][0].Equals(list[0]))
                {
                    found=true;
                    break;                
                }
            }
            if (!found)
            {
                i=buffer.Count;
                buffer.Add(new List<string>());
                buffer[i].Add(list[0]);
            }

            int n, m;
            // search and overwrite sent lines
            for (n=1; n<list.Count(); n++)
            {
                if (!list[n].Equals(""))
                {
                    string[] val = list[n].Split('=');
                    if (val[0].Equals("STA_MACHINE_STATE"))
                    {
                        MachineStateEnum state= (MachineStateEnum) Rx.StrToInt32(val[1]);
                        InReferencing = (state == MachineStateEnum.Referencing);
                        InWebIn       = (state== MachineStateEnum.WebIn);
                        WebInEnabled  = (state != MachineStateEnum.Prepare && state != MachineStateEnum.Pause && state!= MachineStateEnum.Run && state != MachineStateEnum.Error);
                        if (WebInActive && state != MachineStateEnum.Pause)
                            WebInActive = false;
                        if (ToWebIn && state == MachineStateEnum.Error) ToWebIn = false;
                        if (ToWebIn && state == MachineStateEnum.Pause)
                        {
                            WebInActive = true;
                            ToWebIn = false;
                        }
                    }
                    if (val[0].Equals("STA_REFERENCE_ENABLE")) ReferenceEnabled = (val[1].Equals("TRUE"));

                    for (m=1, found=false; m<buffer[i].Count(); m++)
                    {
                        if (buffer[i][m].StartsWith(val[0]))
                        {
                            buffer[i][m]=list[n];
                            found=true;
                            break;
                        }
                    }
                    if (!found) buffer[i].Add(list[n]);
                }
            }
        }

        //--- GetVar -------------------------------------
        public string GetVar(string uintId, string id)
        {
            int i, n;
            string match=id+'=';

            if (!Connected) return "####";
                                
            for(i=0; i<_Variables.Count; i++)
            {
                if (_Variables[i][0].Equals(uintId))
                {
                    for (n=0; n<_Variables[i].Count(); n++)
                    {
                        if (_Variables[i][n].StartsWith(match))
                        {
                            string[] val = _Variables[i][n].Split('=');
                            return val[1];
                        }
                    }
                }
            }
            return null;
        }

        //--- SetVar -----------------------------------------
        public void SetVar(string id, object value)
        {
            string str = string.Format("{0}\n{1}={2}\n", UnitID, id, value.ToString());
            RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_SET_VAR, str);
        }

        //--- Property InService ---------------------------------------
        private bool _InService=false;
        public bool InService
        {
            get { return _InService; }
            set { SetProperty(ref _InService, value); }
        }
        
        //--- Property InSetup ---------------------------------------
        private bool _InSetup=false;
        public bool InSetup
        {
            get { return _InSetup; }
            set { SetProperty(ref _InSetup, value); }
        }

        //--- Property InWebIn ---------------------------------------
        private bool _InWebIn=false;
        public bool InWebIn
        {
            get { return _InWebIn; }
            set { SetProperty(ref _InWebIn, value); }
        }

        //--- Property InReferencing ---------------------------------------
        private bool _InRefencing=false;
        public bool InReferencing
        {
            get { return _InRefencing; }
            set { SetProperty(ref _InRefencing, value); }
        }

        //--- Property CLEAF_CanRefBuf ---------------------------------------
        private bool _ReferenceEnabled=false;
        public bool ReferenceEnabled
        {
            get { return _ReferenceEnabled; }
            set { SetProperty(ref _ReferenceEnabled, value); }
        }

        //--- Property IsReadyForProduction ---------------------------------------
        private bool _IsReadyForProduction=true;
        public bool IsReadyForProduction
        {
            get { return _IsReadyForProduction; }
            set { SetProperty(ref _IsReadyForProduction, value);}
        }

        public bool ToWebIn { get; set; } = false;
        public bool WebInActive { get; set; } = false;

        //--- Property WebInEnabled ---------------------------------------
        private bool _WebInEnabled = false;
        public bool WebInEnabled
        {
            get { return _WebInEnabled; }
            set { SetProperty(ref _WebInEnabled, value);}
        }
    }
}
