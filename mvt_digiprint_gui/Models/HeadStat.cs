using RX_Common;
using RX_DigiPrint.Converters;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Windows.Media;

namespace RX_DigiPrint.Models
{
	public class HeadStat : RxBindable
    {
        //--- Constructor ----------------------------------------
        public HeadStat()
        {
        }

        //--- Property HeadNo ---------------------------------------
        private int _HeadNo=-1;
        public int HeadNo
        {
            get { return _HeadNo; }
            private set { SetProperty(ref _HeadNo, value); }
        }

        //--- Property Connected ---------------------------------------
        private Boolean _Connected=false;
        public Boolean Connected
        {
            get { return _Connected; }
            set { SetProperty(ref _Connected, value);}
        }

        //--- Property Meniscus_Disabled ---------------------------------------
        private Boolean _Meniscus_Disabled = false;
        public Boolean Meniscus_Disabled
        {
            get { return _Meniscus_Disabled; }
            set { SetProperty(ref _Meniscus_Disabled, value); }
        }
        private Int32 _FP_Voltage = 0;
        public Int32 FP_Voltage
        {
            get { return _FP_Voltage; }
            set { SetProperty(ref _FP_Voltage, value); }
        }


        //--- Property No ---------------------------------------
        private string _Name;
        public string Name
        {
            get { return _Name; }
            set { SetProperty(ref _Name, value); }
        }

        //--- Property Color ---------------------------------------
        private Brush _Color;
        public Brush Color
        {
            get { return _Color; }
            set 
            {
                SetProperty(ref _Color, value);
            }
        }

		//--- Property InkSupply ---------------------------------------
		private InkSupply _InkSupply;
		public InkSupply InkSupply
		{
			get { return _InkSupply; }
			set { SetProperty(ref _InkSupply,value); }
		}

		//--- Property ColorFG ---------------------------------------
		private Brush _ColorFG;
        public Brush ColorFG
        {
            get { return _ColorFG; }
            set { SetProperty(ref _ColorFG, value); }
        }
        
        //--- Property Info ---------------------------------------
        private UInt32 _Info;
        public UInt32 Info
        {
            get { return _Info; }
            set { SetProperty(ref _Info, value); }
        }

        //--- Property Warn ---------------------------------------
        private bool _Warn=false;
        public bool Warn
        {
            get { return _Warn; }
            set { SetProperty(ref _Warn, value); }
        }

        //--- Property Err ---------------------------------------
        private UInt32 _Err;
        public UInt32 Err
        {
            get 
            {
                if (RxGlobals.PrinterProperties.Host_Name!=null && RxGlobals.PrinterProperties.Host_Name.StartsWith("TEST-")) return 0;
                else return _Err; 
            }
            set { SetProperty(ref _Err, value); }
        }

        //--- Property Valve ---------------------------------------
        private int _Valve;
        public int Valve
        {
            get { return _Valve; }
            set { SetProperty(ref _Valve, value); }
        }     

        //--- Property DotCnt ---------------------------------------
        private UInt64 _DotCnt;
        public UInt64 DotCnt
        {
            get { return _DotCnt; }
            set { SetProperty(ref _DotCnt, value); }
        }

        //--- Property ImgInCnt ---------------------------------------
        private UInt32 _ImgInCnt;
        public UInt32 ImgInCnt
        {
            get { return _ImgInCnt; }
            set { SetProperty(ref _ImgInCnt, value); }
        }

        //--- Property ImgBuf ---------------------------------------
        private UInt32 _ImgBuf;
        public UInt32 ImgBuf
        {
            get { return _ImgBuf; }
            set { SetProperty(ref _ImgBuf, value); }
        }

        //--- Property PrintGoCnt ---------------------------------------
        private UInt32 _PrintGoCnt;
        public UInt32 PrintGoCnt
        {
            get { return _PrintGoCnt; }
            set { SetProperty(ref _PrintGoCnt, value); }
        }

        //--- Property PrintDoneCnt ---------------------------------------
        private UInt32 _PrintDoneCnt;
        public UInt32 PrintDoneCnt
        {
            get { return _PrintDoneCnt; }
            set { SetProperty(ref _PrintDoneCnt, value); }
        }

