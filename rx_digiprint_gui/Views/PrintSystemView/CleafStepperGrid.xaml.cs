using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.PrintSystemView
{
    public partial class CleafStepperGrid : UserControl
    {
        private RxNumBox[,] _numbox= new  RxNumBox[4,4];

        public CleafStepperGrid()
        {
            InitializeComponent();
            DataContext = RxGlobals.Stepper;
            RxGlobals.Stepper.PropertyChanged += Stepper_PropertyChanged;
            Col0.DataContext = RxGlobals.InkSupply.List[0];
            Col1.DataContext = RxGlobals.InkSupply.List[1];
            Col2.DataContext = RxGlobals.InkSupply.List[2];
            Col3.DataContext = RxGlobals.InkSupply.List[3];

            //--- allocate boxes ---------------------------
            int x, y;
            for (x=0; x<_numbox.GetLength(0); x++)
            {
                for (y=0; y<_numbox.GetLength(1); y++)
                {
                    _numbox[x,y] = new RxNumBox();
                    _numbox[x,y].LostFocus += box_LostFocus;
                    _numbox[x,y].HorizontalContentAlignment = System.Windows.HorizontalAlignment.Right;
                    Grid.SetColumn(_numbox[x,y], 4+x); 
                    Grid.SetRow(_numbox[x,y], y); 
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
            int no=Grid.GetColumn(box)-4;
            int row=Grid.GetRow(box);
            switch(row)
            {
                case 0: RxGlobals.Stepper.Robot[no].ref_height  = (System.Int32)(box.Value*1000); break;
                case 1: RxGlobals.Stepper.Robot[no].head_align  = (System.Int32)(box.Value*1000); break;
                case 2: RxGlobals.Stepper.Robot[no].robot_height = (System.Int32)(box.Value*1000); break;
                case 3: RxGlobals.Stepper.Robot[no].robot_align  = (System.Int32)(box.Value*1000); break;
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
                _numbox[no,2].Text = (RxGlobals.Stepper.Robot[no].robot_height/1000.0).ToString();
                _numbox[no,3].Text = (RxGlobals.Stepper.Robot[no].robot_align /1000.0).ToString();
            }
        }
    }
}
