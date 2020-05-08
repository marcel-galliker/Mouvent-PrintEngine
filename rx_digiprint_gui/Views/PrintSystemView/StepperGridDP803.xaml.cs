using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.PrintSystemView
{
    public partial class StepperGridDP803 : UserControl
    {
        private RxNumBox[,] _numbox= new  RxNumBox[4,2];

        public StepperGridDP803()
        {
            InitializeComponent();
            DataContext = RxGlobals.Stepper;
            RxGlobals.Stepper.PropertyChanged += Stepper_PropertyChanged;
            Col0.DataContext = RxGlobals.InkSupply.List[7];
            Col1.DataContext = RxGlobals.InkSupply.List[6];
            Col2.DataContext = RxGlobals.InkSupply.List[5];
            Col3.DataContext = RxGlobals.InkSupply.List[4];
            Col4.DataContext = RxGlobals.InkSupply.List[0];
            Col5.DataContext = RxGlobals.InkSupply.List[1];
            Col6.DataContext = RxGlobals.InkSupply.List[2];
            Col7.DataContext = RxGlobals.InkSupply.List[3];

            //--- allocate boxes ---------------------------
            int x, y;
            for (x=0; x<_numbox.GetLength(0); x++)
            {
                for (y=0; y<_numbox.GetLength(1); y++)
                {
                    _numbox[x,y] = new RxNumBox();
                    _numbox[x,y].Tag = (100*x+y).ToString();
                    _numbox[x,y].LostFocus += box_LostFocus;
                    _numbox[x,y].HorizontalContentAlignment = System.Windows.HorizontalAlignment.Right;
                    if (x<2) Grid.SetColumn(_numbox[x,y], 4+x);
                    else     Grid.SetColumn(_numbox[x,y], 5+x);
                    Grid.SetRow(_numbox[x,y], 1+y);
                    MainGrid.Children.Add(_numbox[x,y]);
                }
            }

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
            RxNumBox box = sender as RxNumBox;
            int tag = Rx.StrToInt32(box.Tag as string);
            int no=tag/100;
            int row=tag%100;
            switch(row)
            {
                case 0: RxGlobals.Stepper.Robot[no].ref_height  = (System.Int32)(box.Value*1000); break;
                case 1: RxGlobals.Stepper.Robot[no].head_align  = (System.Int32)(box.Value*1000); break;
                default: break;
            }
            RxGlobals.Stepper.Changed=true;
        }

        //--- _update ----------------------------------
        private void _update()
        {
            int no;
            for (no=0; no<4; no++)
            {
                _numbox[no,0].Text = (RxGlobals.Stepper.Robot[no].ref_height /1000.0).ToString();
                _numbox[no,1].Text = (RxGlobals.Stepper.Robot[no].head_align /1000.0).ToString();
            }
        }
    }
}