        //--- Property TempHead ---------------------------------------
        private UInt32 _TempHead;
        public UInt32 TempHead
        {
            get { return _TempHead; }
            set { SetProperty(ref _TempHead, value); }
        }

        //--- Property TempCond ---------------------------------------
        private UInt32 _TempCond;
        public UInt32 TempCond
        {
            get { return _TempCond; }
            set { SetProperty(ref _TempCond, value); }
        }

        //--- Property TempHeater ---------------------------------------
        private UInt32 _TempHeater;
        public UInt32 TempHeater
        {
            get { return _TempHeater; }
            set { SetProperty(ref _TempHeater, value); }
        }
        //--- Property TempSetpoint ---------------------------------------
        private UInt32 _TempSetpoint;
        public UInt32 TempSetpoint
        {
            get { return _TempSetpoint; }
            set { SetProperty(ref _TempSetpoint, value); }
        }

        //--- Property TempReady ---------------------------------------
        private bool _TempReady;
        public bool TempReady
        {
            get { return _TempReady; }
            set 
            { 
                if(SetProperty(ref _TempReady, value))
                    TempReady_Brush = _TempReady? Brushes.Transparent : Rx.BrushWarn;
            }
        }

		//--- Property TempReady_Brush ---------------------------------------
		private Brush _TempReady_Brush;
		public Brush TempReady_Brush
		{
			get { return _TempReady_Brush; }
			set { SetProperty(ref _TempReady_Brush,value); }
		}


		//--- Property Property -------------------------------------------
		/*
        private UInt32 _TempFpga;
        public UInt32 TempFpga
        {
            get { return _TempFpga; }
            set { SetProperty(ref _TempFpga, value); }
        }
        */
		//--- Property Flow ---------------------------------------
		private Int32 _Flow;
        public Int32 Flow
        {
            get { return _Flow; }
            set { SetProperty(ref _Flow, value); }
        }

        //--- Property PresIn ---------------------------------------
        private Int32 _PresIn;
        public Int32 PresIn
        {
            get { return _PresIn; }
            set { SetProperty(ref _PresIn, value); }
        }

        //--- Property PresIn2 ---------------------------------------
        private Int32 _PresIn2;
        public Int32 PresIn2
        {
            get { return _PresIn2; }
            set { SetProperty(ref _PresIn2, value); }
        }

        //--- Property PresMax ---------------------------------------
        private Int32 _PresIn_max;
        public Int32 PresIn_max
        {
            get { return _PresIn_max; }
            set { SetProperty(ref _PresIn_max, value); }
        }

        //--- Property PresIn ---------------------------------------
        private Int32 _PresIn_diff;
        public Int32 PresIn_diff
        {
            get { return _PresIn_diff; }
            set { SetProperty(ref _PresIn_diff, value); }
        }

        //--- Property PresIn_str ---------------------------------------
        private string _PresIn_str;
        public string PresIn_str
        {
            get { return _PresIn_str; }
            set 
            {
                SetProperty(ref _PresIn_str, value);
                OnPropertyChanged("");
            }
        }

        //--- Property PresOut ---------------------------------------
        private Int32 _PresOut;
        public Int32 PresOut
        {
            get { return _PresOut; }
            set { SetProperty(ref _PresOut, value); }
        }

        //--- Property PresOut_diff ---------------------------------------
        private Int32 _PresOut_diff;
        public Int32 PresOut_diff
        {
            get { return _PresOut_diff; }
            set { SetProperty(ref _PresOut_diff, value); }
        }

        //--- Property Meniscus ---------------------------------------
        private Int32 _Meniscus;
        public Int32 Meniscus
        {
            get { return _Meniscus; }
            set { SetProperty(ref _Meniscus, value); }
        }
        
        //--- Property Meniscus_diff ---------------------------------------
        private Int32 _Meniscus_diff;
        public Int32 Meniscus_diff
        {
            get { return _Meniscus_diff; }
            set { SetProperty(ref _Meniscus_diff, value); }
        }

        //--- Property Meniscus_setpoint ---------------------------------------
        private Int32 _Meniscus_setpoint;
        public Int32 Meniscus_setpoint
        {
            get { return _Meniscus_setpoint; }
            set { SetProperty(ref _Meniscus_setpoint, value); }
        }

