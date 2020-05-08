using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
    public class ColorCode_Str : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            List<RxEnum<int>> list = new EN_ColorCodeList().List;
            int val = System.Convert.ToInt32(value);
            string rectoVerso="";
            if (parameter!=null)
            {
                int inkSupply = System.Convert.ToInt32(parameter);
                if (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_DP803)
                    rectoVerso=(RxGlobals.InkSupply.List[inkSupply].RectoVerso==ERectoVerso.rv_verso)? "V": "R";
            }
            foreach(RxEnum<int> item in list)
            {
                if (item.Value == val)
                {
                    string str = item.Display;
                    if (parameter==null) return rectoVerso+str.Substring(str.IndexOf("("));
                    str = str.Substring(str.IndexOf("(")+1);
                    return rectoVerso+str.Replace(")", "");
                }
            }
            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
