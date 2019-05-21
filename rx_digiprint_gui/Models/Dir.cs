using RX_Common.Source;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
    public class Dir : ObservableCollection<DirItem>
    {
        private ObservableCollection<DirItem> _List = new ObservableCollection<DirItem>();
        private string _FilterDir  = "$esnp";

        private static string _ripped_data = TcpIp.RX_RIPPED_DATA_ROOT.Remove(0,1)+":";
        private static string _source_data = TcpIp.RX_SOURCE_DATA_ROOT.Remove(0,1)+":";

        //--- local_ripped_data --------------------------------------------------------
        private static string local_ripped_data()
        {
            if (RxGlobals.PrinterProperties.IP_Address.Equals("localhost")) 
                return "d:"+TcpIp.RX_RIPPED_DATA_ROOT;
            return "\\\\" + RxGlobals.PrinterProperties.IP_Address + TcpIp.RX_RIPPED_DATA_ROOT;
        }

        //--- local_source_data --------------------------------------------------------
        private static string local_source_data()
        {
            return "d:"+TcpIp.RX_SOURCE_DATA_ROOT; 
        }

        //--- local_path ------------------------------------
        public static string local_path(string str)
        {
            if (str.StartsWith(_ripped_data))   return str.Replace(_ripped_data, local_ripped_data());
            if (str.StartsWith(_source_data))   return str.Replace(_source_data, local_source_data());
            if (str.Equals("..")) return str;
   //         return _ripped_data;
            return str;
        }

        //--- global_path ------------------------------------
        public static string global_path(string str)
        {   
            string local;
            local = local_ripped_data();
            if (str.StartsWith(local)) return str.Replace(local, _ripped_data);
            local = local_source_data();
            if (str.StartsWith(local)) return str.Replace(local, _source_data);
            return _ripped_data;
        }

        //--- filename -------------------------------
        public static string filename(string str)
        {
            string local = local_ripped_data();
            if (str.StartsWith(local)) return str.Remove(0, local.Length+1);
            return str;
        }

        //--- isRipped ----------------------------------
        public static bool isRipped(string path)
        {
            string global = global_path(path);
            return (global.StartsWith(_ripped_data));
        }

        //--- Property Busy ---------------------------------------
        private bool _Busy;
        public bool Busy
        {
            get { return _Busy; }
            set { _Busy = value; }
        }
        
        //--- creator ----------------------------------------------------
        public Dir(string dir_macro, bool IsRipped, bool IsRoot)
        {
            string dir = local_path(dir_macro);
            try
            {
                if (!IsRoot) _List.Add(new DirItem(){FileName=".."});

                if (!RxNetUse.ShareConnected(dir))
                {
                    RxNetUse.DeleteShare(dir);
                    RxNetUse.OpenShare(RxGlobals.PrinterProperties.RippedDataPath, null, null);
                }

                //--- standard ripped files -------------------------------
                if (RxNetUse.ShareConnected(dir))
                {
                    var dirs  = Directory.GetDirectories(dir);
                    Console.WriteLine("Directory {0}", dir);
                    foreach (string dirname in dirs)
                    {
                        try
                        {
                            if (!dirname.EndsWith(_FilterDir))
                            {
                                Console.WriteLine("File {0}", dirname);
                                if (IsRipped)   _List.Add(new DirItem(){DataFileName=global_path(dirname)});
                                else            _List.Add(new DirItem(){FileName=global_path(dirname)});
                            }
                        }
                        catch(Exception)
                        { }
                    }

                    var files = Directory.GetFiles(dir);
                    foreach (string file in files)
                    {
                        if (file.ToLower().EndsWith(".bmp")) _List.Add(new DirItem(){FileName=global_path(file)});
                        else if (!IsRipped)
                        {
                            if (file.ToLower().EndsWith(".gif") || file.ToLower().EndsWith(".tif") || file.ToLower().EndsWith(".jpeg") || file.ToLower().EndsWith(".png")
                                 || file.ToLower().EndsWith(".pdf") || file.ToLower().EndsWith(".ps") || file.ToLower().EndsWith(".jpg"))
                                _List.Add(new DirItem() { FileName = global_path(file) });
                        }
                    }
                }
                else _List=null;
            }
            catch(Exception)
            {
            }
        }


        //--- Property List ----------------------------------------------
        public ObservableCollection<DirItem> List
        {
            get { return _List; }
        }

    }
}
