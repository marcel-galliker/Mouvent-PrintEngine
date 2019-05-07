using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RxWfTool.Views
{
    /// <summary>
    /// Interaction logic for GreyLevelView.xaml
    /// </summary>
    public partial class GreyLevelView : UserControl
    {
        static int _GreyLevelCnt = 4;   // rows
        static int _DropletCnt   = 4;   // columns

        CheckBox[][] _CheckBox = new CheckBox[_GreyLevelCnt][];

        //--- constructor ----------------------------------------
        public GreyLevelView()
        {
            InitializeComponent();
        }

        //--- UserControl_Loaded -----------------------------------
        private void UserControl_Loaded(object sender, RoutedEventArgs e)
        {
            int row, col;
            for (row=0; row<_GreyLevelCnt; row++)
            {           
                _CheckBox[row] = new CheckBox[_DropletCnt];
                for  (col=0; col<_DropletCnt; col++)
                {
                    _CheckBox[row][col] = new CheckBox()
                         {
                             VerticalAlignment=VerticalAlignment.Center, 
                             HorizontalAlignment=HorizontalAlignment.Center, 
                             Margin=new Thickness(2),
                         };
                    Grid.SetColumn(_CheckBox[row][col], col+2);
                    Grid.SetRow   (_CheckBox[row][col], row+2);
                    _CheckBox[row][col].Name = string.Format("GreyLevel_{0}_{1}", row,col);
                    _CheckBox[row][col].Checked   += _GreyLevelView_Checked;
                    _CheckBox[row][col].Unchecked += _GreyLevelView_Unchecked;
                    Grid.Children.Add(_CheckBox[row][col]);
                }
            }
            _show_greylevels();
        }

        //--- Property GreyLevel ---------------------------------------
        private int[] _GreyLevel;
        public int[] GreyLevel
        {
            get { return _GreyLevel; }
            set 
            { 
                _GreyLevel = value;
                _show_greylevels();
            }
        }        

        //--- _GreyLevelView_Checked ---
        void _GreyLevelView_Checked(object sender, RoutedEventArgs e)
        {
            try
            {
                CheckBox box = sender as CheckBox;
                string[] str = box.Name.Split('_');
                int row = System.Convert.ToInt32(str[1]);
                int col = System.Convert.ToInt32(str[2]);    
                _GreyLevel[row] |= (1<<col);
            }
            catch(Exception)
            {
            }               
        }

        //--- _GreyLevelView_Unchecked ---
        void _GreyLevelView_Unchecked(object sender, RoutedEventArgs e)
        {
            try
            {
                CheckBox box = sender as CheckBox;
                string[] str = box.Name.Split('_');
                int row = System.Convert.ToInt32(str[1]);
                int col = System.Convert.ToInt32(str[2]);    
                _GreyLevel[row] &= ~(1<<col);
            }
            catch(Exception)
            {
            }               
        }

        //--- _show_greylevels ------------------------------
        private void _show_greylevels()
        {
            try
            {
                for (int row=0; row<_CheckBox.Count(); row++)
                {
                    if (_CheckBox[row]!=null)
                    {
                        for (int col=0; col<_DropletCnt; col++)
                            _CheckBox[row][col].IsChecked = (_GreyLevel[row] & (1<<col))!=0;
                    }
                }
            }
            catch(Exception)
            { }
        }
    }
}
