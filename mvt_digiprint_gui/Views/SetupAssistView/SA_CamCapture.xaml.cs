using rx_CamLib;
using RX_Common;
using RX_DigiPrint.Models;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using static rx_CamLib.RxCam;

namespace RX_DigiPrint.Views.SetupAssistView
{
	/// <summary>
	/// Interaction logic for SA_CamCapture.xaml
	/// </summary>
	public partial class SA_CamCapture :UserControl
	{
		private RxCam _Camera;
		public SA_CamCapture()
		{
			InitializeComponent();
			_InitView();
		}

		public	void Start(RxCam camera)
		{
			_Camera = camera;
			_Camera.SelectCamera(RxGlobals.Settings.SetupAssistCam.Name);
			ENCamResult ret=_Camera.StartCamera(CameraCapture.Handle, false);
			if (ret==ENCamResult.OK)
			{
				FormHost.Visibility = Visibility.Visible;
				{
					System.Drawing.Point res=_Camera.GetCamStreamCaps().Resolution;
					int width = (int)CameraCapture_Grid.ActualWidth;
					int height = width*res.Y/res.X;
					CameraCapture_Height.Height = new GridLength(height);
					CameraCapture_Grid.Width    = width;
					CameraCapture.Height		= height;
					CameraCapture.Width			= width;
				}
				_Camera.SetVideoRectangle(CameraCapture.ClientRectangle);
				CameraCapture_Grid.DataContext = _Camera.Settings;
			}
			else
			{
				FormHost.Visibility = Visibility.Collapsed;
				_Camera.SetVideoRectangle(new System.Drawing.Rectangle());
				MvtMessageBox.Information("Setup Assist", "Align Filter not registered\n" + "Run >>rx_AlignFilter_Register.bat<< as administrator!", MessageBoxImage.Error);
			}
		}

		//--- Stop ------------------------
		public void Stop()
		{
			_Camera.StopCamera();
		}

		//--- _InitView -------------------------------
		private void _InitView()
		{
			List<RxEnum<int>> list = new List<RxEnum<int>>();

			list.Add(new RxEnum<int>(  0, "Camera"));
			list.Add(new RxEnum<int>(  1, "Process"));
			list.Add(new RxEnum<int>(  2, "Inverse"));
			list.Add(new RxEnum<int>( 11, "Histogram Manual"));
			list.Add(new RxEnum<int>( 12, "Histogram Auto"));
			list.Add(new RxEnum<int>( 13, "Histogram Adaptive"));

			CB_View.ItemsSource = list;
			CB_View.SelectedIndex = 0;
			ThresholdGrid.Visibility = Visibility.Collapsed;
		}

		//--- CB_View_SelectionChanged ----------------------------------------
		private void CB_View_SelectionChanged(object sender,SelectionChangedEventArgs e)
		{
			if (_Camera==null) return;

			RxEnum<int> item=CB_View.SelectedItem as RxEnum<int>;
			if (item==null) return;
			_Camera.Settings.ShowProcessImage = (item.Value==1);
			_Camera.Settings.Inverse		  = (item.Value==2);
			if (item.Value>10 && item.Value<=13) _Camera.Settings.BinarizeMode=(uint)item.Value-10;
			else								 _Camera.Settings.BinarizeMode=0;
			if (_Camera.Settings.BinarizeMode==1) ThresholdGrid.Visibility = Visibility.Visible;
			else ThresholdGrid.Visibility = Visibility.Collapsed;
		}
	}
}
