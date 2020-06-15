using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using RX_DigiPrint.Views.PrintSystemExtended.JetCompensation;
using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.PrintSystemExtendedView
{
    /// <summary>
    /// Interaction logic for ColorView.xaml
    /// </summary>
    public partial class DetailsView : UserControl, INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged(string name)
        {
            PropertyChangedEventHandler handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(name));
            }
        }

        private InkCylinderView InkCylinderDetailView;
        private PrintHeadDetailsView PrintHeadDetailView;
        private JetCompensationView JetCompensationView;
        
        private PrintSystemExtendedView ParentView = null;

        private bool _SuperUser;
        public bool SuperUser
        {
            get { return _SuperUser; }
            set { _SuperUser = value; OnPropertyChanged("SuperUser"); }
        }

        public DetailsView(PrintSystemExtendedView parent)
        {
            InitializeComponent();

            ParentView = parent;

            InkCylinderDetailView = new InkCylinderView();
            PrintHeadDetailView = new PrintHeadDetailsView();
            JetCompensationView = new PrintSystemExtended.JetCompensation.JetCompensationView();

            InkCylinderGrid.Children.Add(InkCylinderDetailView);
            PrintHeadsDetailGrid.Children.Add(PrintHeadDetailView);
            JetCompensationGrid.Children.Add(JetCompensationView);

            RB_JetCompensation.DataContext = this;

            SuperUser = false;
        }

        private void InkSupplyPropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            ParentView.InkCylinderInkTypeChanged();
        }

        public void SetContext(int inkCylinderIndex)
        {
            InkSupply inkSupply = null;
            try
            {
                inkSupply = RxGlobals.InkSupply.List[inkCylinderIndex];
                inkSupply.PropertyChanged += InkSupplyPropertyChanged;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                return;
            }

            InkCylinderDetailView.DataContext = inkSupply;
            InkCylinderDetailView.SetUser();

            PrintHeadDetailView.SetInkCylinderIndex(inkCylinderIndex);
            PrintHeadDetailView.SetUser();
            JetCompensationView.SetInkCylinderIndex(inkCylinderIndex);
        }

        private void RadioButton_Checked(object sender, RoutedEventArgs e)
        {
 
        }

        public void SetUser()
        {
            PrintHeadDetailView.SetUser();
            InkCylinderDetailView.SetUser();

            SuperUser = RxGlobals.User.UserType >= EUserType.usr_service ? true : false;
        }
    }
}
