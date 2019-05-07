using System;
using System.Collections.Generic;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Media;

namespace RX_Common
{
    public class RxKeys
    {
        //--- ValueInt -------------------------------------------------------------
        public static int ValueInt(System.Windows.Input.KeyEventArgs e)
        {
            if (e.Key.ToString() == "Subtract")
            {
                e.Handled = true;
                return -1;
            }
            else if (e.Key.ToString() == "Add")
            {
                e.Handled = true;
                return 1;
            }
            else Console.WriteLine("Key >>{0}<<", e.Key.ToString());
            return 0;
        }

        //--- Value ---------------------------------------------------------------
        public static double Value(System.Windows.Input.KeyEventArgs e)
        {
            if (e.Key.ToString() == "Subtract")
            {
                e.Handled = true;
                if ((e.KeyboardDevice.Modifiers & System.Windows.Input.ModifierKeys.Shift)!=0) return -0.1;
                if ((e.KeyboardDevice.Modifiers & System.Windows.Input.ModifierKeys.Control)!=0) return -0.01;
                return -1.0;
            }
            else if (e.Key.ToString() == "Add")
            {
                e.Handled = true;
                if ((e.KeyboardDevice.Modifiers & System.Windows.Input.ModifierKeys.Shift)!=0) return 0.1;
                if ((e.KeyboardDevice.Modifiers & System.Windows.Input.ModifierKeys.Control)!=0) return 0.01;
                return 1.0;
            }
            return 0;
        }
    }
}