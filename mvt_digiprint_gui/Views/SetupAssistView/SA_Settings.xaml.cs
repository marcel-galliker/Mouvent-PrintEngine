using rx_CamLib;
using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Models.Enums;
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
    public partial class SA_Settings : CustomWindow, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        //--- SetProperty ------------------------------------
        private bool SetProperty<type>(ref type property, type val, [CallerMemberName]string propName = null)
        {
            if (property == null || !property.Equals(val))
            {
                property = val;
                if (PropertyChanged != null) PropertyChanged(this, new PropertyChangedEventArgs(propName));
                return true;
            }
            return false;
        }

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
            CB_Camera.ItemsSource = _Camera.GetCameraList();
            CB_Camera.SelectedItem = RxGlobals.Settings.SetupAssistCam;
		}

        //--- Save_Clicked ------------------------------------
        private void Save_Clicked(object sender, RoutedEventArgs e)
        {
            if (CB_Camera.SelectedItem!=null)
			{
                string newCam = CB_Camera.SelectedItem as string;
                if (newCam!=RxGlobals.Settings.SetupAssistCam)
				{
                    RxGlobals.Settings.SetupAssistCam = newCam;
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

