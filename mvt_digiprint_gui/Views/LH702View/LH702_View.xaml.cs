using InTheHand.Net.Bluetooth;
using RX_Common;
using RX_DigiPrint.Comparers;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.PrintQueueView;
using RX_DigiPrint.Views.UserControls;
using System;
using System.Linq;
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
        
        private PrintQueueItem  _PrintingItem;


        public LH702_View()
        {
            InitializeComponent();
            RxGlobals.LH702_View = this;

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
                    Preview.ImgSettings = ImgSettings;
					RxGlobals.PrinterStatus.PropertyChanged += PrinterStatus_PropertyChanged;
                });
            }).Start();
        }

		//--- FileOpen_IsVisibleChanged ---------------------------------
		private void FileOpen_IsVisibleChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue) Button_Add_Icon.Kind = MahApps.Metro.IconPacks.PackIconMaterialKind.ArrowLeft;
            else                  Button_Add_Icon.Kind = MahApps.Metro.IconPacks.PackIconMaterialKind.Plus;
        }

        //--- Add_Clicked -------------------------------------------------
        private void Add_Clicked(object sender, RoutedEventArgs e)
        {
            if (FileOpen.Visibility == Visibility.Visible)
            {
                FileOpen.Visibility = Visibility.Collapsed;
            }
            else
            {
                FileOpen.Visibility = Visibility.Visible;
            }
        }

        //--- _PrintingItem_PropertyChanged ------------------------------------
		private void _PrintingItem_PropertyChanged(object sender,System.ComponentModel.PropertyChangedEventArgs e)
		{
			if (e.PropertyName.Equals("State"))
			{
                if (_PrintingItem!=null && _PrintingItem.State>EPQState.printing) PrintQueueChanged();
			}
		}

        //--- _Next_PropertyChanged ------------------------------------
		private void _Next_PropertyChanged(object sender,System.ComponentModel.PropertyChangedEventArgs e)
		{
			if (e.PropertyName.Equals("State")) PrintQueueChanged();
		}

        //--- PrinterStatus_PropertyChanged -----------------------------------------
		private void PrinterStatus_PropertyChanged(object sender,System.ComponentModel.PropertyChangedEventArgs e)
		{
			if (e.PropertyName.Equals("PrintState"))
			{
                if (RxGlobals.PrinterStatus.PrintState<EPrintState.ps_printing) 
                        Preview.ChangeJob(false);
                PrintQueueChanged();
			}
		}

        //--- UpdatePrintQueueItem ------------------------------------
        public void UpdatePrintQueueItem(PrintQueueItem item)
		{
            if (item.State==EPQState.transfer || item.State==EPQState.printing)
			{
                if (Preview.Next==null)  Console.WriteLine("PRINTING >>{0}<< {1}: Printing >>{2}<<, NULL", item.FileName, item.CopiesPrinted, _PrintingItem.FileName);
                else if (_PrintingItem!=null) Console.WriteLine("PRINTING >>{0}<< {1}: Printing >>{2}<<, next >>{3}<<", item.FileName, item.CopiesPrinted, _PrintingItem.FileName, Preview.Next.FileName);
                else Console.WriteLine("PRINTING >>{0}<<", item.FileName, item.CopiesPrinted);
                if (item!=_PrintingItem) 
                    PrintQueueChanged();
			}
		}

        //--- PrintQueueChanged ---------------------------
        public void PrintQueueChanged()
        {     
            int i=0;
            bool can_print;
            PrintQueueItem printing=null;
            PrintQueueItem next=null;

			/*
            if (true)
            {
                Console.WriteLine("\nPrintQueueChanged");
                for (i=0; i<RxGlobals.PrintQueue.Printed.Count; i++)
                    Console.WriteLine("Printed[{0}]={1}:>>{2}<<.state={3}", i, RxGlobals.PrintQueue.Printed[i].ID, RxGlobals.PrintQueue.Printed[i].FileName, RxGlobals.PrintQueue.Printed[i].State.ToString());
                if (RxGlobals.PrintQueue.Queue.Count>0)
					Console.WriteLine("Queued=>>{0}<<", RxGlobals.PrintQueue.Queue.Last().FileName);
			}
            */

            can_print = (RxGlobals.PrinterStatus.PrintState>=EPrintState.ps_printing && RxGlobals.PrinterStatus.PrintState<=EPrintState.ps_pause);
            i=RxGlobals.PrintQueue.Printed.Count;
			while (--i>=0)
            {
                if (   RxGlobals.PrintQueue.Printed[i].State==EPQState.transfer
                    || RxGlobals.PrintQueue.Printed[i].State==EPQState.printing)
				{
                    printing=RxGlobals.PrintQueue.Printed[i];
                    if (i>0) next = RxGlobals.PrintQueue.Printed[i-1];
                    break;
				}
			}

            if (next==null && RxGlobals.PrintQueue.Queue.Count>0) next = RxGlobals.PrintQueue.Queue.Last();

            if (next!=Preview.Next)
			{
                if (Preview.Next!=null) Preview.Next.PropertyChanged -= _Next_PropertyChanged;
                Preview.Next = next;
                if (Preview.Next!=null) Preview.Next.PropertyChanged += _Next_PropertyChanged;
			}

            if (!can_print) printing=null;
        //  if (printing==null) Console.WriteLine("PrintQueueChanged printing=>>NULL<<");
        //  else                Console.WriteLine("PrintQueueChanged printing={0}:>>{1}<<", printing.ID, printing.FileName);

        //  if (next==null) Console.WriteLine("PrintQueueChanged next=>>NULL<<");
        //  else            Console.WriteLine("PrintQueueChanged next={0}:>>{1}<<", next.ID, next.FileName);

            if (printing!=_PrintingItem)
			{
                Preview.ChangeJob(false);
                if (_PrintingItem!=null) _PrintingItem.PropertyChanged -=_PrintingItem_PropertyChanged;
                _PrintingItem=printing;
                if (_PrintingItem!=null) _PrintingItem.PropertyChanged +=_PrintingItem_PropertyChanged;
                Preview.DataContext = _PrintingItem;
			}
            Preview.UpdateSettings();
        }

		//--- Preview_Loaded ------------------------------
		private void Preview_Loaded(object sender, RoutedEventArgs e)
        {
            PrintQueueChanged();
        }

        //--- Preview_MouseDown ---------------------------
        private void Preview_MouseDown(object sender, System.Windows.Input.MouseButtonEventArgs e)
        {
            PrintQueueChanged(); // FileOpen.Visibility = Visibility.Visible;
        }

    }
}
