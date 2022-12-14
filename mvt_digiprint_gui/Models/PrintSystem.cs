using RX_Common;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Media;

namespace RX_DigiPrint.Models
{
	public class PrintSystem : RxBindable
    {
        private bool _Init=false;

        //--- Constructor ------------------------------------------
        public PrintSystem()
        {
            RxGlobals.Stepper.PropertyChanged += Stepper_PropertyChanged;
            RxGlobals.Chiller.PropertyChanged += Chiller_PropertyChanged;
        }

        private void Chiller_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Enabled"))
            {
                ChillerEnabled = RxGlobals.Chiller.Enabled;
            }
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

        //--- EncoderTypeList -------------------------------
        public EN_EncoderTypeList EncoderTypeList
        {
            get { return new EN_EncoderTypeList(); }
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

        public EN_SpeedList SpeedList(int dropSize, double imgHeight)
        {
            return new EN_SpeedList(RxGlobals.PrinterStatus.MaxSpeed(dropSize), imgHeight);
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
                case EPrinterType.printer_test_slide_HB:    return true;
                case EPrinterType.printer_TX801:			return true;
	            case EPrinterType.printer_TX802:			return true;
	            case EPrinterType.printer_TX404:            return true;
	            case EPrinterType.printer_test_table_seon:  return true;
                case EPrinterType.printer_Dropwatcher:      return true;
                default: return false;
	            }
            }
        }

        //--- Property IsTx ---------------------------------------
        public bool IsTx
        {
            get
            {
                switch (_PrinterType)
                {
                    case EPrinterType.printer_TX801: return true;
                    case EPrinterType.printer_TX802: return true;
                    case EPrinterType.printer_TX404: return true;
                    default: return false;
                }
            }
        }

        //--- Property IsLb ---------------------------------------
        public bool IsLb
        {
            get
            {
                switch (_PrinterType)
                {
                    case EPrinterType.printer_LB701:    return true;
                    case EPrinterType.printer_LB702_UV: return true;
                    case EPrinterType.printer_LB702_WB: return true;
                    case EPrinterType.printer_LH702:    return true;
                    default: return false;
                }
            }
        }

        //--- Property IsClnUsed ---------------------------------------
		private bool _IsClnUsed=false;
		public bool IsClnUsed
		{
			get { return _IsClnUsed; }
			set { SetProperty(ref _IsClnUsed,value); }
		}

		//--- Property IsRobotConnected ---------------------------------------
		private bool _IsRobotConnected=false;
		public bool IsRobotConnected
		{
			get { return _IsRobotConnected; }
			set { SetProperty(ref _IsRobotConnected,value); }
		}

		//--- Property IsCLEAF ---------------------------------------
		public bool IsCLEAF
        {
            get
            {
                switch (_PrinterType)
                {
                    case EPrinterType.printer_cleaf:            return true;
                    default: return false;
                }
            }
        }

        //--- Property HasHeater ---------------------------------------
        public bool HasHeater
        {
            //TODO(CB612) Add case CB612

            get 
            { 
                switch(_PrinterType)
	            {
                case EPrinterType.printer_test_slide_HB:    return true;
	            case EPrinterType.printer_test_table:	    return true;
                case EPrinterType.printer_cleaf:            return true;
                case EPrinterType.printer_LB701:			return true;
	            case EPrinterType.printer_LB702_UV:			return true;
	            case EPrinterType.printer_LH702:			return true;
                case EPrinterType.printer_Dropwatcher:      return true;
	            default:                                    return false;
	            }
            }
        }

        //--- Property HostName ---------------------------------------
        private string _HostName;
        public string HostName
        {
            get { return _HostName; }
            set { SetProperty(ref _HostName, value); }
        }

        //--- Property EncoderType ---------------------------------------
        private EEncoderType _EncoderType;
        public EEncoderType EncoderType
        {
            get { return _EncoderType; }
            set { SetProperty(ref _EncoderType, value); }
        }

        //--- Property PrinterType ---------------------------------------
        private EPrinterType _PrinterType;
        public EPrinterType PrinterType
        {
            get { return _PrinterType; }
            set 
            { 
                if (SetProperty(ref _PrinterType, value))
                {
                    Changed=true;
                    _set_IS_Order();
                }
            }
        }

