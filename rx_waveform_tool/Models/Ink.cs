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

        //--- Property Viscosity ---------------------------------------
        private int _Viscosity;
        public int Viscosity
        {
            get { return _Viscosity; }
            set { SetProperty(ref _Viscosity, value); }
        }

        //--- Property Density ---------------------------------------
        private int _Density;
        public int Density
        {
            get { return _Density; }
            set { SetProperty(ref _Density, value); }
        }

        //--- Property DropletVolume ---------------------------------------
        private int _DropletVolume;
        public int DropletVolume
        {
            get { return _DropletVolume; }
            set { SetProperty(ref _DropletVolume, value); }
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
                RxXml.LoadProperty(xml, this, "GreyLevel");
                RxXml.LoadProperty(xml, this, "MaxFreq");
                RxXml.LoadProperty(xml, this, "Meniscus");
                RxXml.LoadProperty(xml, this, "Viscosity");
                RxXml.LoadProperty(xml, this, "Density");
                RxXml.LoadProperty(xml, this, "DropletVolume");
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
                RxXml.SaveProperty(xml, this, "GreyLevel");
                RxXml.SaveProperty(xml, this, "MaxFreq");
                RxXml.SaveProperty(xml, this, "Meniscus");
                RxXml.SaveProperty(xml, this, "Viscosity");
                RxXml.SaveProperty(xml, this, "Density");
                RxXml.SaveProperty(xml, this, "DropletVolume");
            }
            xml.WriteEndElement();
            _Changed = false;
            return 0;
        }
               
    }
}
