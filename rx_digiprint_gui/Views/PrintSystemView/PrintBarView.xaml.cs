using Infragistics.Controls.Editors;
using Infragistics.Controls.Editors.Primitives;
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Media;

namespace RX_DigiPrint.Views.PrintSystemView
{
    public partial class PrintBarView : UserControl
    {
        public PrintBarView()
        {
            InitializeComponent();
        }
        
        //--- to adjust when styla changes ---------------------------
        private int _border_width  = 3;
        private int _header_height = 28;
        //------------------------------------------------------------

        //----------------------------------------------------------------
        private bool _intialized=false;
        private void ColorCombo_DropDownOpening(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (_intialized) return;

            XamMultiColumnComboEditor editor = sender as XamMultiColumnComboEditor;
            if (VisualTreeHelper.GetChildrenCount(editor)==0) return;

            Grid grid = VisualTreeHelper.GetChild(editor, 0) as Grid;
            Popup popup = ((grid.Children[0] as Border).Child as Grid).Children[0] as Popup;
            
            // Handle the popup opened event.  The grid rows will exist by the time this event is fired.
            popup.Opened += new EventHandler(popup_Opened);

            _intialized = true;            
        }

        //--- ---------------------------------------------------
        private Popup _popup;
        private void MySizeChanged(Object sender, SizeChangedEventArgs e)
        {
            Console.WriteLine("New width={0}", e.NewSize.Width);
            popup_Opened(_popup, null);
        }

        //--- popup_Opened ------------------------------------------
        private bool first=true;
        void popup_Opened(object sender, EventArgs e)
        {
            Popup popup = sender as Popup;
            _popup = popup;
            DependencyObject itemsPanel = FindFirstRow(popup.Child);
            if (itemsPanel != null && VisualTreeHelper.GetChildrenCount(itemsPanel) > 0)
            {
                double width = 0;
                int cnt = VisualTreeHelper.GetChildrenCount(itemsPanel);
                for (int i = 0; i < cnt; i++)
                {
                    ComboCellControl control = VisualTreeHelper.GetChild(itemsPanel, i) as ComboCellControl;
                    if (control!=null)
                    {
                        if (first) control.SizeChanged += MySizeChanged;
                        // Console.WriteLine("col {0}: width={1}", i, control.ActualWidth);
                        if (i<cnt-1 || control.ActualWidth>1) width += control.ActualWidth; // an 1 size cell is added automatically
                    }
                }

                width+=_border_width;
                Border border = (popup.Child as System.Windows.Controls.Grid).Children[0] as Border;
                border.Width = width; 
                popup.MinWidth = popup.MaxWidth  = width;
                popup.MinHeight = popup.MaxHeight = ActualHeight+_header_height;
            }
            first=false;
        }

        //--- FindFirstRow ----------------------------------------------------
        private DependencyObject FindFirstRow(DependencyObject root)
        {
            for (int i = 0; i < VisualTreeHelper.GetChildrenCount(root); i++)
            {
                DependencyObject dObj = VisualTreeHelper.GetChild(root, i);
                if (dObj.GetType() == typeof(ComboCellsPanel))
                    return dObj;

                dObj = FindFirstRow(dObj);
                if (dObj != null)
                    return dObj;
            }
            return null;
        }
    }
}
