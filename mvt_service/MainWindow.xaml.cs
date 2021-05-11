using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
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
        private const String MACHINE_NAME_PLACEHOLDER = "Unused";
        private const String PIN_INFO_TEXT = "Actual Pin for version ";

        private readonly MvtUserLevelManager mml;
        private Timer pinTimer = new Timer(10000);
        private MvtTOTP codes;

        public MainWindow()
        {
            InitializeComponent();

            Version v = Assembly.GetExecutingAssembly().GetName().Version;
            string version = string.Format("{0}.{1}", v.Major, v.Minor);

            mml = new MvtUserLevelManager(version, MACHINE_NAME_PLACEHOLDER);
            codes = new MvtTOTP(mml.CreateMouventSecret(), true, null);

            PinInfo.Text = PIN_INFO_TEXT + version;
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
