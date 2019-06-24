using System;
using Xamarin.Forms;

namespace DigiPrint.Common
{
    public class Rx
    {
        //--- Str2Int -----------------------------
        public static int Str2Int(string str)
        {
            try {return Convert.ToInt32(str);}
            catch(Exception){};
            return 0;
        }

        //--- Invoke ---------------------------------------
        public static void Invoke(Action action)
        {
            Device.BeginInvokeOnMainThread(action);
        }

        public static Color CheckedBackground = Color.FromHex("#FFB5E616");
    }
}
