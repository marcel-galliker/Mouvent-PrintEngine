using RX_Common;
using RX_DigiPrint.Converters;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Collections.ObjectModel;
using System.Windows.Media;

namespace RX_DigiPrint.Models
{
    public class InkSupply  : RxBindable
    {
        private static InkTypeList _InkTypeList = new InkTypeList();

        //--- Constructor -------------------
        public InkSupply()
        {
            
        }

        //--- Property InkTypeList ---------------------------------------
        public static ObservableCollection<InkType> InkTypeList
        {
            get { return _InkTypeList.List; }
        }

        //--- Property No ---------------------------------------
        private int _No;
        public int No
        {
            get { return _No; }
            set { SetProperty(ref _No, value); }
        }

        //--- Property Connected ---------------------------------------
        private Boolean _Connected=false;
        public Boolean Connected
        {
            get { return _Connected; }
            set { SetProperty(ref _Connected, value); }
        }       

        //--- Property Info ---------------------------------------
        private UInt32 _Info;
        public UInt32 Info
        {
            get { return _Info; }
            set { SetProperty(ref _Info, value); }
        }

        //--- Property Warn ---------------------------------------
        private bool _Warn;
        public bool Warn
        {
            get { return _Warn; }
            set { SetProperty(ref _Warn, value); }
        }

        //--- Property Err ---------------------------------------
        private UInt32 _Err;
        public UInt32 Err
        {
            get { return _Err; }
            set { SetProperty(ref _Err, value); }
        }

        //--- Property CanisterLevel ---------------------------------------
        private Int32 _CanisterLevel;
        public Int32 CanisterLevel
        {
            get { return _CanisterLevel; }
            set { SetProperty(ref _CanisterLevel, value); }
        }

        //--- Property CanisterErr ---------------------------------------
        private ELogType _CanisterErr;
        public ELogType  CanisterErr
        {
            get { return _CanisterErr; }
            set { SetProperty(ref _CanisterErr, value); }
        }

        //--- Property BarCode ---------------------------------------
        private string _BarCode;
        public string BarCode
        {
            get { return _BarCode; }
            set { SetProperty(ref _BarCode, value); }
        }

        //--- Property ScannerSN ---------------------------------------
        private string _ScannerSN;
        public string ScannerSN
        {
            get { return _ScannerSN; }
            set { SetProperty(ref _ScannerSN, value); }
        }
        
        //--- Property CylinderPresSet ---------------------------------------
        private Int32 _CylinderPresSet;
        public Int32 CylinderPresSet
        {
            get { return _CylinderPresSet; }
            set { SetProperty(ref _CylinderPresSet, value); }
        }

        //--- Property CylinderPres ---------------------------------------
        private Int32 _CylinderPres;
        public Int32 CylinderPres
        {
            get { return _CylinderPres; }
            set { SetProperty(ref _CylinderPres, value); }
        }

		//--- Property CylinderPresDiff ---------------------------------------
		private int _CylinderPresDiff;
		public int CylinderPresDiff
		{
			get { return _CylinderPresDiff; }
			set { SetProperty(ref _CylinderPresDiff,value); }
		}

		//--- Property PresIntTank ---------------------------------------
		private Int32 _CylinderSetpoint;
        public Int32 CylinderSetpoint
        {
            get { return _CylinderSetpoint; }
            set { SetProperty(ref _CylinderSetpoint, value); }
        }

        //--- Property CondPresOut ---------------------------------------
        private Int32 _CondPresOut;
        public Int32 CondPresOut
        {
            get { return _CondPresOut; }
            set { SetProperty(ref _CondPresOut, value); }
        }

        //--- Property CondPresOut ---------------------------------------
        private Int32 _CondPresIn;
        public Int32 CondPresIn
        {
            get { return _CondPresIn; }
            set { SetProperty(ref _CondPresIn, value); }
        }

        //--- Property CondPumpSpeed ---------------------------------------
        private Int32 _CondPumpSpeed;
        public Int32 CondPumpSpeed
        {
            get { return _CondPumpSpeed; }
            set { SetProperty(ref _CondPumpSpeed, value); }
        }

        //--- Property FlushTime ---------------------------------------
        private int _FlushTime;
        public int FlushTime
        {
            get { return _FlushTime; }
            set { SetProperty(ref _FlushTime, value); }
        }

