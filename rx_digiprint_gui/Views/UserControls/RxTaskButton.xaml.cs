using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.UserControls
{
    public partial class RxTaskButton : Button
    {
        //--- constructor ---------------------------------------
        public RxTaskButton()
        {
            InitializeComponent();
            DataContext = this;
        }

        //--- Property Image ---------------------------------------
        public string Image
        {
            set { MyImage.Source = new BitmapImage(new Uri("..\\..\\Resources\\Bitmaps\\"+value, UriKind.RelativeOrAbsolute)); }
        }

        //--- Property Text ---------------------------------------
        public string Text
        {
            get { return MyText.Text;}
            set { MyText.Text=value; }
        }

        //--- Property Process ---------------------------------------
        private Process _Process;
        public Process Process
        {
            get { return _Process; }
            set { _Process = value; }
        }

        //--- Property used ---------------------------------------
        private int _used;
        public int used
        {
            get { return _used; }
            set { _used = value; }
        }

        //--- Property Order ---------------------------------------
        private int _Order;
        public int Order
        {
            get { return _Order; }
            set { _Order = value; }
        }   
        
    }
}
