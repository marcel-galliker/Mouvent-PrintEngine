using RX_Common;
using RX_Common.Source;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.IO;
using System.Linq;
using System.Threading;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for RxDirTree.xaml
    /// </summary>
    public partial class RxDirTree : UserControl
    {
        private static string _ripped_data = TcpIp.RX_RIPPED_DATA_ROOT.Remove(0,1)+":";
        private static string _source_data = TcpIp.RX_SOURCE_DATA_ROOT.Remove(0,1)+":";

        public delegate void ActDirChanged(string dir);

        public ActDirChanged ActDirChangedEvent = null;


        //--- constructor ---------------------------------
        public RxDirTree()
        {
            InitializeComponent();
            RxBindable.Invoke(() => TreeWiew.Items.Add(_getDir(_ripped_data)));
  //          RxBindable.Invoke(()=>TreeWiew.Items.Add(_getDir(_source_data)));
        }

        //--- Refresh ---------------------------------------
        public void Refresh()
        {
            string actDir = _ActDir;
            TreeWiew.Items.Clear();
            TreeWiew.Items.Add(_getDir(_ripped_data));
            if (TreeWiew.Items.Count>0)
            {
                TreeViewItem item = TreeWiew.Items[0] as TreeViewItem;
                item.IsExpanded = true;
            }
            
          //  TreeWiew.Items.Add(_getDir(_source_data));
            ActDir=actDir;
        }

        //--- Property ActDir ---------------------------------------
        private string _ActDir;
        public string ActDir
        {
            get { return _ActDir;}
            set 
            {
                _ActDir = value;
                _selectItem(TreeWiew.Items,  Dir.local_path(value));
            }
        }

        //--- _selectItem --------------------------------------------
        private void _selectItem(ItemCollection items, string path)
        {
            int i;
            for (i=0; i<items.Count; i++)
            {
                TreeViewItem item = items[i] as TreeViewItem;
                if (item!=null)
                {
                    if (item.Tag.Equals(path))
                    {
                        item.IsSelected = true;
                        item.BringIntoView();
                    }
                    if (item.Items!=null) _selectItem(item.Items, path);
                }
            }
        }
        
        //--- _getDir -----------------------------------------------
        private TreeViewItem _getDir(string path)
        {
            string local_path = Dir.local_path(path);
            if (RxNetUse.ShareConnected(local_path))
            {
                return _getSubDir(local_path);
            }
            else
            {
                RxNetUse.DeleteShare(local_path);
                if (RxNetUse.OpenShare(RxGlobals.PrinterProperties.RippedDataPath, null, null)==0)
                {
                    return _getSubDir(local_path);
                }
            }
            return null;
        }

        //--- _getSubDirs-------------------------------------
        private TreeViewItem _getSubDir(string path)
        {
            try
            {
                TreeViewItem item = new TreeViewItem(){Header=path};
                string[] split = path.Split(new char[] {'\\', '/'});
                if (split.Count()>0)
                {
                    item = new TreeViewItem(){Header=split[split.Count()-1]};
                    item.Tag = path;
                    item.Selected += item_Selected;
                    var dirs  = Directory.GetDirectories(path);
                    if (dirs.Count()==0)
                    { 
                    //    var files = Directory.GetFiles(path, "*.rxd");
                    //    if (files.Count()>0) return item;
                    //    else return null;
                        return null;
                    }
                    foreach (string subdir in dirs)
                    {
                        TreeViewItem subitem = _getSubDir(subdir);
                        if (subitem!=null) item.Items.Add(subitem);
                    }
                }
                return item;
            }
            catch(Exception)
            {
                return null;
            }
        }

        //--- item_Selected -------------------------------------
        private void item_Selected(object sender, RoutedEventArgs e)
        {
            TreeViewItem item = sender as TreeViewItem;
            if (item!=null && item.IsSelected && ActDirChangedEvent!=null)
            {
                ActDir = item.Tag as String;
                ActDirChangedEvent(_ActDir);
            }
        }

    }
}
