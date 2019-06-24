using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;

namespace RX_DigiPrint.Helpers
{
    public class RxPopup : Popup
    {
        private Brush _background;

        //--- constructor ----------------------------
        public RxPopup()
        {
            EventManager.RegisterClassHandler(typeof(Window), Mouse.MouseDownEvent, new MouseButtonEventHandler(OnGlobalMousepDown), true);

            Placement=PlacementMode.Custom;
            CustomPopupPlacementCallback += new CustomPopupPlacementCallback(_popupPos);

            this.AllowsTransparency = true;
            Closed += RxPopup_Closed;
        }

        //--- _popupPos ------------------------------------------
        public CustomPopupPlacement[] _popupPos(Size popupSize, Size targetSize, Point offset)
        {
            Control ctrl = PlacementTarget as Control;
            if (ctrl!=null)
            {
                CustomPopupPlacement placement1 = new CustomPopupPlacement(new Point(0, ctrl.ActualHeight), PopupPrimaryAxis.Vertical);
                CustomPopupPlacement placement2 = new CustomPopupPlacement(new Point(10, 20), PopupPrimaryAxis.Horizontal);
                CustomPopupPlacement[] ttplaces = new CustomPopupPlacement[] { placement1, placement2 };
                return ttplaces;
            }
            return null;
        }

        //--- RxPopup_Closed -------------------------------
        void RxPopup_Closed(object sender, EventArgs e)
        {
            if (_Buddy!=null) _Buddy.Background = _background;
        }

        //--- Open ---------------
        public void Open(Control buddy)
        {
            _Buddy = buddy;
            if (_Buddy!=null) 
            {
                _background = _Buddy.Background;
                _Buddy.Background = Application.Current.Resources["XamGrid_Selected"] as Brush;
            }
            IsOpen = true;
        }

        //--- Property Buddy ---------------------------------------
        private Control _Buddy;
        public Control Buddy
        {
            get { return _Buddy; }
            set { _Buddy = value; }
        }
        
        //--- OnGlobalMousepDown ---------------------------
        private void OnGlobalMousepDown(object sender, MouseButtonEventArgs e)
        {
            Point pos = e.GetPosition(Child);
            Border border = Child as Border;

            if (border!=null)
            {
                if (this.IsOpen && (pos.X<0 || pos.X>border.ActualWidth || pos.Y<0 || pos.Y>border.ActualHeight))
                {
                    
                    this.IsOpen = false;
                    e.Handled = true;
                }
            }

            Grid grid = this.Child as Grid;
            if (grid!=null)
            {
                if (this.IsOpen && (pos.X<0 || pos.X>grid.ActualWidth || pos.Y<0 || pos.Y>grid.ActualHeight))
                {
                    this.IsOpen = false;
                    e.Handled = true;
                }
            }
        }
    }
}
