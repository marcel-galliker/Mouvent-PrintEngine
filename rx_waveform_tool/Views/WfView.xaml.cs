using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using RxWfTool.Models;

namespace RxWfTool.Views
{
    /// <summary>
    /// Interaction logic for WfView.xaml
    /// </summary>
    public partial class WfView : UserControl
    {
        //--- Constructor --------------------------------------
        public WfView()
        {
            InitializeComponent();
        }

        private WfDef _WfDef;
        public WfDef WfDef
        {
            get { return _WfDef; }
            set 
            { 
                _WfDef = value; 
                WfDefList.ItemsSource   = _WfDef.List;
                Graph.WfDef             = _WfDef;
                InkDef.DataContext      = _WfDef.Ink;
                FlushTime.DataContext   = _WfDef.Ink;
                GreyLevelDef.GreyLevel  = _WfDef.Ink.GreyLevel;
                Performance.DataContext = _WfDef;
            }
        }

        //--- Add_clicked ------------------------------------------------------------
        private void Add_clicked(object sender, RoutedEventArgs e)
        {
            if (WfDefList.ActiveCell==null)
            {
                _WfDef.insert(0);
            }
            else
            {
                _WfDef.insert( WfDefList.ActiveCell.Row.Index);
            }
            e.Handled = true;
        }
        
        //--- Delete_clicked ------------------------------------------------------------
        private void Delete_clicked(object sender, RoutedEventArgs e)
        {
            if (WfDefList.ActiveCell!=null)
            {
                _WfDef.delete( WfDefList.ActiveCell.Row.Index);
            }
            e.Handled = true;
        }

        //--- WfDefList_ActiveCellChanged -----------------------------------------------
        private void WfDefList_ActiveCellChanged(object sender, EventArgs e)
        {
            if (WfDefList.ActiveCell!=null)
            {
                Graph.ActiveItem = WfDefList.ActiveCell.Row.Index;
            }
        }
    }
}
