using System.Windows.Controls;
using System.Windows.Media;

namespace RX_DigiPrint.Views.UserControls
{
    public partial class PrintBar : UserControl
    {
        private double _HeadHeight = 80;

        //--- Constructor -------------------------------
        public PrintBar()
        {
            InitializeComponent();
            DataContext = this;
        }

        //--- Property HeadHeight ---------------------------------------
        public double HeadHeight
        {
            get { return _HeadHeight; }
        }
        
        //--- Property Color ---------------------------------------
        private Brush _Color;
        public Brush Color
        {
            get { return _Color; }
            set {  _Color = value; }
        }

        //--- Property Angle ---------------------------------------
        private double _Angle;
        public double Angle
        {
            get { return _Angle; }
            set { _Angle = value; }
        }      
    }
}
