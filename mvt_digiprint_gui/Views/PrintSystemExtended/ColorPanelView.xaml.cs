using RX_DigiPrint.Models;
using System.ComponentModel;
using System.Windows.Controls;
using System.Windows.Media;

namespace RX_DigiPrint.Views.PrintSystemExtendedView
{
    /// <summary>
    /// Interaction logic for ColorPanelView.xaml
    /// </summary>

    public partial class ColorPanelView : UserControl, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        private PrintSystem _PrintSystem = RxGlobals.PrintSystem;

        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        private SolidColorBrush _inkColor;
        public SolidColorBrush InkColor
        {
            get
            {
                return _inkColor;
            }
            set
            {
                _inkColor = value;
                OnPropertyChanged("InkColor");
            }
        }

        private string _header { get; set; }
        public string Header {
            get
            {
                return _header;
            }
            set
            {
                _header = value;
                OnPropertyChanged("Header");
            }
        }

        private int ColorIndex { get; set; }

        public ColorPanelView(int colorIndex)
        {
            InkColor = Brushes.Transparent;
            Header = "Color " + System.Convert.ToString(colorIndex+1);
            ColorIndex = colorIndex;
            InitializeComponent();
            DataContext = this;
        }

        public void UpdateColorDescription()
        {
            InkColor = Brushes.Transparent;
            Header = "Color " + System.Convert.ToString(ColorIndex + 1);

            int globalInkSupplyNo = ColorIndex * _PrintSystem.InkCylindersPerColor; // first cylinder
            InkSupply inkSupply = null;

            string colorName = "";
            SolidColorBrush inkColor = null;
            try
            {
                inkSupply = RxGlobals.InkSupply.List[globalInkSupplyNo];
                if (inkSupply.InkType != null)
                {
                    colorName = inkSupply.InkType.Name;
                    inkColor = new SolidColorBrush(inkSupply.InkType.Color);
                }
            }
            catch
            {
                return;
            }

            bool equal = true;
            /*
            for (int i = 1; i < _PrintSystem.InkCylindersPerColor; i++)
            {
                globalInkSupplyNo = i + ColorIndex * _PrintSystem.InkCylindersPerColor;
                try
                {
                    inkSupply = RxGlobals.InkSupply.List[globalInkSupplyNo];
                    if (inkSupply.InkType != null)
                    {
                        if (!inkSupply.InkType.Name.Equals(colorName))
                        {
                            equal = false;
                            break;
                        }
                    }
                    else
                    {
                        equal = false;
                        break;
                    }
                }
                catch
                {
                    equal = false;
                    break;
                }
            }
            */
            if (equal)
            {
                Header = colorName;
                InkColor = inkColor;
            }
        }
    }
}
