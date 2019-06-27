using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using RxWfTool.Helpers;
using System.Xml;
using System.Windows.Media;
using System.Runtime.CompilerServices;
using System.Reflection;
using RX_GUI.Models.Enums;
using RX_Common;

namespace RxWfTool.Models 
{
    public class Ink : NotificationObject
    {
        //--- Changed ---------------------------------------
        private bool _Changed = false;
        public bool Changed
        {
            get { return _Changed; }
            set { SetProperty(ref _Changed, value); }
        }

        //--- Property Name ---------------------------------------
        private string _Name;
        public string Name
        {
            get { return _Name; }
            set { _Changed |= SetProperty(ref _Name, value); }
        }
        
        //--- Property Description ---------------------------------------
        private string _Description;
        public string Description
        {
            get { return _Description; }
            set { _Changed |= SetProperty(ref _Description, value); }
        }
        
        //--- Property ColorRGB --------------------------------
        private Color _ColorRGB;
        public Color ColorRGB
        {
            get { return _ColorRGB; }
            set { _Changed |= SetProperty(ref _ColorRGB, value); }
        }

        //--- Property ColorCode ---------------------------------------
        private int _ColorCode;
        public int ColorCode
        {
            get { return _ColorCode; }
            set { _Changed |= SetProperty(ref _ColorCode, value); }
        }
        
        //--- OrientationList -------------------------------
        public EN_ColorCodeList ColorCodeList
        {
            get { return new EN_ColorCodeList(); }
        }

        //--- Property Temeprature ------------------------------
        private int _Temp;
        public int Temp
        {
            get { return _Temp; }
            set { _Changed |= SetProperty(ref _Temp, value); }
        }

        //--- Property Temeprature ------------------------------
        private int _TempMax;
        public int TempMax
        {
            get { return _TempMax; }
            set { _Changed |= SetProperty(ref _TempMax, value); }
        }

        //--- Property Temeprature ------------------------------
        private int _TempChiller;
        public int TempChiller
        {
            get { return _TempChiller; }
            set { _Changed |= SetProperty(ref _TempChiller, value); }
        }

        //--- Property Temeprature ------------------------------
        private int _Viscosity;
        public int Viscosity
        {
            get { return _Viscosity; }
            set { _Changed |= SetProperty(ref _Viscosity, value); }
        }

        //--- Property GreyLevel ---------------------------------------
        private int[] _GreyLevel = new int[4];
        public int[] GreyLevel
        {
            get { return _GreyLevel; }
            set { SetProperty(ref _GreyLevel, value); }
        }

        //--- Property MaxFreq ---------------------------------------
        private int _MaxFreq;   //Hz
        public int MaxFreq
        {
            get { return _MaxFreq; }
            set { SetProperty(ref _MaxFreq, value); }
        }

        //--- Property Meniscus ---------------------------------------
        private int _Meniscus;
        public int Meniscus
        {
            get { return _Meniscus; }
            set { SetProperty(ref _Meniscus, value); }
        }
        
        //--- Property FlushTime0 ---------------------------------------
        private int _FlushTime0;
        public int FlushTime0
        {
            get { return _FlushTime0; }
            set { SetProperty(ref _FlushTime0, value); }
        }

        //--- Property FlushTime1 ---------------------------------------
        private int _FlushTime1;
        public int FlushTime1
        {
            get { return _FlushTime1; }
            set { SetProperty(ref _FlushTime1, value); }
        }

        //--- Property FlushTime2 ---------------------------------------
        private int _FlushTime2;
        public int FlushTime2
        {
            get { return _FlushTime2; }
            set { SetProperty(ref _FlushTime2, value); }
        }

        //--- Property FlushTime ---------------------------------------
        public string FlushTime
        {
            get 
            { 
                return string.Format("{0} {1} {2}", FlushTime0, FlushTime1, FlushTime2); 
            }
            set 
            { 
                string[] list = (value as string).Split(' ');
                FlushTime0 = Rx.StrToInt32(list[0]); 
                FlushTime1 = Rx.StrToInt32(list[1]); 
                FlushTime2 = Rx.StrToInt32(list[2]); 
            }
        }

        //--- Property PrintSpeed0 ---------------------------------------
        private int _PrintSpeed0;
        public int PrintSpeed0
        {
            get { return _PrintSpeed0; }
            set { SetProperty(ref _PrintSpeed0, value); }
        }
        
        //--- Property PrintSpeed1 ---------------------------------------
        private int _PrintSpeed1;
        public int PrintSpeed1
        {
            get { return _PrintSpeed1; }
            set { SetProperty(ref _PrintSpeed1, value); }
        }

