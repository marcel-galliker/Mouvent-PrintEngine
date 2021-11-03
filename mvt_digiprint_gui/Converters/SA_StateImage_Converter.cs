using rx_CamLib;
using RX_DigiPrint.Models;
using System;
using System.Windows.Data;

namespace RX_DigiPrint.Converters
{
	//--- StateImage_Converter ----------------------------------------------------
	public class SA_StateImage_Converter : IValueConverter
	{
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			switch((ECamFunctionState)value)
			{
				case ECamFunctionState.runningCam:	return "VideoOutline";
				case ECamFunctionState.waitRob:		return "VideoCheckOutline";
				case ECamFunctionState.runningRob:	return "RobotIndustrial";
				case ECamFunctionState.done:		return "Check";
				case ECamFunctionState.aborted:		return "Cancel";
				case ECamFunctionState.error:		return "AlertCircleOutline";
				case ECamFunctionState.manualCw:    return "AxisZRotateClockwise";
				case ECamFunctionState.manualCcw:   return "AxisZRotateCounterclockwise";
				default: return null;
			}
		}
		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            return null;
        }
	}
}
