using RX_Common;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.UserControls;
using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;

namespace RX_DigiPrint.Models
{
    public class PrinterProperties : RxBindable, ICloneable
    {
        private string _Path = "D:\\radex\\user";

        //--- Constructor -------------------------------
        public PrinterProperties()
        {
            if (!Directory.Exists("d:\\"))
            {
                _Path = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData) + "\\Radex";
            }
            Settings.The_Settings.Load(_Path);
        }

        public object Clone()
        {
            return this.MemberwiseClone();
        }

        public PrinterProperties RxClone()
        {
            return (PrinterProperties)this.MemberwiseClone();
        }
        
        //--- NetworkSettings -----------------------------
        private bool _first=true;
        public void NetworkSettings(TcpIp.SIfConfig settings)
        {            
            Host_DHCP    = (settings.dhcp!=0);
            Host_Name    = settings.hostname;
            Host_Address = string.Format("{0}.{1}.{2}.{3}", settings.addr[0], settings.addr[1], settings.addr[2], settings.addr[3]);
            Host_Mask    = string.Format("{0}.{1}.{2}.{3}", settings.mask[0], settings.mask[1], settings.mask[2], settings.mask[3]);
            if (_first)
            {
                _first=false;
                string name=Rx.GetComputerName().ToUpper();
                string hostname = settings.hostname.ToUpper();
                if (name!=null && Process.GetProcessesByName("Win10-Startup").Length>0 && !name.Contains(hostname))
                {
                    SetComputerNameWindow wnd = new SetComputerNameWindow();
                    wnd.ShowDialog();
                }
            }
  //          Host_Enabled = (settings.hostname.Length>0) && (Host_DHCP || settings.mask[0]!=0);
        }

        //--- SetNetworkSettings -----------------------------
        public void SetNetworkSettings()
        {
            TcpIp.SIfConfig     settings = new TcpIp.SIfConfig();
            settings.addr       = new byte[4];
            settings.mask       = new byte[4];
            settings.dhcp     = Convert.ToUInt32(Host_DHCP);
            settings.hostname = Host_Name;
            int i;
            try
            {
                string[] addr=Host_Address.Split('.');
                for (i=0; i<addr.Length && i<4; i++) settings.addr[i] = Convert.ToByte(addr[i]);
            }
            catch(Exception){}

            try
            {
            string[] mask=Host_Mask.Split('.');
            for (i=0; i<mask.Length && i<4; i++) settings.mask[i] = Convert.ToByte(mask[i]);            
            }
            catch(Exception){}

            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_NETWORK_SETTINGS, ref settings);
        }

        //--- init -------------------------------------------
        public bool init
        {
            set 
            { 
                Name        = Settings.The_Settings.Name;
                IP_Address  = Settings.The_Settings.IP_Addr;
                use();
            }
        }

        //--- use ----------------------------------
        public void use()
        {
            if (RxGlobals.RxInterface!=null)
                RxGlobals.RxInterface.Address = _IP_Address;
            Changed=false;
        }

        //--- Save --------------------------------------------
        public void Save()
        {
            Settings.The_Settings.Name     = Name;
            Settings.The_Settings.IP_Addr  = IP_Address;
            Settings.The_Settings.Save(_Path);
            use();
        }

        //--- Property Changed ---------------------------------------
        private bool _Changed=false;
        public bool Changed
        {
            get { return _Changed; }
            set { SetProperty(ref _Changed, value); }
        }        

        //--- Property Name -----------------------------
        private string _Name;
        public string Name
        {
            get { return _Name; }
            set { if (SetProperty(ref _Name, value)) Changed=true; }
        }
        
        //--- Property IP_Address -----------------------
        private string _IP_Address;
        public string IP_Address
        {
            get { return _IP_Address; }
            set { if(SetProperty(ref _IP_Address, value)) Changed=true; }
        }

        //--- Property IP_AddressList ---------------------------------------
        private ObservableCollection<string> _IP_AddressList;
        public ObservableCollection<string> IP_AddressList
        {
            get 
            { 
                if (_IP_AddressList==null)
                {
                    IP_AddressList = new ObservableCollection<string>{};
                    IP_AddressList.Add("192.168.200.1");
                    IP_AddressList.Add("localhost");
                }
                return _IP_AddressList; 
            }
            set { SetProperty(ref _IP_AddressList, value); }
        }
        
        //--- RippedDataPath -----------------------------------
        public string RippedDataPath
        {
            get { return Settings.The_Settings.RippedDataPath;}
            set { string path=value;}
        }

        /*
        //--- Property Enabled ---------------------------------------
        private bool _Host_Enabled;
        public bool Host_Enabled
        {
            get { return _Host_Enabled; }
            set { SetProperty(ref _Host_Enabled, value); }
        }
        */

        //--- Property Host_DHCP ---------------------------------------
        private bool _Host_DHCP;
        public bool Host_DHCP
        {
            get { return _Host_DHCP; }
            set { SetProperty(ref _Host_DHCP, value); }
        }

        //--- Property Host_Name ---------------------------------------
        private string _Host_Name;
        public string Host_Name
        {
            get 
            { 
                if (_Host_Name==null || _Host_Name.Equals(""))
                {
                    switch (RxGlobals.PrintSystem.PrinterType)
                    {
                        case EPrinterType.printer_test_slide:       SetProperty(ref _Host_Name, "TestSlide"); break; 
                        case EPrinterType.printer_test_slide_only:  SetProperty(ref _Host_Name, "TestSlide"); break; 
                        case EPrinterType.printer_test_table:       SetProperty(ref _Host_Name, "TestTable"); break; 
                        case EPrinterType.printer_LB701:            SetProperty(ref _Host_Name, "LB701-xxxx"); break; 
                        case EPrinterType.printer_LB702:            SetProperty(ref _Host_Name, "LB702-xxxx"); break; 
                        case EPrinterType.printer_TX801:            SetProperty(ref _Host_Name, "TX801-xxxx"); break; 
                        case EPrinterType.printer_TX802:            SetProperty(ref _Host_Name, "TX802-xxxx"); break; 
                        case EPrinterType.printer_cleaf:            SetProperty(ref _Host_Name, "PRINTER-xxxx"); break; 
                    }
                }
                return _Host_Name; 
            }

            set { SetProperty(ref _Host_Name, value); }
        }

        //--- Property Host_Address ---------------------------------------
        private string _Host_Address;
        public string Host_Address
        {
            get { return _Host_Address; }
            set { SetProperty(ref _Host_Address, value); }
        }

        //--- Property Host_Mask ---------------------------------------
        private string _Host_Mask;
        public string Host_Mask
        {   
            get { return _Host_Mask; }
            set { SetProperty(ref _Host_Mask, value); }
        }

    }
}