        //--- Property PrintSpeed2 ---------------------------------------
        private int _PrintSpeed2;
        public int PrintSpeed2
        {
            get { return _PrintSpeed2; }
            set { SetProperty(ref _PrintSpeed2, value); }
        }

        //--- Property PrintSpeed2 ---------------------------------------
        private int _PrintSpeed3;
        public int PrintSpeed3
        {
            get { return _PrintSpeed3; }
            set { SetProperty(ref _PrintSpeed3, value); }
        }

        //--- Property PrintSpeed4 ---------------------------------------
        private int _PrintSpeed4;
        public int PrintSpeed4
        {
            get { return _PrintSpeed4; }
            set { SetProperty(ref _PrintSpeed4, value); }
        }

        //--- Property PrintSpeed5 ---------------------------------------
        private int _PrintSpeed5;
        public int PrintSpeed5
        {
            get { return _PrintSpeed5; }
            set { SetProperty(ref _PrintSpeed5, value); }
        }

        //--- Property PrintingSpeed ---------------------------------------
        private string _PrintingSpeed;
        public string PrintingSpeed
        {
            get 
            { 
                return string.Format("{0} {1} {2} {3} {4} {5}", PrintSpeed0, PrintSpeed1, PrintSpeed2, PrintSpeed3, PrintSpeed4, PrintSpeed5); 
            }
            set 
            { 
                string[] list = (value as string).Split(' ');
                PrintSpeed0 = Rx.StrToInt32(list[0]); 
                PrintSpeed1 = Rx.StrToInt32(list[1]); 
                PrintSpeed2 = Rx.StrToInt32(list[2]); 
                PrintSpeed3 = Rx.StrToInt32(list[3]); 
                PrintSpeed4 = Rx.StrToInt32(list[4]); 
                PrintSpeed5 = Rx.StrToInt32(list[5]); 
            }
        }   

        //--- Property DropletVolume ---------------------------------------
        private int _DropletVolume;
        public int DropletVolume
        {
            get { return _DropletVolume; }
            set { SetProperty(ref _DropletVolume, value); }
        }
        
        //--- Property DropSpeed ---------------------------------------
        private int _DropSpeed;
        public int DropSpeed
        {
            get { return _DropSpeed; }
            set { SetProperty(ref _DropSpeed, value); }
        }

        //--- load --------------------------------------------
        public bool load(ref XmlTextReader xml)
        {
            if (xml.NodeType==XmlNodeType.Element && xml.Name.Equals("Ink"))
            {
                TempMax = 0;
                RxXml.LoadProperty(xml, this, "Name");
                RxXml.LoadProperty(xml, this, "ColorCode");
                RxXml.LoadProperty(xml, this, "Description");
                RxXml.LoadProperty(xml, this, "ColorRGB");
                RxXml.LoadProperty(xml, this, "Temp");
                RxXml.LoadProperty(xml, this, "TempMax");
                RxXml.LoadProperty(xml, this, "TempChiller");
                RxXml.LoadProperty(xml, this, "GreyLevel");
                RxXml.LoadProperty(xml, this, "MaxFreq");
                RxXml.LoadProperty(xml, this, "Meniscus");
                RxXml.LoadProperty(xml, this, "Viscosity");
                RxXml.LoadProperty(xml, this, "DropSpeed");
                RxXml.LoadProperty(xml, this, "DropletVolume");
                RxXml.LoadProperty(xml, this, "FlushTime");
                RxXml.LoadProperty(xml, this, "PrintingSpeed");
                _Changed = false;
                return true;
            }
            return false;
        }

        //--- save -------------------------------------------
//        public int save(XmlTextWriter xml)
        public int save(XmlWriter xml)
        {
            xml.WriteStartElement("Ink");
            {
                RxXml.SaveProperty(xml, this, "Name");
                RxXml.SaveProperty(xml, this, "ColorCode");
                RxXml.SaveProperty(xml, this, "Description");
                RxXml.SaveProperty(xml, this, "ColorRGB");
                RxXml.SaveProperty(xml, this, "Temp");
                RxXml.SaveProperty(xml, this, "TempMax");
                RxXml.SaveProperty(xml, this, "TempChiller");
                RxXml.SaveProperty(xml, this, "GreyLevel");
                RxXml.SaveProperty(xml, this, "MaxFreq");
                RxXml.SaveProperty(xml, this, "Meniscus");
                RxXml.SaveProperty(xml, this, "Viscosity");
                RxXml.SaveProperty(xml, this, "DropSpeed");
                RxXml.SaveProperty(xml, this, "DropletVolume");
                RxXml.SaveProperty(xml, this, "FlushTime");
                RxXml.SaveProperty(xml, this, "PrintingSpeed");
            }
            xml.WriteEndElement();
            _Changed = false;
            return 0;
        }
               
    }
}
