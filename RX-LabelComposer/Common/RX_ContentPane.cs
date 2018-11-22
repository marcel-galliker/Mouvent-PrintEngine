using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using Infragistics.Windows.DockManager;

namespace RX_JobComposer.Common
{
    class RX_ContentPane : ContentPane
    {
        private static int h = 0;

        protected override System.Windows.Size MeasureOverride(System.Windows.Size constraint)
        {
            Size desiredSize = new System.Windows.Size();
            desiredSize.Width = constraint.Width;
            h += 20;
            desiredSize.Height = h;

            return desiredSize;
//            return base.MeasureOverride(constraint);
        }

        protected override Size ArrangeOverride(Size finalSize)
        {
            if (Equals(Header, "Page"))
            {
                finalSize.Height = 20;
            }
            return finalSize;
        }


    }
}
