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
		}

        //--- _InitCamera ----------------------------
        private void _InitCamera()
		{
            CB_Camera.ItemsSource    = _Camera.GetCameraList();
            CB_ToleranceAngle.ItemsSource = new SA_Tolerance_List();
            CB_ToleranceStitch.ItemsSource = new SA_Tolerance_List();

            CB_Camera.SelectedItem = RxGlobals.Settings.SetupAssistCam.Name;
            NB_DistToStops.Text    = Math.Round(RxGlobals.Settings.SetupAssistCam.DistToStop, 3).ToString();
            CB_ToleranceAngle.SelectedValue = RxGlobals.Settings.SetupAssistCam.ToleranceAngle;
            CB_ToleranceStitch.SelectedValue = RxGlobals.Settings.SetupAssistCam.ToleranceStitch;
        }

        //--- Save_Clicked ------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            bool save=true;
            RxGlobals.Settings.SetupAssistCam.Name       = CB_Camera.SelectedItem as string;
            RxGlobals.Settings.SetupAssistCam.DistToStop = NB_DistToStops.Value;
            RxGlobals.Settings.SetupAssistCam.ToleranceAngle  = Convert.ToDouble(CB_ToleranceAngle.SelectedValue);
            RxGlobals.Settings.SetupAssistCam.ToleranceStitch = Convert.ToDouble(CB_ToleranceStitch.SelectedValue);

            if (save)
            {
                RxGlobals.Settings.Save(null);
                DialogResult = true;
                return;
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