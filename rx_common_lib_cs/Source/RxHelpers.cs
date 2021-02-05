using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Net;
using System.Net.NetworkInformation;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;

namespace RX_Common
{
    public class Rx
    {
        public static Brush BrushError  = (Brush)Application.Current.Resources["RX.Error"];
        public static Brush BrushWarn   = (Brush)Application.Current.Resources["RX.Warn"];

        public static List<string> MonthName = new List<string>(){"", "JAN", "FEB", "MAR", "APR", "MAI", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
		
        //--- SplitArrayName ----------------------------------------------------------
        public static void SplitArrayName(string fullname, out string array, out int idx)
        {
            idx = -1;
            array = null;
            string[] str = fullname.Split(new char[]{'[', ']'});
            if (str.Length > 0)
            {
                array = str[0];
                if (str.Length > 1)
                {
                    try { idx = Convert.ToInt32(str[1]); }
                    catch { };
                }
            }
        }

        //--- ToArgb -------------------------------------
        public static UInt32 ToArgb(Color color)
        {
           byte[] bytes = new byte[] { color.B, color.G, color.R, color.A };
           return BitConverter.ToUInt32(bytes, 0); 
        }

        //--- GetCharFromKey ---------------------------------------
        public enum MapType : uint
        {
            MAPVK_VK_TO_VSC = 0x0,
            MAPVK_VSC_TO_VK = 0x1,
            MAPVK_VK_TO_CHAR = 0x2,
            MAPVK_VSC_TO_VK_EX = 0x3,
        }

        //--- StrToDouble ---------------------------------------
        public static double StrToDouble(string str)
        {
            double val=0;
            double weight=1;
            double sign=1.0;

            if (str==null) return 0;
            foreach(char c in str)
            {
                if (c=='-') sign=-1.0;
                if (c=='.' || c==',') weight=0.1;
                if (c>='0' && c<='9')
                {
                    if (weight==1) val=10*val+c-'0';
                    else 
                    {
                        val += weight*(c-'0');
                        weight/=10;
                    }
                }
               // else if (c>'9') 
               //     return double.NaN;
            }
                
            return val*sign; 
        }        

        //--- StrToUInt32 ------------------------------
        public static UInt32 StrToUInt32(string str)
        {
            UInt32 val=0;

            if (str==null) return 0;
            foreach(char c in str)
            {
                if (c=='.' || c==',') return val;
                if (c>='0' && c<='9') val=10*val+c-'0';
            }                
            return val; 
        }

        //--- StrToInt32 ------------------------------
        public static Int32 StrToInt32(string str)
        {
            Int32 val=0;
            Int32 sign=1;

            if (str==null) return 0;
            foreach(char c in str)
            {
                if (c=='-') sign=-1;
                if (c=='.' || c==',') return val*sign;
                if (c>='0' && c<='9') val=10*val+c-'0';
            }                
            return val*sign; 
        }

        //--- DoubleRange ------------------------------
        public static double DoubleRange(double value, double min, double max)
        {
            double val = Math.Round(value, 3);
            if (val<=min) return min;
            if (val>=max) return max;
            return val;
        }

        //--- StrNumFormat ---------------------------------------
        public static string StrNumFormat(string val, int decimals, double factor)
        {
         //   return val;

            if (val==null || (val[0]!='-' && (val[0]<'0' || val[0]>'9'))) return val;

            if (Rx.StrToInt32(val)==1431677568) return "----";

            if (factor!=1.0)
            {
                double value=StrToDouble(val);
                value *= factor;
                return Math.Round(value, decimals).ToString();
            }

            StringBuilder str = new StringBuilder(32);
            int i;
            int pos = val.IndexOf('.');
            if (pos<0)   
                pos = val.Length;
            if (val[0]=='-')
			{
                for (i=0; i<pos; i++)
                {
                    str.Append(val[i]);
                    if ((i+2<pos) && ((pos-i-1)%3)==1) str.Append('\'');
                }
			}
            else
			{
                for (i=0; i<pos; i++)
                {
                    str.Append(val[i]);
                    if ((i+1<pos) && ((pos-i)%3)==1) str.Append('\'');
                }
			}
            if (decimals>0)
            {
                str.Append('.'); pos++;
                while (pos<val.Length && decimals>0)
                {
                    str.Append(val[pos++]);
                    decimals--;
                }
                while (decimals>0)
                {
                    str.Append('0');
                    decimals--;
                }
            }
            return str.ToString();
        }

        [DllImport("user32.dll")]
        public static extern int ToUnicode(
            uint wVirtKey,
            uint wScanCode,
            byte[] lpKeyState,
            [Out, MarshalAs(UnmanagedType.LPWStr, SizeParamIndex = 4)] 
            StringBuilder pwszBuff,
            int cchBuff,
            uint wFlags);

        [DllImport("user32.dll")]
        public static extern bool GetKeyboardState(byte[] lpKeyState);

        [DllImport("user32.dll")]
        public static extern uint MapVirtualKey(uint uCode, MapType uMapType);

        public static char GetCharFromKey(Key key)
        {
            char ch = (char)0;

            int virtualKey = KeyInterop.VirtualKeyFromKey(key);
            byte[] keyboardState = new byte[256];
            GetKeyboardState(keyboardState);

            uint scanCode = MapVirtualKey((uint)virtualKey, MapType.MAPVK_VK_TO_VSC);
            StringBuilder stringBuilder = new StringBuilder(2);

            int result = ToUnicode((uint)virtualKey, scanCode, keyboardState, stringBuilder, stringBuilder.Capacity, 0);
            switch (result)
            {
            case -1:    break;
            case 0:     break;
            case 1:     ch = stringBuilder[0]; break;
            default:    ch = stringBuilder[0]; break;
            }
            return ch;
        }

        public static string CalledFrom()
        {
            /*
            StackTrace trace = new StackTrace();
            int caller = 1;
            StackFrame frame = trace.GetFrame(caller);
            string callerName = frame.GetMethod().Name;
			StackTrace trace = new StackTrace();
            */
			return new StackTrace().GetFrame(1).GetMethod().Name;
		}


        //--- StartProcess -----------------------------------------------
        public static Process StartProcess(string filepath, string arguments)
        {
            try
            {
                Process process = new Process();
                process.StartInfo.RedirectStandardOutput = true;
                process.StartInfo.UseShellExecute = false;
                process.StartInfo.CreateNoWindow = true;
                process.StartInfo.FileName = filepath;
                process.StartInfo.Arguments = arguments;
                process.Start();        
                return process;
            }
            catch(Exception e)
            {
                Console.WriteLine("Event: {0}", e.Message);
                return null;
            }
        }

        //--- ExecuteProcess -----------------------------------------------
        public static int ExecuteProcess(string filepath, string arguments)
        {
            try
            {
                Process process = new Process();
                process.StartInfo.RedirectStandardOutput = true;
                process.StartInfo.UseShellExecute = false;
                process.StartInfo.CreateNoWindow = true;
                process.StartInfo.FileName = filepath;
                process.StartInfo.Arguments = arguments;
                process.Start();
                process.WaitForExit();
                return process.ExitCode;
            }
            catch (Exception)
            {
                return 1;
            }
        }

        //--- ExecuteProcessWithOutput -----------------------------------------------
        public static string ExecuteProcessWithOutput(string filepath, string arguments)
        {
            try
            {
                Process process = new Process();
                process.StartInfo.RedirectStandardOutput = true;
                process.StartInfo.UseShellExecute = false;
                process.StartInfo.CreateNoWindow = true;
                process.StartInfo.FileName = filepath;
                process.StartInfo.Arguments = arguments;
                process.Start();
                string output = process.StandardOutput.ReadToEnd();
                process.WaitForExit();
                if (process.ExitCode==0) return output;
            }
            catch (Exception)
            {
                return null;
            }
            return null;
        }

        //--- ExecuteProcessAsAdmin -----------------------------------------------
        public static int ExecuteProcessAsAdmin(string filepath, string arguments)
        {
            try
            {

                Process process = new Process();
             //   process.StartInfo.RedirectStandardOutput = true;
             //   process.StartInfo.UseShellExecute = false;
             //   process.StartInfo.CreateNoWindow = true;
                process.StartInfo.FileName = filepath;
                process.StartInfo.Arguments = arguments;
                process.StartInfo.Verb = "runas";
                process.Start();
                process.WaitForExit();
                return process.ExitCode;
            }
            catch (Exception)
            {
                return 1;
            }
        }

        //--- GetComputerName -------------------------
        enum COMPUTER_NAME_FORMAT
        {
            ComputerNameNetBIOS,
            ComputerNameDnsHostname,
            ComputerNameDnsDomain,
            ComputerNameDnsFullyQualified,
            ComputerNamePhysicalNetBIOS,
            ComputerNamePhysicalDnsHostname,
            ComputerNamePhysicalDnsDomain,
            ComputerNamePhysicalDnsFullyQualified
        }

        [DllImport("kernel32.dll", SetLastError=true, CharSet=CharSet.Auto)] 
        static extern bool GetComputerNameEx(COMPUTER_NAME_FORMAT NameType, StringBuilder lpBuffer, ref uint lpnSize);
        public static string GetComputerName()
        {
            StringBuilder name = new StringBuilder(260);
            uint size = (uint) name.MaxCapacity;
            if (GetComputerNameEx(COMPUTER_NAME_FORMAT.ComputerNamePhysicalDnsHostname, name, ref size)) return name.ToString();
            else return null;
        }

        //--- wnd_remove_close_button -------------------------
        private const int GWL_STYLE = -16;
        private const int WS_SYSMENU = 0x80000;
        [DllImport("user32.dll", SetLastError = true)]
        private static extern int GetWindowLong(IntPtr hWnd, int nIndex);
        [DllImport("user32.dll")]
        private static extern int SetWindowLong(IntPtr hWnd, int nIndex, int dwNewLong);

        public static void wnd_remove_close_button(Window wnd)
        {
            var hwnd = new WindowInteropHelper(wnd).Handle;
            SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_SYSMENU);
        }
    }
}