		//--- Property FlowFactor ---------------------------------------
		private Int32 _FlowFactor;
		public Int32 FlowFactor
		{
			get { return _FlowFactor; }
			set { SetProperty(ref _FlowFactor,value); }
		}

		//--- Property FlowFactorWarning ---------------------------------------
		private bool _FlowFactorWarning=false;
		public bool FlowFactorWarning
		{
			get { return _FlowFactorWarning; }
			set { SetProperty(ref _FlowFactorWarning,value); }
		}

		//--- Property PumpSpeed ---------------------------------------
		private UInt32 _PumpSpeed;
        public UInt32 PumpSpeed
        {
            get { return _PumpSpeed; }
            set { SetProperty(ref _PumpSpeed, value); }
        }

        //--- Property PumpFeedback ---------------------------------------
        private UInt32 _PumpFeedback;
        public UInt32 PumpFeedback
        {
            get 
            {
                if (_PumpFeedback==TcpIp.INVALID_VALUE) return TcpIp.INVALID_VALUE;
                else                                    return _PumpFeedback; 
            }
            set { SetProperty(ref _PumpFeedback, value); }
        }

        private double _FlowResistance;
        public double FlowResistance
        {
            get { return _FlowResistance; }
            set { SetProperty(ref _FlowResistance, value); }
        }

        //--- Property PrintingTime ---------------------------------------
        private string _PrintingTime="***";
        public string PrintingTime
        {
            get { return _PrintingTime; }
            set { SetProperty(ref _PrintingTime, value); }
        }

		//--- Property CtrlMode ---------------------------------------
		private EFluidCtrlMode _CtrlMode;
        public EFluidCtrlMode CtrlMode
        {
            get { return _CtrlMode; }
            set { SetProperty(ref _CtrlMode, value); }
        }
        
        //--- Property FluidCtrlModeList ---------------------------------------
        private static EN_FluidCtrlList _FluidCtrlModeList = new EN_FluidCtrlList();
	    public EN_FluidCtrlList FluidCtrlModeList
	    {
		    get { return _FluidCtrlModeList;}
	    }

        //--- Property ClusterNo ----------------------------------------------------
        private UInt32 _ClusterNo;
        public UInt32 ClusterNo
        {
            get { return _ClusterNo; }
            set { SetProperty(ref _ClusterNo, value); }
        }

        //--- Property Cooler_Pressure ----------------------------------------
        private Int32 _Cooler_Pressure = 0;
        public Int32 Cooler_Pressure
        {
            get { return _Cooler_Pressure; }
            set { SetProperty(ref _Cooler_Pressure, value); }
        }

        //--- Property Cooler_Temp ---------------------------------
        private UInt32 _Cooler_Temp = 0;
        public UInt32 Cooler_Temp
        {
            get { return _Cooler_Temp; }
            set { SetProperty(ref _Cooler_Temp, value); }
        }

        //--- DensityValue ---------------------------------------------------
    //  private Byte? _DensityValuesCRC;
        public Int16[] DensityValue = new Int16[TcpIp.MAX_DENSITY_VALUES];

        //--- DensityValue ---------------------------------------------------
    //  private Byte? _DisabledJetsCRC;
        public UInt16[] DisabledJets = new UInt16[TcpIp.MAX_DISABLED_JETS];

        //--- Property Voltage ---------------------------------------
        private Byte _Voltage;
        public Byte Voltage
        {
            get { return _Voltage; }
            set { SetProperty(ref _Voltage, value); }
        }

        //--- Property Drive_Voltage ---------------------------------------
        private UInt16 _Drive_Voltage;
        public UInt16 Drive_Voltage
        {
            get { return _Drive_Voltage; }
            set { SetProperty(ref _Drive_Voltage, value); }
        }

        //--- Property StateBrush ---------------------------------------
        private Brush _StateBrush;
		public Brush StateBrush
		{
			get { return _StateBrush; }
			set { 
                    if (SetProperty(ref _StateBrush,value))
				    {
                        if (InkSupply!=null) InkSupply.UpdateStateBrush(_StateBrush);
				    }
                }
		}

