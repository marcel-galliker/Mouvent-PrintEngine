using rx_CamLib;
using RX_Common;
using RX_DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;
using static RX_DigiPrint.Models.CTC_Test;

namespace RX_DigiPrint.Converters
{
	//--- CTC_StateImage_Converter ----------------------------------------------------
	public class CTC_StateImage_Converter : IValueConverter
	{
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			switch ((EN_State)value)
			{
				case EN_State.undef:	return null;
				case EN_State.running:	return "ProgressCheck";
				case EN_State.ok:		return "Check";
				case EN_State.failed:	return "CloseThick";
				default:				return null;
			}
		}
		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return null;
        }
	}

	//--- CTC_StateColor_Converter ----------------------------------------
	public class CTC_StateColor_Converter : IValueConverter
	{
		public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			switch ((EN_State)value)
			{
				case EN_State.undef:	return Brushes.Transparent;
				case EN_State.running:	return Brushes.Black;
				case EN_State.ok:		return Brushes.Green;
				case EN_State.failed:	return Rx.BrushError;
				default:				return Brushes.Transparent;
			}
		}
		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			return null;
		}
	}
}
