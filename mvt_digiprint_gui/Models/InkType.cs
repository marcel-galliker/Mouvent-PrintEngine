using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Windows.Media;

namespace RX_DigiPrint.Models
{
    public class InkType : RxBindable
    {
        static public InkType Flush = new InkType(){Name="Flush", ColorFG=Colors.Black, Color=Color.FromRgb(0xf0,0xf0,0xf0), ColorCode=888};
        static public InkType Waste = new InkType(){Name="Waste", ColorFG=Colors.White, Color=Color.FromRgb(0x80,0x80,0x80), ColorCode=999};

        //--- constructor -------------------------------
        public InkType()
        {

        }

        //--- constructor -------------------------------
        public InkType(TcpIp.SInkDefinition msg)
        {
            Family      = msg.family;
            FileName    = msg.fileName;
            Name        = msg.name;
            ColorCode   = msg.colorCode; 
            Description = msg.description;
            MaxSpeed = msg.maxSpeed;
            CanFlush    = msg.flushTime[0]!=0 || msg.flushTime[1]!=0 || msg.flushTime[2]!=0;
            Color=new Color(){R=msg.r, G=msg.g, B=msg.b, A=255};
            Temp = (double)msg.temp * 1000.0;
            TempMax = (double)msg.tempMax * 1000.0;
            if (_PrintingSpeed==null) _PrintingSpeed = new Int32[msg.printingSpeed.Length];
            int i;
            for (i=0; i<msg.printingSpeed.Length; i++) _PrintingSpeed[i] = msg.printingSpeed[i];
        }

        public Int32[] MaxSpeed  { get; set; }

        //--- Property FileName ---------------------------------------
        private string _FileName;
        public string FileName
        {
            get { return _FileName; }
            set { SetProperty(ref _FileName, value); }
        }
        
        //--- Property Name ---------------------------------------
        private string _Name = "name";
        public string Name
        {
            get { return _Name; }
            set { SetProperty(ref _Name, value); }
        }

        //--- Property ColorCode ---------------------------------------
        private UInt32 _ColorCode=0;
        public UInt32 ColorCode
        {
            get { return _ColorCode; }
            set { SetProperty(ref _ColorCode, value); }
        }
        
        //--- Property Description ---------------------------------------
        private string _Description;
        public string Description
        {
            get { return _Description; }
            set { SetProperty(ref _Description, value); }
        }
        
        //--- Property Color ---------------------------------------
        private Color _Color;
        public Color Color
        {
            get { return _Color; }
            set 
            { 
                if (SetProperty(ref _Color, value))
                {
                    if (_Color.B>100 && _Color.G<100 || _Color.R + _Color.G + _Color.B < 200) ColorFG = Colors.White;
                    else ColorFG = Colors.Black;
                } 
            }
        }        

        //--- ColorFG ---------------------------------------------------
        Color _ColorFG;
        public Color ColorFG
        {
            get { return _ColorFG;}
            set { _ColorFG = value;}
        }

        //--- Property CanFlush ---------------------------------------
        private bool _CanFlush = false;
        public bool CanFlush
        {
            get { return _CanFlush; }
            set { SetProperty(ref _CanFlush, value); }
        }

        //--- Property PrintingSpeed ---------------------------------------
        private int[] _PrintingSpeed;
        public int[] PrintingSpeed
        {
            get { return _PrintingSpeed; }
            set { SetProperty(ref _PrintingSpeed, value); }
        }

        //--- Property Temp ---------------------------------------
        private double _Temp;
        public double Temp
        {
            get { return _Temp; }
            set { SetProperty(ref _Temp, value); }
        }

        //--- Property Temp ---------------------------------------
        private double _TempMax;
        public double TempMax
        {
            get { return _TempMax; }
            set { SetProperty(ref _TempMax, value); }
        }
        
        //--- Property Family ---------------------------------------
        private string _Family;
        public string Family
        {
            get { return _Family; }
            set { SetProperty(ref _Family, value); }
        }
        
        //--- ColorNameShort --------------------------------
        public static string ColorNameShort(UInt32 ColorCode )
        {
            switch(ColorCode)
            {
            case  0: return "K"   ;
	        case  1: return "C"   ;
	        case  2: return "M"   ;
	        case  3: return "Y"   ;
	        case  4: return "R"   ;
	        case  5: return "G"   ;
	        case  6: return "B"   ;
	        case  7: return "LK"  ;
	        case  8: return "LC"  ;
	        case  9: return "LM"  ;
	        case 10: return "LLK" ;
	        case 11: return "LLC" ;
	        case 12: return "LLM" ;
	        case 13: return "LY"  ;
	        case 14: return "LR"  ;
	        case 15: return "LG"  ;
	        case 16: return "LB"  ;
	        case 17: return "S6"  ;
	        case 18: return "S5"  ;
	        case 19: return "S4"  ;
	        case 20: return "S3"  ;
	        case 21: return "S2"  ;
	        case 22: return "S1"  ;
	        case 23: return "LLY" ;
	        case 24: return "MID" ;
	        case 25: return "PRI1";
	        case 26: return "PRI2";
	        case 27: return "V4"  ;
	        case 28: return "V3"  ;
	        case 29: return "V2"  ;
	        case 30: return "V1"  ;
	        case 31: return "W4"  ;
	        case 32: return "W3"  ;
	        case 33: return "W2"  ;
	        case 34: return "W1"  ;
	        case 35: return "P8"  ;
	        case 36: return "P9"  ;
	        case 37: return "P10" ;
	        case 38: return "P11" ;
	        case 39: return "P12" ;
	        case 40: return "3LK" ;
	        case 41: return "4LK" ;
	        case 42: return "5LK" ;
	        case 43: return "6LK" ;
	        case 44: return "LP8" ;
	        case 45: return "LP9" ;
	        case 46: return "LP10";
	        case 47: return "LP11";
	        case 48: return "LP12";
	        case 49: return "LS6" ;
	        case 50: return "LS5" ;
	        case 51: return "LS4" ;
	        case 52: return "LS3" ;
	        case 53: return "LS2" ;
	        case 54: return "LS1" ;
            default: return "x";
            }
        }
    }
}
