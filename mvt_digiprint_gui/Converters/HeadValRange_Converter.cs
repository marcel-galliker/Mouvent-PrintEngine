using RX_Common;
using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Windows;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
	//--- HeadVal_Converter ----------------------------------------------------------------------
	public class HeadValRange_Converter : IValueConverter
    {

        private static Dictionary<string, int[]> _Range;

        private void _initRange()
        {
            //--- default values ------------------------
            _Range = new Dictionary<string, int[]>();
            _Range.Add("PresIn",        new int[2]{   10,  100 });
            _Range.Add("PresOut",       new int[2]{ -340, -260 });
            _Range.Add("Meniscus",      new int[2]{ -160, -140 });
            _Range.Add("PumpFeedback",  new int[2]{  240,  400 });
            _Range.Add("InkFeed",       new int[2]{  240,  400 });
            _Range.Add("CDA",           new int[2]{  300,  500 });

            //--- from file ------------------------------------------
            string fpath = Assembly.GetExecutingAssembly().Location;
            fpath = Path.Combine(Path.GetDirectoryName(fpath), "PrintSystemRange.csv");
            try
            {
                if (File.Exists(fpath))
                {
                    foreach (string line in System.IO.File.ReadLines(fpath))
                    {
                        string[] str = line.Split(';');
                        if (str.Length>=3 && _Range.ContainsKey(str[0]))
                        {
                            var item=_Range[str[0]];
                            item[0] = Rx.StrToInt32(str[1]);
                            item[1] = Rx.StrToInt32(str[2]);
                        }
                    }
                }
            }
            catch(Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
        }

        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (_Range==null) _initRange();
            if (parameter!=null)
            {
                int[] range =_Range[parameter as string];
                int val  = Rx.StrToInt32(value.ToString());
                if (val>=range[0] && val<=range[1]) return Visibility.Collapsed;
            }
            return Visibility.Visible;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            try
            {
                return System.Convert.ToInt32(value); 
            }
            catch (Exception)
            {
                return null;
            }
        }
    }
}