        //--- Property PresLung ---------------------------------------
        private Int32 _PresLung;
        public Int32 PresLung
        {
            get { return _PresLung; }
            set { SetProperty(ref _PresLung, value); }
        }

        //--- Property TempSetpoint ---------------------------------------
        private double _TempSetpoint;
        public double TempSetpoint
        {
            get { return _TempSetpoint; }
            set { SetProperty(ref _TempSetpoint, value); }
        }

        //--- Property TempSetpoint ---------------------------------------
        private double _TempMaxSetpoint;
        public double TempMaxSetpoint
        {
            get { return _TempMaxSetpoint; }
            set { SetProperty(ref _TempMaxSetpoint, value); }
        }  

        //--- Property Temp ---------------------------------------
        private Int32 _Temp;
        public Int32 Temp
        {
            get { return _Temp; }
            set { SetProperty(ref _Temp, value); }
        }

        //--- Property TempReady ---------------------------------------
        private bool _TempReady;
        public bool TempReady
        {
            get { return _TempReady; }
            set { SetProperty(ref _TempReady, value); }
        }

		//--- Property FlowFactortOk ---------------------------------------
		private bool _FlowFactorOk;
		public bool FlowFactorOk    
		{
			get { return _FlowFactorOk; }
			set { SetProperty(ref _FlowFactorOk,value); }
		}

		//--- Property CondTemp ---------------------------------------
		private Int32 _CondTemp;
        public Int32 CondTemp
        {
            get { return _CondTemp; }
            set { SetProperty(ref _CondTemp, value); }
        }
                
        //--- Property CondTempReady ---------------------------------------
        private bool _CondTempReady;
        public bool CondTempReady
        {
            get { return _CondTempReady; }
            set { SetProperty(ref _CondTempReady, value); }
        }
        
        //--- Property PumpSpeedSet ---------------------------------------
        private Int32 _PumpSpeedSet;
        public Int32 PumpSpeedSet
        {
            get { return _PumpSpeedSet; }
            set { SetProperty(ref _PumpSpeedSet, value); }
        }

        //--- Property PumpSpeed ---------------------------------------
        private Int32 _PumpSpeed;
        public Int32 PumpSpeed
        {
            get { return _PumpSpeed; }
            set { SetProperty(ref _PumpSpeed, value); }
        }

        //--- Property BleedValve ---------------------------------------
        private string _BleedValve;
        public string BleedValve
        {
            get { return _BleedValve; }
            set { SetProperty(ref _BleedValve, value); }
        }

        //--- Property AirCusionValve ---------------------------------------
        private string _AirCusionValve;
        public string AirCusionValve
        {
            get { return _AirCusionValve; }
            set { SetProperty(ref _AirCusionValve, value); }
        }
        
        //--- Property CtrlMode ---------------------------------------
        private EFluidCtrlMode _CtrlMode;
        public EFluidCtrlMode CtrlMode
        {
            get { return _CtrlMode; }
            set 
            { 
                if (SetProperty(ref _CtrlMode, value))
                {
                    RxGlobals.PrintSystem.Changed |= (_CtrlMode==EFluidCtrlMode.ctrl_cal_start);
                }
            }
        }        

        //--- Property Flushed ---------------------------------------
        private bool _Flushed = true;
        public bool Flushed
        {
            get { return _Flushed; }set 
            { 
               if (SetProperty(ref _Flushed, value))
               {
                    Console.WriteLine("InkSupply[{0}].Flushed={1}", No, value);
               }
            }
        }
        
        //--- AnyFlushed -------------------------------------------
        public static bool AnyFlushed()
		{
            foreach(InkSupply inkSupply in RxGlobals.InkSupply.List)
			{
                if (inkSupply.InkType!=null && inkSupply.Connected && inkSupply.Flushed)
				{
                    MvtMessageBox.Information("Print System", "At least one ink supply is flushed. Please purge first.");
                    return true;
				}
			}
            return false;
		}

        //--- Property FluidCtrlModeList ---------------------------------------
        private static EN_FluidCtrlList _FluidCtrlModeList = new EN_FluidCtrlList();
	    public EN_FluidCtrlList FluidCtrlModeList
	    {
		    get { return _FluidCtrlModeList;}
	    }