        //--- Property TTS_simulation ---------------------------------------
        private bool _TTS_simulation = false;
        public bool TTS_simulation
        {
            get { return _TTS_simulation; }
            set { SetProperty(ref _TTS_simulation, value); }
        }

        //--- Property LH702_simulation ---------------------------------------
        private bool _LH702_simulation = false;
        public bool LH702_simulation
        {
            get { return _LH702_simulation; }
            set { SetProperty(ref _LH702_simulation, value); }
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
                Changed|=SetProperty(ref _ColorCnt, Math.Max(value, 1));
                _set_IS_Order();
                int i;
                for (i=0; i<RxGlobals.Network.List.Count; i++)
                    RxGlobals.Network.List[i].DeviceNoList = new EN_DeviceNumbers(RxGlobals.Network.List[i].DeviceType, _ColorCnt, HeadsPerColor);
            }
        }

        private int _InkSuppliesPerColor;
        public int InkCylindersPerColor
        {
            get { return _InkSuppliesPerColor; }
            set
            {
                Changed |= SetProperty(ref _InkSuppliesPerColor, Math.Max(value,1));
            }
        }

        //--- Property HeadsPerColor ---------------------------------------
        private int _HeadsPerColor;
        public int HeadsPerColor
        {
            get { return (_HeadsPerColor==0)? 1:_HeadsPerColor; }
            set { Changed |= SetProperty(ref _HeadsPerColor, Math.Max(value,1 )); }
        }

        //--- HeadsPerInkCylinder -----------------------------------
        public int HeadsPerInkCylinder
        {
            get 
            { 
                if (_HeadsPerColor==0 || _InkSuppliesPerColor==0) return 1;
                else return _HeadsPerColor / _InkSuppliesPerColor;
            }
        }

        //--- Cluster ---------------------
        public int Cluster(int head)
        {
            return (head+TcpIp.HEAD_CNT-1)/TcpIp.HEAD_CNT;
        }

        //--- Property Color_Order ---------------------------------------
        private int[] _Color_Order;
        public int[] Color_Order
        {
            get { return _Color_Order; }
            private set { SetProperty(ref _Color_Order, value); }
        }

        //--- Property IS_Order ---------------------------------------
        private int[] _IS_Order;
        public int[] IS_Order
        {
            get { return _IS_Order; }
            private set { SetProperty(ref _IS_Order,value); }
        }

        //--- _set_IS_Order ---------------------------------------
        private EPrinterType _IsOrder_PrinterType = EPrinterType.printer_undef;
        private int IsOrder_ColorCnt = 0;
        private void _set_IS_Order()
		{
            if (IS_Order==null || Color_Order == null || PrinterType!=_IsOrder_PrinterType || ColorCnt!=IsOrder_ColorCnt)
			{
                _IsOrder_PrinterType = PrinterType;
                IsOrder_ColorCnt = ColorCnt;
                switch(_PrinterType)
                {
                    /*
                    case EPrinterType.printer_DP803: IS_Order = new int[] { 0,1,2,3,4,5,6,7 }; break;

                    case EPrinterType.printer_LB701: 
                        if (RxGlobals.PrintSystem.ColorCnt<=4) IS_Order = new int[] { 0,1,2,3 };
                        else IS_Order = new int[] { 0,1,2,3,4,5,6,7 };
                        break;
                    */

                    case EPrinterType.printer_LB701:
                    case EPrinterType.printer_LB702_UV:
                    case EPrinterType.printer_LH702:
                        switch(ColorCnt)
                        {
                            case 5:
                                IS_Order    = new int[] { 4, 0, 1, 2, 3 };
                                Color_Order = new int[] { 4, 0, 1, 2, 3 };
                                break;
                            case 6:
                                IS_Order    = new int[] { 4, 5, 0, 1, 2, 3 };
                                Color_Order = new int[] { 4, 5, 0, 1, 2, 3 };
                                break;
                            case 7:
                                IS_Order    = new int[] { 4, 5, 6, 0, 1, 2, 3 };
                                Color_Order = new int[] { 4, 5, 6, 0, 1, 2, 3 };
                                break;
                            case 8:
                                IS_Order    = new int[] { 4, 5, 6, 7, 0, 1, 2, 3 };
                                Color_Order = new int[] { 4, 5, 6, 7, 0, 1, 2, 3 };
                                break;
                            default:
                                IS_Order    = new int[] { 0, 1, 2, 3, 4, 5, 6, 7 };
                                Color_Order = new int[] { 0, 1, 2, 3, 4, 5, 6, 7 };
                                break;
                        }                        break;

                    case EPrinterType.printer_LB702_WB:
                        IS_Order    = new int[] { 0, 1, 2, 3, 4, 5, 6, 7 };
                        Color_Order = new int[] { 0, 1, 2, 3, 4, 5, 6, 7 };                        
                        break;
                    case EPrinterType.printer_TX801:
                        IS_Order    = new int[] { 7, 6, 5, 4, 3, 2, 1, 0 };
                        Color_Order = new int[] { 7, 6, 5, 4, 3, 2, 1, 0 };
                        break;
                    case EPrinterType.printer_TX802:
                        IS_Order    = new int[] { 7, 6, 5, 4, 3, 2, 1, 0 };
                        Color_Order = new int[] { 7, 6, 5, 4, 3, 2, 1, 0 };
                        break;
                    case EPrinterType.printer_TX404:
                        if (ColorCnt <= 4)
						{
                            IS_Order    = new int[] { 3, 2, 1, 0 };
                            Color_Order = new int[] { 3, 2, 1, 0 };
						}
                        else
						{
                            IS_Order    = new int[] { 7, 6, 5, 4, 3, 2, 1, 0 };
                            Color_Order = new int[] { 7, 6, 5, 4, 3, 2, 1, 0 };
						}
                        break;

                    case EPrinterType.printer_CB612:
                        IS_Order    = new int[] { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23 };
                        Color_Order = new int[] { 1, 0, 2, 3, 4, 5 }; // order: to be defined !!!
                        break;

                    default:
                        IS_Order    = new int[] { 0, 1, 2, 3, 4, 5, 6, 7 };
                        Color_Order = new int[] { 0, 1, 2, 3, 4, 5, 6, 7 }; 
                        break;
                }
			}
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
        private double[] _HeadDist; // in mm
        public double[] HeadDist
        {
            get { return _HeadDist; }
            set { Changed|=SetProperty(ref _HeadDist, value); }
        }

        //--- Property HeadDistBack ---------------------------------------
        private double[] _HeadDistBack; // in mm
        public double[] HeadDistBack
        {
            get { return _HeadDistBack; }
            set { Changed|=SetProperty(ref _HeadDistBack, value); }
        }

        //--- Property ColorOffset ---------------------------------------
        private double[] _ColorOffset; // in mm
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

        //--- Property ManualFlightTimeComp ---------------------------------------
        private double _ManualFlightTimeComp ;
        public double ManualFlightTimeComp 
        {
            get { return _ManualFlightTimeComp ; }
            set { Changed|=SetProperty(ref _ManualFlightTimeComp , value); }
        }

        // Property ExpandSettingsPanel
        private bool _ExpandSettingsPanel;
        public bool ExpandSettingsPanel
        {
            get { return _ExpandSettingsPanel; }
            set { SetProperty(ref _ExpandSettingsPanel, value); }
        }

        // Property ChillerEnabled
        private bool _ChillerEnabled;
        public bool ChillerEnabled
        {
            get { return _ChillerEnabled; }
            set { SetProperty(ref _ChillerEnabled, value); }
        }

        //--- Property StateBrush ---------------------------------------
		private Brush _StateBrush=Brushes.Transparent;
		public Brush StateBrush
		{
			get { return _StateBrush; }
			set { SetProperty(ref _StateBrush,value); }
		}

        //--- UpdateStateBrush -----------------------------------------
        public void UpdateStateBrush()
		{
            Brush brush=Brushes.Transparent;
            foreach(InkSupply inkSupply in RxGlobals.InkSupply.List)
			{
                if (inkSupply.StateBrush==Rx.BrushError)
				{
                    StateBrush=inkSupply.StateBrush;
                    return;
				}
                if (inkSupply.StateBrush==Rx.BrushWarn) brush=inkSupply.StateBrush;
			}
            StateBrush = brush;
		}

        //--- ReadyToPrint ------------------------------------------------
		public bool ReadyToPrint()
		{
			if (InkSupply.AnyFlushed()) return false;

			if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_cleaf && !(RxGlobals.StepperStatus[0].DripPans_InfeedDOWN && RxGlobals.StepperStatus[0].DripPans_OutfeedDOWN))
			{
				MvtMessageBox.YesNo("Print System", "Drip Pans below the clusters. Move it out before printing", MessageBoxImage.Question, true);
				return false;
			}

			if (!RxGlobals.PrinterStatus.AllInkSupliesOn)
			{
                if (RxGlobals.PrintSystem.IsClnUsed)
				{
				    if (!MvtMessageBox.YesNo("Print System", "Some ink supplies are OFF. \n\nStart Printing?", MessageBoxImage.Question, false))
                        return false;
				}
                else
				{
				    if (MvtMessageBox.YesNo("Print System", "Some ink supplies are OFF. Switch them ON.", MessageBoxImage.Question, true))
				    {
					    TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
					    msg.no = -1;
					    msg.ctrlMode = EFluidCtrlMode.ctrl_prepareToPrint;

					    RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
				    }
				}
			}

			if (RxGlobals.UvLamp.Visible == Visibility.Visible && !RxGlobals.UvLamp.Ready)
			{
				if (!MvtMessageBox.YesNo("UV Lamp", "The UV Lamp is NOT READY.\n\nStart Printing?", MessageBoxImage.Question, false))
					return false;
			}

			return true;
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

            LH702_simulation = !msg.hostName.StartsWith("LH702");
            TTS_simulation = !msg.hostName.StartsWith("TTS");

            HostName = msg.hostName;

            PrinterType             = msg.type;
            EncoderType             = msg.encoderType;
            Overlap                 = msg.overlap>0;
            OffsetVerso             = msg.offset.versoDist;
            ManualFlightTimeComp    = (msg.offset.manualFlightTimeComp/1000.0);
            OffsetAngle             = msg.offset.angle;
            OffsetStep              = msg.offset.step;
            OffsetIncPerMeter       = msg.offset.incPerMeter;
            OffsetIncPerMeterVerso  = msg.offset.incPerMeterVerso;

            ColorCnt                = msg.colorCnt;
            InkCylindersPerColor    = msg.InkCylindersPerColor;
            HeadsPerColor           = msg.headsPerColor;

            _HeadFpVoltage= new Int32[msg.headFpVoltage.Count()];
            for (i=0; i<_HeadFpVoltage.Count(); i++) _HeadFpVoltage[i]    = msg.headFpVoltage[i];
            this.OnPropertyChanged("HeadFpVoltage");
            
            _HeadDist = new double[msg.headDist.Count()];
            for (i = 0; i < _HeadDist.Count(); i++)
            {
                _HeadDist[i] = msg.headDist[i] / 1000.0;
            }
            
            _HeadDistBack = new double[msg.headDistBack.Count()];
            for (i = 0; i < _HeadDistBack.Count(); i++)
            {
                _HeadDistBack[i] = msg.headDistBack[i] / 1000.0;
            }

            _ColorOffset = new double[msg.colorOffset.Count()];
            for (i = 0; i < _ColorOffset.Count(); i++)
            {
                _ColorOffset[i] = msg.colorOffset[i] / 1000.0;                
            }
            this.OnPropertyChanged("CorrectionValuesSet"); // inform Alignment.cs
            
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
            msg.rectoVerso     = new ERectoVerso[24];

            if (RxGlobals.PrintSystem.ColorCnt * RxGlobals.PrintSystem.InkCylindersPerColor > TcpIp.InkSupplyCnt) // max = 24!
            {
                RxGlobals.Events.AddItem(new LogItem("Configuration missmatch!! To many ink supplies configured"));
                return;
            }

            msg.type                    = _PrinterType;
            msg.encoderType             = _EncoderType;
            msg.overlap                 = Convert.ToUInt32(_Overlap);
            msg.offset.versoDist        = OffsetVerso;
            msg.offset.manualFlightTimeComp = Convert.ToInt32(ManualFlightTimeComp*1000);
            msg.offset.angle            = OffsetAngle;
            msg.offset.step             = OffsetStep;
            msg.offset.incPerMeter      = OffsetIncPerMeter; 
            msg.offset.incPerMeterVerso = OffsetIncPerMeterVerso;

            msg.colorCnt                = ColorCnt;
            msg.headsPerColor           = HeadsPerColor;
            msg.inkSupplyCnt            = ColorCnt*InkCylindersPerColor;
            msg.InkCylindersPerColor    = InkCylindersPerColor;

            msg.externalData            = Convert.ToInt32(ExternalData);
            
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

            for (i = 0; i < ColorCnt * InkCylindersPerColor; i++)
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
