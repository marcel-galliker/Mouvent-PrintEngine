using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.PrintSystemView
{
    public partial class StepperGridLB702 : UserControl
    {
        private MvtNumBox[,] _numbox = new MvtNumBox[4, 3];
        private bool IsStandardPrintSystemView { get; set; }
        private EPrinterType CurrentPrinterType { get; set; }

        public StepperGridLB702()
        {
            InitializeComponent();
            IsStandardPrintSystemView = true;
            CurrentPrinterType = RxGlobals.PrintSystem.PrinterType;

            //--- allocate boxes ---------------------------
            int x, y;
            for (x = 0; x < _numbox.GetLength(0); x++)
            {
                for (y = 0; y < _numbox.GetLength(1); y++)
                {
                    _numbox[x, y] = new MvtNumBox();
                    _numbox[x, y].Tag = (100 * x + y).ToString();
                    _numbox[x, y].LostFocus += box_LostFocus;
                    _numbox[x, y].HorizontalContentAlignment = System.Windows.HorizontalAlignment.Right;
                    Grid.SetColumn(_numbox[x, y], 1 + x);
                    Grid.SetRow(_numbox[x, y], 2 + y);
                    MainGrid.Children.Add(_numbox[x, y]);
                }
            }
        }

        public void SetDataContext()
        {
            DataContext = RxGlobals.Stepper;
            ManualFlightTimeComp.DataContext = RxGlobals.PrintSystem;
            RxGlobals.Stepper.PropertyChanged += Stepper_PropertyChanged;
            /*Col0.DataContext = RxGlobals.InkSupply.List[0];
            Col1.DataContext = RxGlobals.InkSupply.List[1];
            Col2.DataContext = RxGlobals.InkSupply.List[2];
            Col3.DataContext = RxGlobals.InkSupply.List[3];
            Col4.DataContext = RxGlobals.InkSupply.List[4];
            Col5.DataContext = RxGlobals.InkSupply.List[5];
            Col6.DataContext = RxGlobals.InkSupply.List[6];
            Col7.DataContext = RxGlobals.InkSupply.List[7];*/
            IsStandardPrintSystemView = true;
            _update();
        }

        public void SetDataContext(PrintSystemExtendedView.StepperValues values)
        {
            DataContext = values;
            /*Col0.DataContext = RxGlobals.InkSupply.List[0];
            Col1.DataContext = RxGlobals.InkSupply.List[1];
            Col2.DataContext = RxGlobals.InkSupply.List[2];
            Col3.DataContext = RxGlobals.InkSupply.List[3];
            Col4.DataContext = RxGlobals.InkSupply.List[4];
            Col5.DataContext = RxGlobals.InkSupply.List[5];
            Col6.DataContext = RxGlobals.InkSupply.List[6];
            Col7.DataContext = RxGlobals.InkSupply.List[7];*/
            IsStandardPrintSystemView = false;
            _update();
        }

        public void SetPrinterType(EPrinterType type)
        {
            CurrentPrinterType = type;
            _update();
        }

        //--- Stepper_PropertyChanged -------------------------------------
        void Stepper_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("Robot"))
                _update();
        }

        //--- box_LostFocus ------------------------------------
        private void box_LostFocus(object sender, RoutedEventArgs e)
        {
            MvtNumBox box = sender as MvtNumBox;
            int tag = Rx.StrToInt32(box.Tag as string);
            int no = tag / 100;
            int row = tag % 100;

            if (IsStandardPrintSystemView)
            {
                switch (row)
                {
                    case 0: RxGlobals.Stepper.Robot[no].ref_height_back = (System.Int32)(box.Value * 1000); break;
                    case 1: RxGlobals.Stepper.Robot[no].ref_height_front = (System.Int32)(box.Value * 1000); break;
                    case 2: RxGlobals.Stepper.Robot[no].cap_height = (System.Int32)(box.Value * 1000); break;
                    default: break;
                }
                RxGlobals.Stepper.Changed = true;
            }
            else  
            {
                var stepperValues = DataContext as PrintSystemExtendedView.StepperValues;
                if (stepperValues == null)
                {
                    Console.WriteLine("Error: Cannot acces stepper values");
                    return;
                }
                switch (row)
                {
                    case 0: stepperValues.Robot[no].ref_height_back = (System.Int32)(box.Value * 1000); break;
                    case 1: stepperValues.Robot[no].ref_height_front = (System.Int32)(box.Value * 1000); break;
                    case 2: stepperValues.Robot[no].cap_height = (System.Int32)(box.Value * 1000); break;
                    default: break;
                }
            }
        }

        //--- _update ----------------------------------
        static readonly GridLength _hidden = new GridLength(0);
        private void _update()
        {
            int no;

            if (IsStandardPrintSystemView)
            {
                for (no = 0; no < 4; no++)
                {
                    _numbox[no, 0].Text = (RxGlobals.Stepper.Robot[no].ref_height_back / 1000.0).ToString();
                    _numbox[no, 1].Text = (RxGlobals.Stepper.Robot[no].ref_height_front / 1000.0).ToString();
                    _numbox[no, 2].Text = (RxGlobals.Stepper.Robot[no].cap_height / 1000.0).ToString();
                }
                if (RxGlobals.StepperStatus[0].RobotUsed || RxGlobals.PrintSystem.PrinterType == EPrinterType.printer_LB702_WB)
                {
                    WipingDelay_Height.Height = GridLength.Auto;
                    WipingSpeed_Height.Height = GridLength.Auto;
                    CapHeight.Height = GridLength.Auto;
                }
                else
                {
                    WipingDelay_Height.Height = _hidden;
                    WipingSpeed_Height.Height = _hidden;
                    CapHeight.Height = _hidden;
                }
            }
            else
            {
                var stepperValues = DataContext as PrintSystemExtendedView.StepperValues;
                if (stepperValues == null)
                {
                    Console.WriteLine("Error: Cannot acces stepper values");
                    return;
                }
                for (no = 0; no < 4; no++)
                {
                    _numbox[no, 0].Text = (stepperValues.Robot[no].ref_height_back / 1000.0).ToString();
                    _numbox[no, 1].Text = (stepperValues.Robot[no].ref_height_front / 1000.0).ToString();
                    _numbox[no, 2].Text = (stepperValues.Robot[no].cap_height / 1000.0).ToString();
                }
                if (RxGlobals.StepperStatus[0].RobotUsed || CurrentPrinterType == EPrinterType.printer_LB702_WB)
                {
                    WipingDelay_Height.Height = GridLength.Auto;
                    WipingSpeed_Height.Height = GridLength.Auto;
                    CapHeight.Height = GridLength.Auto;
                }
                else
                {
                    WipingDelay_Height.Height = _hidden;
                    WipingSpeed_Height.Height = _hidden;
                    CapHeight.Height = _hidden;
                }
            }
        }
    }
}
