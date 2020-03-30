using RX_Common;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Windows;

namespace RX_DigiPrint.Models
{
    public class NetworkItem 
    {
        #region Creator
        public NetworkItem(ref TcpIp.SNetworkItem item, int flash)
        {
            MacAddr    = item.macAddr;
            DeviceType = item.devType;
            SerialNo   = item.serialNo;
            DeviceNo   = item.devNo+1;
            IpAddr     = item.ipAddr;
            Connected  = (item.connected!=0);
            Flash      = flash;
            DeviceNoList = new EN_DeviceNumbers(DeviceType, RxGlobals.PrintSystem.ColorCnt, RxGlobals.PrintSystem.HeadCnt);            
            DeviceNoEnabled = RxGlobals.User.UserType >= EUserType.usr_service;
        }
        #endregion

        #region Properties

        //--- MacAddr -----------------------------------------------
        private UInt64 _MacAddr;
        public UInt64 MacAddr
        {
            get { return _MacAddr; }
            set { _MacAddr = value; }
        }

        //--- DeviceTypeList --------------------------------------
        public EN_DeviceTypeList DeviceTypeList
        {
            get { return new  EN_DeviceTypeList(); }
        }

        //--- DeviceType -------------------------------------------
        private EDeviceType _DeviceType;
        public EDeviceType DeviceType
        {
            get { return _DeviceType; }
            set { _DeviceType = value;}
        }

        //--- SerialNo ---------------------------------------------
        private string _SerialNo;
        public string SerialNo
        {
            get { return _SerialNo; }
            set { _SerialNo = value; }
        }

        //--- DeviceNoList ------------------------------------------------
        private EN_DeviceNumbers _DeviceNoList;
        public EN_DeviceNumbers DeviceNoList
        {
            set { _DeviceNoList =value;}
            get { return _DeviceNoList;}
        }

        //--- DeviceNo --------------------------------------------------
        private int _DeviceNo;
        public int DeviceNo
        {
            get { return _DeviceNo; }
            set { if (value>0)_DeviceNo = value; else _DeviceNo=1;}
        }

        //--- Property DeviceNoEnabled ---------------------------------------
        private bool _DeviceNoEnabled = false;
        public bool DeviceNoEnabled
        {
            get { return _DeviceNoEnabled; }
            set { _DeviceNoEnabled =value; }
        }

        //--- IpAddr --------------------------------------------------------
        private string _IpAddr;
        public string IpAddr
        {
            get { return _IpAddr; }
            set { _IpAddr = value; }
        }

        //--- Connected --------------------------------------------------
        private bool _Connected;
        public bool Connected
        {
            get { return _Connected; }
            set 
            { 
                if (value!=_Connected)
                { 
                    _Connected = value;
                    if (DeviceType==EDeviceType.dev_plc)    RxGlobals.Plc.Connected=_Connected;
                    if (DeviceType==EDeviceType.dev_head)   RxGlobals.HeadStat.SetConnected(DeviceNo-1, _Connected);
                    if (DeviceType==EDeviceType.dev_fluid)  RxGlobals.InkSupply.SetConnected(DeviceNo-1, _Connected);
                //  if (DeviceType==EDeviceType.dev_stepper && DeviceNo<=RxGlobals.StepperStatus.Length) RxGlobals.StepperStatus[DeviceNo-1].SetConnected(_Connected);
                }
            }
        }
        
        //--- Flash --------------------------------------------------
        private int _Flash;
        public int Flash
        {
            get { return _Flash; }
            set { _Flash = value; _DeviceNoEnabled=(value!=0);}
        }

        //--- Property PuttyDebug ---------------------------------------
        public bool PuttyDebug
        {
            get { return true; }
        }    
        
        //--- Property Putty ---------------------------------------
        public bool Putty
        {
            get { return true; }
        }    

        //--- SendMsg -------------------------------------------               
        public void SendMsg(UInt32 cmdId)
        {
            TcpIp.sSetNetworkCmd msg = new TcpIp.sSetNetworkCmd();

            msg.item.macAddr    = MacAddr;
            msg.item.devType    = DeviceType;
            msg.item.serialNo   = SerialNo;
            msg.item.devNo      = (byte)(DeviceNo-1);

            msg.flash           = (byte)Flash;

            RxGlobals.RxInterface.SendMsg(cmdId, ref msg);
        } 
        #endregion
    }
}
