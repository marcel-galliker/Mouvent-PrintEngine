using MahApps.Metro.IconPacks;
using rx_CamLib;
using rx_CamLib.Models;
using System;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;

namespace RX_DigiPrint.Views.SetupAssistView
{
	/// <summary>
	/// Interaction logic for SA_CamSettings.xaml
	/// </summary>
	public partial class SA_CamSettings :UserControl
	{
		//--- Datacontext = Camera -----------------------

		private RxCam _Camera;

		public SA_CamSettings()
		{
			InitializeComponent();
			DataContext = this;
		}

		//--- Show ------------------------------------------------
		public void Show(RxCam cam)
		{
			_Camera				 = cam;
			Settings.ItemsSource = CamGlobals.CamDevice.GetProperties();
			this.Visibility		 = Visibility.Visible;
		}

		//--- Hide -------------------------------------------------
		public void Hide()
		{
			this.Visibility = Visibility.Collapsed;
		}

		//--- Default_Clicked ---------------------------------------------
		private void Default_Clicked(object sender,RoutedEventArgs e)
		{
			CamDeviceProperty item = Settings.ActiveItem as CamDeviceProperty;
			item.Value = item.Default;
			e.Handled = true;
		}

		//--- Auto_Clicked ---------------------------------------------
		private void Auto_Clicked(object sender,RoutedEventArgs e)
		{
			CamDeviceProperty item = Settings.ActiveItem as CamDeviceProperty;
			item.Auto = !item.Auto;
		}
	}

	//--- FlagVisible_Converter ----------------------------------------------------
	public class PropName_Converter : IValueConverter
	{
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			string src = value as String;
			StringBuilder dst = new StringBuilder(64);
			int i;
			dst.Append(src[0]);
			for (i=1; i<src.Length; i++)
			{
				if (src[i]>='A' && src[i]<='Z') dst.Append(' ');
				dst.Append(src[i]);
			}
			return dst.ToString();
		}
		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
			return null;
        }
	}

	//--- FlagVisible_Converter ----------------------------------------------------
	public class IsDefault_Converter : IValueConverter
	{
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			if ((bool)value) return PackIconMaterialDesignKind.Star;
			return PackIconMaterialDesignKind.StarBorder;
		}
		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
			return null;
        }
	}

}
