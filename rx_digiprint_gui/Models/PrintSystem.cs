using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using SmartXLS;
using System;
using System.Linq;
using System.Text;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace RX_DigiPrint.Models
{
    public class PrintSystem : RxBindable
    {     
        private bool  _Init=false;

        //--- Constructor ------------------------------------------
        public PrintSystem()
        {
            RxGlobals.Stepper.PropertyChanged += Stepper_PropertyChanged;

        }

        private void Stepper_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Changed"))
            {
                Changed |= RxGlobals.Stepper.Changed;
            } 
        }

        //--- Property Changed ---------------------------------------
        private bool _Changed = false;
        public bool Changed
        {
            get { return _Init && _Changed; }
            set { SetProperty(ref _Changed, value); }
        }

        //--- PrinterTypeList -------------------------------
        public EN_PrinterTypeList PrinterTypeList
        {
            get { return new EN_PrinterTypeList(); }
        }

        //--- SpeedList ---------------------------------------------
        /*
        private List<EN_SpeedList> _SpeedList;
        public List<EN_SpeedList> SpeedList
        {
            set { SetProperty(ref _SpeedList, value); }
            get { return _SpeedList; }
        }
        */

        public EN_SpeedList SpeedList(int dropSize)
        {
            return new EN_SpeedList(RxGlobals.PrinterStatus.MaxSpeed(dropSize));
        }

        //--- Property IsScanning ---------------------------------------
        public bool IsScanning
        {
            get 
            { 
                switch(_PrinterType)
	            {
	            case EPrinterType.printer_test_table:	    return true;
	            case EPrinterType.printer_test_slide:	    return true;
                case EPrinterType.printer_test_slide_only:  return true;
                case EPrinterType.printer_TX801:			    return true;
	            case EPrinterType.printer_TX802:			    return true;
	            default: return false;
	            }
            }
        }

        //--- Property HasHeater ---------------------------------------
        public bool HasHeater
        {
            get 
            { 
                switch(_PrinterType)
	            {
	            case EPrinterType.printer_test_table:	    return true;
                case EPrinterType.printer_cleaf:            return true;
                case EPrinterType.printer_LB701:			return true;
	            case EPrinterType.printer_LB702_UV:			return true;
	         // case EPrinterType.printer_LB702_WB:			return true;
	            default:                                    return false;
	            }
            }
        }
                      
        //--- Property PrinterType ---------------------------------------
        private EPrinterType _PrinterType;
        public EPrinterType PrinterType
        {
            get { return _PrinterType; }
            set { Changed|=SetProperty(ref _PrinterType, value); }
        }

        //--- Property ExternalData ---------------------------------------
        private bool _ExternalData;
        public bool ExternalData
        {
            get { return _ExternalData; }
            set { SetProperty(ref _ExternalData, value); }
        }
        
        //--- Property ColorCnt ---------------------------------------
        private int _ColorCnt;
        public int ColorCnt
        {
            get { return _ColorCnt; }
            set 
            { 
                Changed|=SetProperty(ref _ColorCnt, value); 
                int i;
                for (i=0; i<RxGlobals.Network.List.Count; i++)
                    RxGlobals.Network.List[i].DeviceNoList = new EN_DeviceNumbers(RxGlobals.Network.List[i].DeviceType, _ColorCnt, HeadCnt);
            }
        }
        
        //--- Property HeadCnt  ---------------------------------------
        private Int32 _HeadCnt ;
        public Int32 HeadCnt 
        {
            get { return _HeadCnt; }
            set 
            { 
                Changed|=SetProperty(ref _HeadCnt , value); 
                int i;
                for (i=0; i<RxGlobals.Network.List.Count; i++)
                    RxGlobals.Network.List[i].DeviceNoList = new EN_DeviceNumbers(RxGlobals.Network.List[i].DeviceType, ColorCnt, HeadCnt);
            }
        }

        //--- Property Reverse (head order) ---------------------------------------
        private bool _Reverse;
        public bool Reverse
        {
            get { return _Reverse; }
            set { SetProperty(ref _Reverse, value);}
        }
        
        //--- Property CheckedInkSupply ---------------------------------------
        private int _CheckedInkSupply;
        public int CheckedInkSupply
        {
            get { return _CheckedInkSupply; }
            set { SetProperty(ref _CheckedInkSupply, value); }
        }

        //--- Property AllInkSupplies ---------------------------------------
        private bool _AllInkSupplies=false;
        public bool AllInkSupplies
        {
            get { return _AllInkSupplies; }
            set { SetProperty(ref _AllInkSupplies, value); }
        }
        
        //--- Property HeadDist ---------------------------------------
        private Int32[] _HeadFpVoltage;
        public Int32[] HeadFpVoltage
        {
            get { return _HeadFpVoltage; }
            set { Changed|=SetProperty(ref _HeadFpVoltage, value); }
        }

        //--- SetFpVoltage -----------------------------
        public void SetFpVoltage(int no, int voltage)
        {
            Changed = (voltage!=_HeadFpVoltage[no]);
            _HeadFpVoltage[no] = voltage;
        }

        //--- Property HeadDist ---------------------------------------
        private double[] _HeadDist;
        public double[] HeadDist
        {
            get { return _HeadDist; }
            set { Changed|=SetProperty(ref _HeadDist, value); }
        }
        
        //--- Property HeadDistBack ---------------------------------------
        private double[] _HeadDistBack;
        public double[] HeadDistBack
        {
            get { return _HeadDistBack; }
            set { Changed|=SetProperty(ref _HeadDistBack, value); }
        }

        //--- Property ColorOffset ---------------------------------------
        private double[] _ColorOffset;
        public double[] ColorOffset
        {
            get { return _ColorOffset; }
            set { Changed|=SetProperty(ref _ColorOffset, value); }
        }

        //--- Property OffsetAngle ---------------------------------------
        private Int32 _OffsetAngle;
        public Int32 OffsetAngle
        {
            get { return _OffsetAngle; }
            set { Changed|=SetProperty(ref _OffsetAngle, value); }
        }

        //--- Property OffsetStep ---------------------------------------
        private Int32 _OffsetStep;
        public Int32 OffsetStep
        {
            get { return _OffsetStep; }
            set { Changed|=SetProperty(ref _OffsetStep, value); }
        }

        //--- Property OffsetIncPerMeter ---------------------------------------
        private int _OffsetIncPerMeter;
        public int OffsetIncPerMeter
        {
            get { return _OffsetIncPerMeter; }
            set { Changed|=SetProperty(ref _OffsetIncPerMeter, value); }
        }

        //--- Property OffsetIncPerMeter ---------------------------------------
        private int _OffsetIncPerMeterVerso;
        public int OffsetIncPerMeterVerso
        {
            get { return _OffsetIncPerMeterVerso; }
            set { Changed|=SetProperty(ref _OffsetIncPerMeterVerso, value); }
        }

        //--- Property Overlap ---------------------------------------
        private bool _Overlap;
        public bool Overlap
        {
            get { return _Overlap; }
            set { Changed|=SetProperty(ref _Overlap, value); }
        }

        //--- Property OffsetVerso ---------------------------------------
        private Int32 _OffsetVerso;
        public Int32 OffsetVerso
        {
            get { return _OffsetVerso; }
            set { Changed|=SetProperty(ref _OffsetVerso, value); }
        }
                
        //--- SetPrintCfg ----------------------------------------
        public void SetPrintCfg(TcpIp.SPrinterCfgMsg msg)
        {
            int i;
            string[] inkFileName = msg.inkFileNames.Split(',');
            for (i=0; i<inkFileName.Count(); i++)
            {
                RxGlobals.InkSupply.List[i].InkType     = RxGlobals.InkTypes.FindByFileName(inkFileName[i]);
                RxGlobals.InkSupply.List[i].RectoVerso  = msg.rectoVerso[i]; 
            }
            RxGlobals.InkSupply.List[TcpIp.InkSupplyCnt].InkType = InkType.Flush;
            RxGlobals.InkSupply.List[TcpIp.InkSupplyCnt+1].InkType = InkType.Waste;

            PrinterType             = msg.type;
            Reverse                 = (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_TX801 || RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_TX802);
            Overlap                 = msg.overlap>0;
            OffsetVerso             = msg.offset.versoDist;
            OffsetAngle             = msg.offset.angle;
            OffsetStep              = msg.offset.step;
            OffsetIncPerMeter       = msg.offset.incPerMeter;
            OffsetIncPerMeterVerso  = msg.offset.incPerMeterVerso;

            HeadCnt                 = msg.headsPerColor;

            _HeadFpVoltage= new Int32[msg.headFpVoltage.Count()];
            for (i=0; i<_HeadFpVoltage.Count(); i++) _HeadFpVoltage[i]    = msg.headFpVoltage[i];
            this.OnPropertyChanged("HeadFpVoltage");

            _HeadDist = new double[msg.headDist.Count()];
            for (i=0; i<_HeadDist.Count(); i++) _HeadDist[i]    = msg.headDist[i]/1000.0;

            _HeadDistBack = new double[msg.headDistBack.Count()];
            for (i=0; i<_HeadDistBack.Count(); i++) _HeadDistBack[i]    = msg.headDistBack[i]/1000.0;
            _ColorOffset = new double[msg.colorOffset.Count()];
            for (i=0; i<_ColorOffset.Count(); i++)
                _ColorOffset[i]     = msg.colorOffset[i]/1000.0;
            ColorCnt                = inkFileName.Count();
            ExternalData            = msg.externalData!=0;
            Changed = false;
            _Init   = true;
        }

        //--- SendMsg ------------------------------------------------------
        public void SendMsg(UInt32 msgId)
        {
            int i;
            TcpIp.SPrinterCfgMsg msg = new TcpIp.SPrinterCfgMsg();
            InkSupplyList list = RxGlobals.InkSupply;
            StringBuilder str  = new StringBuilder(1024);
            msg.rectoVerso     = new ERectoVerso[16];

            msg.type                = _PrinterType;
            msg.overlap             = Convert.ToUInt32(_Overlap);
            msg.offset.versoDist    = OffsetVerso;
            msg.offset.angle        = OffsetAngle;
            msg.offset.step         = OffsetStep;
            msg.offset.incPerMeter = OffsetIncPerMeter; 
            msg.offset.incPerMeterVerso = OffsetIncPerMeterVerso; 
            msg.headsPerColor       = _HeadCnt;
            msg.externalData        = Convert.ToInt32(ExternalData);

            if (_HeadFpVoltage!=null)
            {
                msg.headFpVoltage = new Int32[_HeadFpVoltage.Count()];
                for (i=0; i<msg.headFpVoltage.Count(); i++)
                    msg.headFpVoltage[i] = _HeadFpVoltage[i];
            }

            if (_HeadDist!=null)
            {
                msg.headDist            = new Int32[_HeadDist.Count()];
                for (i=0; i<msg.headDist.Count(); i++)
                    msg.headDist[i]     = Convert.ToInt32(_HeadDist[i]*1000);
            }

            if (_HeadDistBack!=null)
            {
                msg.headDistBack        = new Int32[_HeadDistBack.Count()];
                for (i=0; i<msg.headDistBack.Count(); i++)
                    msg.headDistBack[i]     = Convert.ToInt32(_HeadDistBack[i]*1000);
            }

            if (_ColorOffset!=null)
            {
                msg.colorOffset        = new Int32[_ColorOffset.Count()];
                for (i=0; i<msg.colorOffset.Count(); i++)
                    msg.colorOffset[i] = Convert.ToInt32(_ColorOffset[i]*1000);
            }

            for (i=0; i<ColorCnt; i++)
            {   
                InkSupply item = list.List[i];
                if (item!=null)
                {
                    if (i>0) str.Append(',');
                    if (item.InkType!=null) str.Append(item.InkType.FileName);
                    msg.rectoVerso[i] = item.RectoVerso; 
                }             
            }
            msg.inkFileNames            = str.ToString();
            
            RxGlobals.RxInterface.SendMsg(msgId, ref msg);
        }        
    }
}
