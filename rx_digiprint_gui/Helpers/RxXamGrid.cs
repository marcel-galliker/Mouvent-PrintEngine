using Infragistics.Controls.Grids;
using RX_Common;
using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace RX_DigiPrint.Helpers
{
    public class RxXamGrid
    {

        private static RxScreen _Screen = new RxScreen();

        public static void RxStylusSystemGesture(object sender, System.Windows.Input.StylusSystemGestureEventArgs e)
        {
            if (e.SystemGesture == SystemGesture.Tap)
            {
                XamGrid grid = sender as XamGrid;
                var pos = e.GetPosition(grid);
                Point pos0 = grid.PointToScreen(new Point(0,0));
                pos.Y = pos0.Y+pos.Y/_Screen.Scale;
                // Debug.WriteLine(string.Format("pos.Y={0} pos0.Y={1}", pos.Y, pos0.Y));
                try 
                {
                    for (int row=0; row<grid.Rows.Count; row++)
                    {
                        Control ctrl = grid.Rows[row].Cells[0].Control;
                        if (ctrl!=null)
                        {
                            Point ctrl_pos=ctrl.PointToScreen(new Point(0,0));
        
                            //  Debug.WriteLine(string.Format("ROW[{0}]: Top={1}, Bottom={2}", row, ctrl_pos.Y, ctrl_pos.Y+2*ctrl.ActualHeight));
                            if (pos.Y >= ctrl_pos.Y && pos.Y <= ctrl_pos.Y+ctrl.ActualHeight/_Screen.Scale)
                            {
                                // Debug.WriteLine("FOUND");
                                grid.Rows[row].IsSelected = true;
                                break;
                            }
                        }
                    }   
                }
                catch(Exception) 
                { }
            }
        }


    }
}
