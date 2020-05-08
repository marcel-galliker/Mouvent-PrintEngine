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
using RxWfTool.Models;
using RX_GUI.Models.Enums;

namespace RxWfTool.Views
{
    /// <summary>
    /// Interaction logic for InkView.xaml
    /// </summary>
    public partial class InkView : UserControl
    {
        private Ink _Ink;
        public Ink Ink
        {
            get { return _Ink; }
            set { _Ink = value; }
        }

        public InkView()
        {
            InitializeComponent();
            DataContext = _Ink;
        }
    }
}
