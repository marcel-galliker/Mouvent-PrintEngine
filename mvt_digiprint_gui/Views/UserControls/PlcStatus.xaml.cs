using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for PlcStatus.xaml
    /// </summary>
    public partial class PlcStatus : UserControl
    {
        public PlcStatus()
        {
            InitializeComponent();

            RxGlobals.User.PropertyChanged += User_PropertyChanged;
            User_PropertyChanged(null, null);
        }

        private void PLC_INFO_IsVisibleChanged(object sender, System.Windows.DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue)
            {
                RxGlobals.RxInterface.SendCommand(TcpIp.CMD_PLC_GET_INFO);
                RxGlobals.Plc.PropertyChanged += Plc_PropertyChanged;
            }
        }

        //--- Plc_PropertyChanged ------------------------------------
        void Plc_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Info"))
            {
                SerielNo.Text       = RxGlobals.Plc.Info.serialNo;
                Temparature.Text    = String.Format("{0}°C", RxGlobals.Plc.Info.tempAct);
                TemparatureMax.Text = String.Format("{0}°C", RxGlobals.Plc.Info.tempMax);
                VersionHW.Text      = RxGlobals.Plc.Info.versionHardware;
                VersionFW.Text      = RxGlobals.Plc.Info.versionFirmware;
                VersionLogic.Text   = RxGlobals.Plc.Info.versionLogic;  
                VersionBSP.Text     = RxGlobals.Plc.Info.versionBsp;
                VersionMPLI.Text    = RxGlobals.Plc.Info.versionMlpi;
                HardwareDetail.Text = RxGlobals.Plc.Info.hardwareDetails.Replace("; ", "\n").Replace(';', '\n');
                LocalBusConfig.Text = RxGlobals.Plc.Info.localBusConfig.Replace("; ", "\n").Replace(';', '\n');
                ModulBusConfig.Text = RxGlobals.Plc.Info.modulBusConfig.Replace("; ", "\n").Replace(';', '\n');      
                ModulHwDetails.Text = RxGlobals.Plc.Info.modulHwDetails.Replace("; ", "\n").Replace(';', '\n');
            }
        }

        //--- User_PropertyChanged ----------------------------------
        private void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            Visibility visibility = (RxGlobals.User.UserType >= EUserType.usr_maintenance) ? Visibility.Visible : Visibility.Collapsed;
            PlcVersion.Visibility = visibility;
        }
    }
}
