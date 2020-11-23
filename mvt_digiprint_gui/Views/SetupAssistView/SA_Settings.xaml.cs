using rx_CamLib;
using rx_CamLib.Models;
using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.SetupAssistView
{
    /// <summary>
    /// Interaction logic for AlignmentSettings.xaml
    /// </summary>
    public partial class SA_Settings : CustomWindow
    {
        private RxCam _Camera;
//      private StreamCaps _StreamCaps;
        private List<StreamCaps> _StreamCapsList;
     // private List<RxCamSettings.CamCapStruct> _CamCapsList;
     // private List<RxCamSettings.CamVideoProcStruct> _CamVideoProcList;

		//--- constructor -----------------------------------------------------
		public SA_Settings(RxCam cam)
        {
            InitializeComponent();
            DataContext = this;
            _Camera = cam;
            Init();
        }

        //--- Init --------------------------------
        private void Init()
		{
            _InitCamera();
            _InitStreamCaps();
		}

        //--- _InitCamera ----------------------------
        private void _InitCamera()
		{
            CB_Camera.ItemsSource  = _Camera.GetCameraList();
            CB_Camera.SelectedItem = RxGlobals.Settings.SetupAssistCam.Name;
            NB_DistToStops.Text    = Math.Round(RxGlobals.Settings.SetupAssistCam.DistToStop, 3).ToString();
		}

        //--- _InitStreamCaps -------------------------------
        private void _InitStreamCaps()
		{
            _StreamCapsList           = _Camera.GetCamStreamCapsList();
            CB_StreamCaps.ItemsSource = _StreamCapsList;
            StreamCaps caps=RxGlobals.Settings.SetupAssistCam.StreamCaps;
            if (_StreamCapsList==null || caps==null) return;
            foreach (StreamCaps item in _StreamCapsList)
			{
                if (!item.Mediasubtype.Equals(caps.Mediasubtype))   continue;
                if (item.FrameRate    != caps.FrameRate)            continue;
			    if (item.Resolution.X != caps.Resolution.X)         continue;
			    if (item.Resolution.Y != caps.Resolution.Y)         continue;
				{
                    CB_StreamCaps.SelectedItem = item;
                    return;
				}
			}
		}

        //--- Save_Clicked ------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            if (CB_Camera.SelectedItem!=null)
			{
                bool save=true;
                RxGlobals.Settings.SetupAssistCam.Name       = CB_Camera.SelectedItem as string;
                RxGlobals.Settings.SetupAssistCam.StreamCaps = CB_StreamCaps.SelectedItem as StreamCaps;
                RxGlobals.Settings.SetupAssistCam.DistToStop = NB_DistToStops.Value;

                if (save)
                {
                    RxGlobals.Settings.Save(null);
                    DialogResult = true;
                    return;
                }

			}
            DialogResult = false;
        }

        //--- Cancel_Clicked --------------------------------
        private void Cancel_Clicked(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
        }

        //--- Window_Loaded ----------------------------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            RxGlobals.Screen.PlaceWindow(this);
        }
    }
}

