using RX_Common;
using RX_DigiPrint.Models.Enums;
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
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.PrintQueueView
{
    /// <summary>
    /// Interaction logic for PageNumber.xaml
    /// </summary>
    public partial class PageNumberView : UserControl
    {
        public PageNumberView()
        {
            InitializeComponent();
            DataContext = this;
        }

    }
}
