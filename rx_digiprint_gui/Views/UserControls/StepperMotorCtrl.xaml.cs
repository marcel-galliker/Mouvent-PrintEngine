using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.UserControls
{
    /// <summary>
    /// Interaction logic for StepperMotorCtrl.xaml
    /// </summary>
    public partial class StepperMotorCtrl : UserControl
    {         
        //--- Constructor -------------------------------------------
        public StepperMotorCtrl()
        {
            // DataContext = StepperMotor
            InitializeComponent();
            RxGlobals.User.PropertyChanged += User_PropertyChanged;
            User_PropertyChanged(null, null);
        }

        //--- User_PropertyChanged ----------------------------------
        private void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            bool enabled = RxGlobals.User.UserType>=EUserType.usr_service;
            UpButton.IsEnabled   = enabled;
            DownButton.IsEnabled = enabled;
        }
        
        //--- Property Header ----------------------------------------
        private string _Header;
        public string Header
        {
            set {_Header = value; }
        }

        //--- GroupBox_Loaded ---------------------
        private void GroupBox_Loaded(object sender, RoutedEventArgs e)
        {
            (sender as GroupBox).Header = _Header;
        }

        //--- Up_Clicked ----------------------------
        private void Up_Clicked(object sender, RoutedEventArgs e)
        {
            StepperMotor motor = DataContext as StepperMotor;
            if (motor!=null) motor.DoTest(true);
        }

        //--- Down_Clicked ---------------------------------
        private void Down_Clicked(object sender, RoutedEventArgs e)
        {
            StepperMotor motor = DataContext as StepperMotor;
            if (motor!=null) motor.DoTest(false);
        }
    }
}
