using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Views.PrintQueueView;
using RX_DigiPrint.Views.UserControls;
using System;
using System.Threading;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.LH702View
{
    /// <summary>
    /// Interaction logic for LB701UVView.xaml
    /// </summary>
    public partial class LH702_View : UserControl
    {
        private FileOpen        FileOpen;

        public LH702_View()
        {
            InitializeComponent();
            RxGlobals.PrintQueueChanged += _PrintQueueChanged;

            new Thread(() =>
            {
                RxBindable.Invoke(() =>
                {
                    FileOpen       = new FileOpen();
                    FileOpen.Visibility      = Visibility.Collapsed;
                    Grid.SetRow(FileOpen, 1);
                    Grid.SetRowSpan(FileOpen, 3);
                    FileOpen.IsVisibleChanged += FileOpen_IsVisibleChanged;
                    MainGrid.Children.Add(FileOpen);
                });
            }).Start();

        }

        //--- FileOpen_IsVisibleChanged ---------------------------------
        private void FileOpen_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
        //    Button_Add.IsChecked = (bool)e.NewValue;        
        }

        //--- Add_Clicked -------------------------------------------------
        private void Add_Clicked(object sender, RoutedEventArgs e)
        {
            if (FileOpen.Visibility==Visibility.Visible)    FileOpen.Visibility = Visibility.Collapsed;
            else                                            FileOpen.Visibility = Visibility.Visible;
        }

        //--- _PrintQueueChanged ---------------------------
        private void _PrintQueueChanged()
        {
            Preview.DataContext = RxGlobals.NextItem;
        }

        //--- Preview_Loaded ------------------------------
        private void Preview_Loaded(object sender, RoutedEventArgs e)
        {
            _PrintQueueChanged();
        }

        //--- Preview_MouseDown ---------------------------
        private void Preview_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            _PrintQueueChanged(); // FileOpen.Visibility = Visibility.Visible;
        }

    }
}
