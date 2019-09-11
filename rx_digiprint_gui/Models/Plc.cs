using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Services;
using RxRexrothGui.Models;
using RxRexrothGui.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;

namespace RX_DigiPrint.Models
{
    public class Plc :RxBindable
    {
        public Action   LogComplete;
        RxWorkBook      _WB;
        int             _WB_sheet;
        int             _WB_row=0;
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
            set 
            { 
               _Info = value;
               if (_WB_row>0)
               {
                    LogInfo(_WB, _WB_row);
                    _WB_row = 0;
               }               
               else OnPropertyChanged();
            }
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

        //--- LogInfo ---------------------------------------
        private void LogInfo(RxWorkBook wb, int row)
        {
            row+=2;
            wb.setText(row, 0, "Rexroth System Info");
            wb.HeaderRow(row);
            row++;
            wb.setText(row, 0, "versionHardware");          wb.setText(row++, 1, Info.versionHardware); 
            wb.setText(row, 0, "versionFirmware");          wb.setText(row++, 1, Info.versionFirmware); 
            wb.setText(row, 0, "versionLogic");             wb.setText(row++, 1, Info.versionLogic); 
            wb.setText(row, 0, "versionBsp");               wb.setText(row++, 1, Info.versionBsp); 
            wb.setText(row, 0, "versionMlpi");              wb.setText(row++, 1, Info.versionMlpi); 
            wb.setText(row, 0, "serialNo");                 wb.setText(row++, 1, Info.serialNo); 
            wb.setText(row, 0, "hardwareDetails");          wb.setText(row++, 1, Info.hardwareDetails); 
            wb.setText(row, 0, "localBusConfig");           wb.setText(row++, 1, Info.localBusConfig); 
            wb.setText(row, 0, "modulBusConfig");           wb.setText(row++, 1, Info.modulBusConfig); 
            wb.setText(row, 0, "modulHwDetails");           wb.setText(row++, 1, Info.modulHwDetails); 
            wb.setText(row, 0, "tempAct");                  wb.setText(row++, 1, Info.tempAct); 
            wb.setText(row, 0, "tempMax");                  wb.setText(row++, 1, Info.tempMax);
        }

        //--- WriteLog -----------------------------------------------------
        private void WriteLog()
        {
            int row = 0;
            _WB.Sheet = _WB_sheet;
            _WB.setText(row, 0, "No");
            _WB.setText(row, 1, "Time");
            _WB.setText(row, 2, "Err");
            _WB.setText(row, 3, "Message");
            _WB.HeaderRow(row);

            foreach(CPlcLogItem item  in RxGlobals.Plc.Log)
            {
                row++;
                _WB.setText(row, 0, item.No);
                _WB.setText(row, 1, item.Time);
                _WB.setText(row, 2, string.Format("{0:X}", item.ErrNo));
                _WB.setText(row, 3, item.Text);
            }
            LogComplete -= WriteLog;
            _WB.SizeColumns();
            _WB.write(_WB.FileName);
        }                

        //--- LogInfo ------------------------------------------
        public void LogInfo(RxWorkBook wb)
        {            
            _WB = wb;
            _WB_row = wb.LastRow;
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_GET_INFO);
            for (int i=0; i<20 &&_WB_row>0;  i++) Thread.Sleep(100);
            _WB_row = 0;
        }

        //--- LogErrors ------------------------------------------------------
        public void LogErrors(RxWorkBook wb)
        {
            _WB = wb;
            _WB_sheet = wb.Sheet;
            LogComplete += WriteLog;
            RxGlobals.Plc.Log = new CPlcLogCollection();
            RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_GET_LOG);
        }

        //--- Property Connected ---------------------------------------
        private bool _Connected;
        public bool Connected
        {
            get { return _Connected; }
            set 
            { 
                if (value!=_Connected)
                {
                    _Connected=value;
                    if (_Connected) RxBindable.Invoke(_RequestLog);
                    foreach(List<string> list in _Variables)
                    {
                        OnPropertyChanged(list[0]);
                    }
                    OnPropertyChanged();
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
                        int state=Rx.StrToInt32(val[1]);
                        InReferencing = (state==13);
                        InWebIn       = (state==9);
                        WebInEnabled  = (state!=4 && state!=6);
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
        private bool _ReferenceEnabled;
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

        //--- Property WebInEnabled ---------------------------------------
        private bool _WebInEnabled = false;
        public bool WebInEnabled
        {
            get { return _WebInEnabled; }
            set { SetProperty(ref _WebInEnabled, value);}
        }
    }
}
