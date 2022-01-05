using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using RX_DigiPrint.Services;
using RX_Common;

namespace RX_DigiPrint.Views.UserControls
{
    public partial class ConsumablesView : UserControl
    {
        public ConsumablesView()
        {
            InitializeComponent();
            _create_controls();
        }

        //--- _create_controls --------------------------------
        private void _create_controls()
        { 
            for (int i=0; i<TcpIp.InkSupplyCnt+2; i++)
            {
                MainGrid.ColumnDefinitions.Add(new ColumnDefinition(){Width=GridLength.Auto});
            }
        }

        //--- _BindValues ------------------------------
        private EPrinterType _PrinterType;
        private void _BindValues()
        {
            if (RxGlobals.PrintSystem.PrinterType!=_PrinterType)
            {
                MainGrid.Children.Clear();

                // Add more ink levels when needed
                int additionalInkLevels = 0;
                if (RxGlobals.PrintSystem.IsTx || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_table_seon) additionalInkLevels = 1;    // Flush
                if (RxGlobals.PrintSystem.IsLb) additionalInkLevels = 2;    // Flush + waste
                if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LH702) additionalInkLevels = 0;  // Neither flush or waste
                for (int i=0; i<additionalInkLevels; i++)
                {
                    InkLevel additionalLevel = new InkLevel(RxGlobals.InkSupply.List[TcpIp.InkSupplyCnt + i]);
                    Grid.SetColumn(additionalLevel, i);
                    MainGrid.Children.Add(additionalLevel);
                }

                for (int i=0; i<RxGlobals.PrintSystem.ColorCnt; i++)
                {
                    if ( RxGlobals.PrintSystem.IS_Order!=null)
                    {
                        InkLevel ctrl = null;
                        int inkCylinderIndex = i * RxGlobals.PrintSystem.InkCylindersPerColor;
                        if (inkCylinderIndex < RxGlobals.PrintSystem.IS_Order.Length)
                        {
                            ctrl = new InkLevel(RxGlobals.InkSupply.List[RxGlobals.PrintSystem.IS_Order[inkCylinderIndex]]);
                        }
                        else
                        {
                            // configuration missmatch!!
                            ctrl = new InkLevel();
                        }
                        Grid.SetColumn(ctrl, additionalInkLevels + i);
                        MainGrid.Children.Add(ctrl);
                    }
                }

                if (RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_cleaf || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_test_table_seon)
                {
                    InkLevel waste = new InkLevel() { DataContext = RxGlobals.InkSupply.List[TcpIp.InkSupplyCnt + 1] };
                    Grid.SetColumn(waste, TcpIp.InkSupplyCnt + 1);
                    MainGrid.Children.Add(waste);
                }

                _PrinterType = RxGlobals.PrintSystem.PrinterType;
            }
        }

        private void UserControl_IsVisibleChanged(object sender,DependencyPropertyChangedEventArgs e)
        {
            if ((bool)e.NewValue==true) _BindValues();
        }
    }
}