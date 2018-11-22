using RX_Common;
using RX_DigiPrint.Converters;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
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
        private int _HeadNo;
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
            set { SetProperty(ref _Color, value); }
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

        //--- Property Property -------------------------------------------
        private UInt32 _TempFpga;
        public UInt32 TempFpga
        {
            get { return _TempFpga; }
            set { SetProperty(ref _TempFpga, value); }
        }
        
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

        //--- Property PresIn ---------------------------------------
        private Int32 _PresIn_diff;
        public Int32 PresIn_diff
        {
            get { return _PresIn_diff; }
            set { SetProperty(ref _PresIn_diff, value); }
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
                else                                    return _PumpFeedback * 60 / 100; 
            }
            set { SetProperty(ref _PumpFeedback, value); }
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

        //--- SetItem ----------------------------------------------
        public void SetItem(int no, TcpIp.SHeadStat item, UInt32 tempFpga, Int32 flow)
        {   
            List<RxEnum<int>> list = new EN_ColorCodeList().List;
            HeadNo  = no;

            try
            {
                int ink = no/(int)RxGlobals.PrintSystem.HeadCnt;
                Color   = new SolidColorBrush(RxGlobals.InkSupply.List[ink].InkType.Color);
                ColorFG = new SolidColorBrush(RxGlobals.InkSupply.List[ink].InkType.ColorFG);

                string str = new ColorCode_Str().Convert(RxGlobals.InkSupply.List[ink].InkType.ColorCode, null, ink, null).ToString();

                Name    = str+"-"+(1+no%(int)RxGlobals.PrintSystem.HeadCnt).ToString();
            }
            catch(Exception)
            {
                Color   = Brushes.Transparent;
                ColorFG = Brushes.Black;
                Name    = "?-"+(1+no).ToString();
            }

            Connected   = (item.info&0x00000001)!=0;
             
            Info        = item.info;
            Warn        = item.warn;
            Err         = item.err;

            Valve       = ((item.info&0x02)==0)? 0:1;

            DotCnt      = item.dotCnt;
            ImgInCnt    = item.imgInCnt;
            PrintGoCnt  = item.printGoCnt;
            PrintDoneCnt= item.printDoneCnt;

            // Conditioner
            TempHead    = item.tempHead;
            TempCond    = item.tempCond;
            PresIn      = item.presIn;
            PresIn_diff = item.presIn_diff;
            PresOut     = item.presOut;
            PresOut_diff = item.presOut_diff;
            Meniscus    = item.meniscus;
            Meniscus_diff= item.meniscus_diff;
            PumpSpeed   = item.pumpSpeed; 
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

            TempFpga    = tempFpga;
            Flow        = flow;
        }

        //--- SendBt ----------------------------------------------------
        public void SendBt()
        {
            RxBtDef.SBtHeadStatMsg msg = new RxBtDef.SBtHeadStatMsg();
            msg.no           = HeadNo;
            msg.status.name  = Name;
            try
            {
                int ink = HeadNo/(int)RxGlobals.PrintSystem.HeadCnt;
                msg.status.color   = Rx.ToArgb(RxGlobals.InkSupply.List[ink].InkType.Color);
                msg.status.colorFG = Rx.ToArgb(RxGlobals.InkSupply.List[ink].InkType.ColorFG);
            }
            catch(Exception e)
            {
                Console.WriteLine("Exception {0}", e.Message);
                msg.status.color   = Rx.ToArgb(Colors.Black);
                msg.status.colorFG = Rx.ToArgb(Colors.White);
            }
            msg.status.ctrlMode = (RxBtDef.EFluidCtrlMode)CtrlMode;
            msg.status.temp     = (Int32)TempHead;
            msg.status.meniscus = Meniscus;
            RxGlobals.Bluetooth.SendMsg(RxBtDef.BT_REP_HEAD_STATE, ref msg);
        }
    }
}
