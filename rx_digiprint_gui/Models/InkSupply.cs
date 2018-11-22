using RX_Common;
using RX_DigiPrint.Converters;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Collections.ObjectModel;

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
        private UInt32 _Warn;
        public UInt32 Warn
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

        //--- Property Humitity ---------------------------------------
        private Int32 _Humitity;
        public Int32 Humitity
        {
            get { return _Humitity; }
            set { SetProperty(ref _Humitity, value); }
        }

        //--- Property PresIntTankSet ---------------------------------------
        private Int32 _PresIntTankSet;
        public Int32 PresIntTankSet
        {
            get { return _PresIntTankSet; }
            set { SetProperty(ref _PresIntTankSet, value); }
        }

        //--- Property PresIntTank ---------------------------------------
        private Int32 _PresIntTank;
        public Int32 PresIntTank
        {
            get { return _PresIntTank; }
            set { SetProperty(ref _PresIntTank, value); }
        }

        //--- Property CondPresOut ---------------------------------------
        private Int32 _CondPresOut;
        public Int32 CondPresOut
        {
            get { return _CondPresOut; }
            set { SetProperty(ref _CondPresOut, value); }
        }

        //--- Property PresLung ---------------------------------------
        private Int32 _PresLung;
        public Int32 PresLung
        {
            get { return _PresLung; }
            set { SetProperty(ref _PresLung, value); }
        }

        //--- Property Temp ---------------------------------------
        private UInt32 _Temp;
        public UInt32 Temp
        {
            get { return _Temp; }
            set { SetProperty(ref _Temp, value); }
        }

        //--- Property PumpSpeedSet ---------------------------------------
        private UInt32 _PumpSpeedSet;
        public UInt32 PumpSpeedSet
        {
            get { return _PumpSpeedSet; }
            set { SetProperty(ref _PumpSpeedSet, value); }
        }

        //--- Property PumpSpeed ---------------------------------------
        private UInt32 _PumpSpeed;
        public UInt32 PumpSpeed
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
            get { return _Flushed; }
            set 
            { 
                if (value!=Flushed)
                    Console.WriteLine("Flushed changed");
                SetProperty(ref _Flushed, value); 
            }
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
            set { SetProperty(ref _InkType, value);}
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

        //--- SetStatus --------------------------------------------------
        public void SetStatus(int no, TcpIp.SInkSupplyStat msg)
        {
            Info            = msg.info;
            Warn            = msg.warn;
            Err             = msg.err;
//            if (no<15) CanisterLevel = canisterLevel[no+1];
            Humitity        = msg.humidity;
            PresIntTankSet  = msg.presIntTankSet;
            PresIntTank     = msg.presIntTank;
            PresLung        = msg.presLung;
            CondPresOut     = msg.condPresOut;
            Temp            = msg.temp;
            PumpSpeedSet    = msg.pumpSpeedSet;
            PumpSpeed       = msg.pumpSpeed;
            CtrlMode        = msg.ctrlMode;

            Connected       = (msg.info & 0x00000001)!=0;
            BleedValve      = ((msg.info & 0x00000002)==0)? "--":"ON";
            AirCusionValve  = ((msg.info & 0x00000004)==0)? "--":"ON";
            Flushed         = (msg.info & 0x00000008)!=0;
        }

        //--- CreateLog -----------------------------------
        public void CreateLog(RxWorkBook wb, int row, int col)
        {
            ColorCode_Str converter = new ColorCode_Str();
            if (_InkType!=null)
            {      
                wb.SetFieldColor(row, col, InkType.ColorFG, InkType.Color);
                wb.setText(row, 0, "Ink");                       wb.setText(row++, col, _InkType.Description);
                wb.setText(row, 0, "ColorCode");                 wb.setText(row++, col, converter.Convert(_InkType.ColorCode, null, null, null) as string);
                wb.setText(row, 0, "...");                       wb.setText(row++, col, "...");
            }
        }
    }	
}
