using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.ComponentModel;
using System.IO;
using System.Net;
using System.Reflection;
using System.Threading.Tasks;
using System.Timers;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Forms.VisualStyles;

namespace RX_DigiPrint.Views.UserControls
{
    public partial class UVLampView : UserControl
    {
        private const int _refreshPeriod = 15; // Seconds
        private const string _IPAddress = "192.168.200.222";
        private const int _lampMaxHours = 2000;

        public class UVLampData : INotifyPropertyChanged
        {
            private string _SoftVers;
            public string SoftVers
            {
                get { return _SoftVers; }
                set
                {
                    if (value == _SoftVers) return;
                    _SoftVers = value;
                    OnPropertyChanged("SoftVers");
                }
            }
            private string _SystemOpTime;
            public string SystemOpTime
            {
                get { return _SystemOpTime; }
                set
                {
                    if (value == _SystemOpTime) return;
                    _SystemOpTime = value;
                    OnPropertyChanged("SystemOpTime");
                }
            }
            private string _LampOpTime;
            public string LampOpTime
            {
                get { return _LampOpTime; }
                set
                {
                    if (value == _LampOpTime) return;
                    _LampOpTime = value;

                    Int32 t = 0;
                    try { t = Int32.Parse(_LampOpTime.Split('h')[0]); }
                    catch (Exception) { t = 0; }
                    LampOK = t < _lampMaxHours;

                    OnPropertyChanged("LampOpTime");
                }
            }
            private bool _LampOk;
            public bool LampOK
            {
                get { return _LampOk; }
                set
                {
                    if (value == _LampOk) return;
                    _LampOk = value;
                    OnPropertyChanged("LampOK");
                }
            }

            public event PropertyChangedEventHandler PropertyChanged;

            protected void OnPropertyChanged(string propertyName)
            {
                if (PropertyChanged != null)
                {
                    PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
                }
            }
        };

        private System.Timers.Timer _timer;
        private UVLampData _lampData = new UVLampData();
        
        public UVLampView()
        {
            this.DataContext = _lampData;
           
            InitializeComponent();
            
            Task.Run(() => GetLampStatus());

            SetTimer();

            RxGlobals.User.PropertyChanged += User_PropertyChanged;
            User_PropertyChanged(null, null);
        }

        public void ResetLampBtn_Click(object sender, RoutedEventArgs e)
        {
            if (RX_Common.MvtMessageBox.YesNo(RX_DigiPrint.Resources.Language.Resources.UvLamp, RX_DigiPrint.Resources.Language.Resources.ResetTheCounterOfUvLampOperatingHours, MessageBoxImage.Question, false))
            {
                Task.Run(() =>
                {
                    SendResetCommand();
                    GetLampStatus();
                });
            }
            ResetLampBtn.IsChecked = false;
        }

        private bool GetLampStatus()
        {
            string data;
            string url = String.Format(@"http://{0}/html/top.html?SysStatusData?", _IPAddress);

            //data = "Power=0&Voltage=0&Lampcurrent=0.00&PT1000=24&SysState=IDLE&SystemOpTime=1839h 51min&LampOpTime=839h 51min&SoftVers=1.9&PIDOUT=0.0&PIDSetValue=100&TemperatureDevMax=20&TemperatureMax=120&CoolDownTime= &XiSystemParamsState=";

            if (!SendHTTPGetRequest(url, out data))
            {
                _lampData.SoftVers = "Not connected";
                _lampData.SystemOpTime = "-";
                _lampData.LampOpTime = "-";
                return false;
            }

            string[] fields = data.Split('&');
            foreach (string field in fields)
            {
                string[] pair = field.Split('=');
                if (pair.Length == 2)
                {
                    PropertyInfo info = _lampData.GetType().GetProperty(pair[0]);
                    if (info != null)
                    {
                        info.SetValue(_lampData, pair[1]);
                    }
                }
            }
            return true;
        }

        private bool SendResetCommand()
        {
            string data;
            string url = String.Format(@"http://{0}/html/top.html?ResetLampOpTime?", _IPAddress);
            return SendHTTPGetRequest(url, out data);
        }

        private bool SendHTTPGetRequest(string url, out string data)
        {
            data = string.Empty;
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(url);
            request.Timeout = 3000;
            try
            {
                using (HttpWebResponse response = (HttpWebResponse)request.GetResponse())
                using (Stream stream = response.GetResponseStream())
                using (StreamReader reader = new StreamReader(stream))
                {
                    data = reader.ReadToEnd();
                }
            }
            catch (Exception)
            {
                return false;
            }
            return true;
        }

        private void SetTimer()
        {
            _timer = new System.Timers.Timer(_refreshPeriod * 1000);
            _timer.Elapsed += (sender, e) => OnTimedEvent(sender, e, this);
            _timer.AutoReset = true;
            _timer.Enabled = true;
        }

        private static void OnTimedEvent(Object source, ElapsedEventArgs e, UVLampView view)
        {
            if (view != null)
            {
                view.GetLampStatus();
            }
        }

        //--- User_PropertyChanged ----------------------------------
        private void User_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            Visibility visibility = (RxGlobals.User.UserType >= EUserType.usr_maintenance) ? Visibility.Visible : Visibility.Collapsed;
            SoftwareVersion.Visibility = visibility;
            SoftwareVersionValue.Visibility = visibility;
            SystemTime.Visibility = visibility;
            SystemTimeValue.Visibility = visibility;
            ResetLampBtn.Visibility = visibility;
        }
    }
}
