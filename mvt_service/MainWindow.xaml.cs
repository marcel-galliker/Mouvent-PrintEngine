using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Timers;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using RX_DigiPrint.Helpers;

namespace mvt_service
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private const String VERSION_PLACEHOLDER = "1.5";
        private const String MACHINE_NAME_PLACEHOLDER = "Unused";
        private const String PIN_INFO_TEXT = "Actual Pin for version ";

        private readonly MvtUserLevelManager mml = new MvtUserLevelManager(VERSION_PLACEHOLDER, MACHINE_NAME_PLACEHOLDER);
        private Timer pinTimer = new Timer(10000);
        private MvtTOTP codes;

        public MainWindow()
        {
            InitializeComponent();

            codes = new MvtTOTP(mml.CreateMouventSecret(), true, null);

            PinInfo.Text = PIN_INFO_TEXT + VERSION_PLACEHOLDER;
            PinValue.Text = codes.GetPin();

            pinTimer.Elapsed += this.PinRefresh;
            pinTimer.AutoReset = true;
            pinTimer.Start();
        }

        private void CodeCreationButton_Click(object sender, RoutedEventArgs e)
        {
            if(!String.IsNullOrWhiteSpace(VersionTB.Text))
            {
                codes = new MvtTOTP(MvtUserLevelManager.CreateMouventSecret(VersionTB.Text), true, null);

                PinInfo.Text = PIN_INFO_TEXT + VersionTB.Text;
                PinValue.Text = codes.GetPin();
                QRCode.Source = codes.QRImgSrc();
                ManualKey.Text = codes.GetManualCode();
            }
        }

        private void PinRefresh(object sender, ElapsedEventArgs e)
        {
            this.Dispatcher.Invoke(() =>
            {
                PinValue.Text = codes.GetPin();
            });
        }

        private void QuitButton_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
