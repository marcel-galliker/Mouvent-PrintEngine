using Infragistics.Controls.Grids;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace RxWfTool.Models
{
    public class RxCellControl : CellControl
    {

        //--- Attached Property MouseOverBrush------------------------------------------------------
        static SolidColorBrush _brush = Brushes.Green;
        public static readonly DependencyProperty MouseOverBrushProperty;

        static RxCellControl()
        {
            _brush = (SolidColorBrush)App.Current.FindResource("XamGrid_MouseOver");
            MouseOverBrushProperty = DependencyProperty.RegisterAttached("MouseOverBrush", typeof(Brush), typeof(RxCellControl), new PropertyMetadata(_brush));
        }

        public static Brush GetMouseOverBrush(DependencyObject obj)
        {
            return (Brush)obj.GetValue(MouseOverBrushProperty);
        }

        public static void SetMouseOverBrush(DependencyObject obj, Brush value)
        {
            obj.SetValue(MouseOverBrushProperty, value);
        }
    }
}
