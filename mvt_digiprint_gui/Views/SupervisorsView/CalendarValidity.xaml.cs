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
using RX_DigiPrint.Helpers;

namespace RX_DigiPrint.Views.SupervisorsView
{
    /// <summary>
    /// Class managing the view for Validity date changes
    /// </summary>
    public partial class CalendarValidity : Window
    {
        private MvtUserLevelManager ulm;
        private String name;

        public CalendarValidity(MvtUserLevelManager ulm, String name)
        {
            InitializeComponent();

            this.ulm = ulm;
            this.name = name;
        }

        private void ValidateDate_Click(object sender, RoutedEventArgs e)
        {
            /* Validity is changed if a date was chosen */
            if (!String.IsNullOrWhiteSpace(name) && NewValidity.SelectedDate != null)
            {
                ulm.ChangeSupervisorValidity(name, (DateTime)NewValidity.SelectedDate);
                Close();
            }
        }

        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }
    }
}
