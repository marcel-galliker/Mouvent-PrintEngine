using System;
using System.Collections.Generic;

namespace RX_DigiPrint.Comparers
{
    public class Date_Comparer:IComparer<string>
    {
        public int Compare( string x, string y )
        {
            try
            {
                DateTime x_time = DateTime.Parse(x);
                if (y==null) return 1;
                DateTime y_time = DateTime.Parse(y);
                return x_time.CompareTo(y_time);
            }
            catch(Exception)
            {
            }
            return string.Compare(x, y);
        }
    }
}