		//--- SetItem ----------------------------------------------
		public void SetItem(int no, TcpIp.SHeadStat item, Int32 tempFpga, Int32 flow)
        {   
            bool used=false;
            HeadNo  = no;

            for (int i = 0; i < DensityValue.Length; i++) DensityValue[i] = item.eeprom_mvt.densityValue[i];
            for (int i = 0; i < DisabledJets.Length; i++) DisabledJets[i] = item.eeprom_mvt.disabledJets[i];
            Voltage = item.eeprom_mvt.voltage;

            try
            {
                int ink;
                if (RxGlobals.PrintSystem.HeadsPerColor!=0) ink = no/RxGlobals.PrintSystem.HeadsPerColor;
                else ink=no;
                if (ink< RxGlobals.InkSupply.List.Count)
                {
                    InkSupply = RxGlobals.InkSupply.List[ink];
                    if (InkSupply!=null && InkSupply.InkType!=null)
                    {
                        Color   = new SolidColorBrush(InkSupply.InkType.Color);
                        ColorFG = new SolidColorBrush(InkSupply.InkType.ColorFG);

                        string str = new ColorCode_Str().Convert(InkSupply.InkType.ColorCode, null, ink, null).ToString();

                        Name    = str+"-"+(1+no%(int)RxGlobals.PrintSystem.HeadsPerColor).ToString();
                        used = true;
                    }
                }
            }
            catch(Exception)
            {
                Color   = Brushes.Transparent;
                ColorFG = Brushes.Black;
                Name    = "?-"+(1+no).ToString();
            }

            Connected   = (item.info&0x00000001) != 0;
            Info        = item.info;
            Err         = item.err;
            
            Valve       = (int)(item.info & 0x06)>>1;

            DotCnt      = item.dotCnt;
            ImgInCnt    = item.imgInCnt;
            ImgBuf      = item.imgBuf;
            PrintGoCnt  = item.printGoCnt;
            PrintDoneCnt= item.printDoneCnt;

            ClusterNo = item.eeprom_mvt.clusterNo;

            // Conditioner
            TempHead    = item.tempHead;
            TempCond    = item.tempCond;
            TempHeater  = item.tempHeater;
            TempSetpoint= item.tempSetpoint;
         //   if (used) TempReady   = item.tempReady!=0;
         //   else TempReady=false;
            TempReady   = (!used) || (item.ctrlMode!=EFluidCtrlMode.ctrl_print) || ((item.info&(1<<5))!=0);
         // if (no==0) Console.WriteLine("Head[0].PresIn={0}, PresIn2={1}", item.presIn, item.presIn2);
            PresIn      = item.presIn;
            PresIn2     = item.presIn2;
            PresIn_max  = item.presIn_max;
            PresIn_diff = item.presIn_diff;
            if (item.presIn_max==TcpIp.INVALID_VALUE) PresIn_str = string.Format("~{0}", HeadVal_Converter10._convert(PresIn_diff));
            else                                      PresIn_str = string.Format("^{0}", HeadVal_Converter10._convert(PresIn_max)); 
            PresOut      = item.presOut;
            PresOut_diff = item.presOut_diff;
            Meniscus     = item.meniscus;
            Meniscus_diff= item.meniscus_diff;
            PumpSpeed    = item.pumpSpeed;
            FlowFactor   = item.flowFactor;
            FlowFactorWarning = (CtrlMode==EFluidCtrlMode.ctrl_print) && (item.info&(1<<6))==0;

            Warn         = used && (FlowFactorWarning || !TempReady);
            if (Err!=0)    StateBrush = Rx.BrushError;
            else if (Warn) StateBrush = Rx.BrushWarn;
            else           StateBrush = Brushes.Transparent; 

            Meniscus_setpoint = item.meniscus_Setpoint;
            PumpFeedback= item.pumpFeedback;
            if (item.printingSeconds==TcpIp.INVALID_VALUE) PrintingTime="-----";
            else
            {
                UInt32 h, m, s;
                s = item.printingSeconds%60;
                m = item.printingSeconds/60;
                h = m/60;
                m = m%60;
                PrintingTime = string.Format("{0}:{1:00}:{2:00}", h, m, s);
            }
            CtrlMode    = item.ctrlMode;

            Flow        = flow;

            FlowResistance = 0.0;
            int pressureDifference = PresIn - PresOut;
            if (PumpFeedback != 0)
            {
                FlowResistance = pressureDifference / PumpFeedback;
            }
        }

    }
}