        //--- Property InkType ---------------------------------------
        private InkType _InkType;
        public InkType InkType
        {
            get { return _InkType; }
            set 
            { 
                if (SetProperty(ref _InkType, value))
                {
                    if (_InkType==null) TempSetpoint = 0;
                    else                TempSetpoint = _InkType.Temp;
                    if (_InkType == null) TempMaxSetpoint = 0;
                    else TempMaxSetpoint = _InkType.TempMax;
                }
            }
        }

        //--- Property RectoVerso ---------------------------------------
        private ERectoVerso _RectoVerso=ERectoVerso.rv_undef;
        public ERectoVerso RectoVerso
        {
            get { return _RectoVerso; }
            set { SetProperty(ref _RectoVerso, value);}
        }

        //--- Property IsChecked ---------------------------------------
        private bool _IsChecked;
        public bool IsChecked
        {
            get { return _IsChecked; }
            set 
            { 
                SetProperty(ref _IsChecked, value); 
                if (_IsChecked) RxGlobals.PrintSystem.CheckedInkSupply=_No-1;
            }
        }   

        //--- Property StateBrush ---------------------------------------
		private Brush _StateBrush;
		public Brush StateBrush
		{
			get { return _StateBrush; }
			set 
            { 
                if (SetProperty(ref _StateBrush, value))
				{
                    RxGlobals.PrintSystem.UpdateStateBrush();
				}
            }
		}

        //--- UpdateStateBrush -------------------------------------------
        public void UpdateStateBrush(Brush brush)
		{
            if      (brush==Rx.BrushError) StateBrush=Rx.BrushError;
            else if (brush==Rx.BrushWarn && StateBrush!=Rx.BrushError) StateBrush=Rx.BrushWarn;
            else 
			{
                brush=Brushes.Transparent;
                foreach(HeadStat head in RxGlobals.HeadStat.List)
				{
                    if (head.InkSupply==this)
					{
                        if (head.StateBrush==Rx.BrushError)
						{
                            brush=Rx.BrushError;
                            break;
						}
                        else if (head.StateBrush==Rx.BrushWarn)
						{
                            brush=Rx.BrushWarn;
						}
					}
				}
                StateBrush = brush;
			}
		}

        //--- SetStatus --------------------------------------------------
        public void SetStatus(int no, TcpIp.SInkSupplyStat msg)
        {
            Info            = msg.info;
        //  Warn            = msg.warn;
            Err             = msg.err;

            CylinderPresSet  = msg.cylinderPresSet;
            CylinderPres     = msg.cylinderPres;
            CylinderPresDiff = msg.cylinderPresDiff;
            CylinderSetpoint = msg.cylinderSetpoint;
            PresLung        = msg.presLung;
            CondPresOut     = msg.condPresOut;
            CondPresIn      = msg.condPresIn;
            CondPumpSpeed   = msg.condPumpSpeed;
            CondTemp        = msg.condTemp;
            FlushTime       = msg.flushTime;
            Temp            = msg.temp;
            PumpSpeedSet    = msg.pumpSpeedSet;
            PumpSpeed       = msg.pumpSpeed;
            CtrlMode        = msg.ctrlMode;
            CanisterLevel   = msg.canisterLevel;
            CanisterErr     = (ELogType)msg.canisterErr;
            ScannerSN       = msg.scannerSN;
            BarCode         = msg.barcode.Replace("; ", "\n");

            Connected       = (msg.info & 0x00000001)!=0;
            BleedValve      = ((msg.info & 0x00000002)==0)? "--":"ON";
            AirCusionValve  = ((msg.info & 0x00000004)==0)? "--":"ON";
            Flushed         = (msg.info & 0x00000008)!=0;
            CondTempReady   = ((msg.info & 0x00000010)!=0) || (CtrlMode!=EFluidCtrlMode.ctrl_print);
            TempReady       = ((msg.info & 0x00000020)!=0) || (CtrlMode!=EFluidCtrlMode.ctrl_print);
            FlowFactorOk    = ((msg.info & 0x00000040)!=0) || (CtrlMode!=EFluidCtrlMode.ctrl_print);
            Warn            = !TempReady || !CondTempReady || !FlowFactorOk;

            
            if (Err!=0)    UpdateStateBrush(Rx.BrushError);
            else if (Warn) UpdateStateBrush(Rx.BrushWarn);
            else           UpdateStateBrush(Brushes.Transparent); 
            
        }
    }	
}
