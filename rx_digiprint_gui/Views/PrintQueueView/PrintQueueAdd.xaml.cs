using iTextSharp.text.pdf;
using RX_DigiPrint.Models;
using System;
using System.Drawing.Imaging;
using System.IO;
using System.Windows;
using RX_LabelComposer.External;
using System.Runtime.InteropServices;
using RX_Common;
using System.Text;
using RX_DigiPrint.Views.UserControls;
using System.Windows.Controls;
using System.Windows.Threading;
using System.Windows.Input;

namespace RX_DigiPrint.Views.PrintQueueView
{
    public partial class PrintQueueAdd : Window
    {
        private PrintQueueItem                      _item = new PrintQueueItem();
        private DispatcherTimer                     _timer;
        private System.Windows.Input.TouchEventArgs _touchEvtArgs;
        private object                              _source;

        //--- Constructor --------------------------------------------------
        public PrintQueueAdd()
        {
            InitializeComponent();
            DataContext = _item;
            _item.FilePath="Test";
            _item.FirstPage=1;
            _item.LastPage=1;
            _item.Copies=1;
            _item.Collate=0;
            _item.LengthUnit = Services.EPQLengthUnit.copies;
            _item.ScanLength = 1;
            _item.Passes     = 1;

            FileSettingsWeb.SaveButton.Visibility=Visibility.Collapsed;
            FileSettingsScan.SaveButton.Visibility=Visibility.Collapsed;

            _timer = new DispatcherTimer();
            _timer.Interval = new TimeSpan(0, 0, 0, 0, 100);
            _timer.Tick += _timer_Tick;
        }

        void _timer_Tick(object sender, EventArgs e)
        {
            _timer.Stop();
            if (_touchEvtArgs!=null)
            {
                (_source as Control).RaiseEvent(_touchEvtArgs);
            }
            _touchEvtArgs=null;
        }

        //--- Window_Loaded ------------------------------------------
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            RxGlobals.Screen.PlaceWindow(this);
//            TouchDevice device = new TouchDevice();
//            int timestamp = 1;
//            System.Windows.Input.TouchEventArgs touchEvent = new TouchEventArgs(device, timestamp);
//            RaiseEvent(touchEvent);
        }

        //--- Item ---------------------------------------------------------
        public PrintQueueItem Item
        {
            get { return _item; }
        }       

        //--- File_Clicked -------------------------------------------------
        private void File_Clicked(object sender, RoutedEventArgs e)
        {
            RxFileOpen dlg = new RxFileOpen(){RootButton=Properties.Settings.Default.FileOpen_DataSource, InitDir=Properties.Settings.Default.FileOpen_ActDir};
            bool? ret=dlg.ShowDialog();
            if (ret!=null && ret==true)
            {
                _item.FilePath = dlg.Selected;
                _item.read_image_properties(_item.FilePath);
                _item.LoadDefaults(false);
                
                //--- save actual position ----------------------
                Properties.Settings.Default.FileOpen_DataSource = dlg.RootButton;
                Properties.Settings.Default.FileOpen_ActDir     = dlg.ActDir;
                Properties.Settings.Default.Save();
            }
        }

        //--- Print_Clicked -------------------------------------------------
        private void Print_Clicked(object sender, RoutedEventArgs e)
        {
            this.DialogResult=true;         
        }

        private int _cnt=0;
        private void Window_TouchUp(object sender, System.Windows.Input.TouchEventArgs e)
        {
            _cnt++;
            Console.WriteLine("Window_TouchUp {0}", _cnt);
            Console.WriteLine("Source = {0}", e.Source.ToString());
            if (_source!=null) Console.WriteLine("_Source = {0}", _source.ToString());
            Console.WriteLine("RoutedEvent = {0}", e.RoutedEvent.ToString());
            Console.WriteLine("TouchDevice = {0}", e.TouchDevice);
            Console.WriteLine("Handled = {0}", e.Handled);
            Console.WriteLine("OriginalSource {0}", e.OriginalSource.ToString());

            if (false && _cnt==1)
            {
                Button b = e.Source as Button;

                _touchEvtArgs = new System.Windows.Input.TouchEventArgs(e.TouchDevice, e.Timestamp+10);
                _touchEvtArgs.RoutedEvent = UIElement.TouchUpEvent;
                _touchEvtArgs.Source = this;
                _source = e.Source;

                 (_source as Control).RaiseEvent(_touchEvtArgs);

                // _timer.Start();


             //   if (b!=null)
            //        RaiseEvent(new RoutedEventArgs(TouchUpEvent, sender, e));       
            }
        }

        private void Window_GotStylusCapture(object sender, StylusEventArgs e)
        {

        }
    }
}
