using RX_Common;
using RX_DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.ComponentModel;
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

namespace RX_DigiPrint.Views.PrintSystemExtended.JetCompensation
{
    /// <summary>
    /// Interaction logic for JetCompensationView.xaml
    /// </summary>
    public partial class JetCompensationView : UserControl, INotifyPropertyChanged
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

        public JetCompensationViewModel JetCompensationViewModel;

        private string _Result;
        public string Result
        {
            get { return _Result; }
            set { _Result = value; OnPropertyChanged("Result"); }
        }
        
        public JetCompensationView()
        {
            InitializeComponent();

            JetCompensationViewModel = new JetCompensationViewModel();
            Result = "";
            AddJetPopup.DataContext = this;
           
            this.DataContext = JetCompensationViewModel;
        }

        private void UserControl_DataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            JetCompensationViewModel = DataContext as JetCompensationViewModel;
            JetCompensationNewDataGrid.ItemsSource = JetCompensationViewModel.DisabledJets;
        }

        public void SetInkCylinderIndex(int inkCylinderIndex)
        {
            JetCompensationViewModel = DataContext as JetCompensationViewModel;

            JetCompensationViewModel.SetInkCylinderIndex(inkCylinderIndex);

            this.DataContext = JetCompensationViewModel;
            JetCompensationNewDataGrid.ItemsSource = JetCompensationViewModel.DisabledJets;
        }

        private void PreviousCluster_Clicked(object sender, RoutedEventArgs e)
        {
            JetCompensationViewModel.PreviousCluster();
            JetCompensationNewDataGrid.ItemsSource = JetCompensationViewModel.DisabledJets;
        }

        private void NextCluster_Clicked(object sender, RoutedEventArgs e)
        {
            JetCompensationViewModel.NextCluster();
            
            JetCompensationNewDataGrid.ItemsSource = JetCompensationViewModel.DisabledJets;
        }

        private void PreviousPrintHead_Clicked(object sender, RoutedEventArgs e)
        {
            JetCompensationViewModel.PreviousPrintHead();
            
            JetCompensationNewDataGrid.ItemsSource = JetCompensationViewModel.DisabledJets;
        }

        private void NextPrintHead_Clicked(object sender, RoutedEventArgs e)
        {
            JetCompensationViewModel.NextPrintHead();
            this.DataContext = JetCompensationViewModel;
            
            JetCompensationNewDataGrid.ItemsSource = JetCompensationViewModel.DisabledJets;
        }

        private void AddJet_Clicked(object sender, RoutedEventArgs e)
        {
            if (AddJetPopup.IsOpen)
            {
                AddJetPopup.IsOpen = false;
            }
            else
            {
                AddJetPopup.Open(AddJetButton);
            }
        }

        static int _Time = Environment.TickCount;
        private void AddJetPopup_ButtonClicked(object sender, RoutedEventArgs e)
        {
            //--- filter out double clicks ---
            if (Environment.TickCount - _Time < 50) return;
            _Time = Environment.TickCount;

            Button button = sender as Button;

            if (button == null) return;
            if (button.CommandParameter == null)
            {
                _handle_key(button.Content.ToString());
            }
            else
            {
                _handle_key(button.CommandParameter.ToString());
            }
        }

        private void _handle_key(string key)
        {
            if (key.Equals("Escape"))
            {
                Result = "";
                AddJetPopup.IsOpen = false;
            }
            else if (key.Equals("Return"))
            {
                int jetNumber = System.Convert.ToInt32(Result);
                Result = "";

                JetCompensationViewModel.AddJet(jetNumber);
                
                JetCompensationNewDataGrid.ItemsSource = JetCompensationViewModel.DisabledJets;
                
                AddJetPopup.IsOpen = false;
            }
            else if (key.Equals("Back"))
            {
                if (Result.Length > 0) Result = Result.Remove(Result.Length - 1);
            }
            else if (Result.Equals("0")) Result = key;
            else Result += key;
        }


        private void RemoveSelectedJets_Clicked(object sender, RoutedEventArgs e)
        {
            JetCompensationViewModel.RemoveSelectedJets();

            JetCompensationNewDataGrid.ItemsSource = JetCompensationViewModel.DisabledJets;
        }

        private void ActivateSelected_Clicked(object sender, RoutedEventArgs e)
        {
            JetCompensationViewModel.ActivatedSelectedJets(true);
            JetCompensationNewDataGrid.ItemsSource = JetCompensationViewModel.DisabledJets;
        }

        private void DeactivateSelected_Clicked(object sender, RoutedEventArgs e)
        {
            JetCompensationViewModel.ActivatedSelectedJets(false);
            
            JetCompensationNewDataGrid.ItemsSource = JetCompensationViewModel.DisabledJets;
            
        }

        private void CheckBox_Checked(object sender, RoutedEventArgs e)
        {
            
        }

        private void CheckBox_Unchecked(object sender, RoutedEventArgs e)
        {
            
        }

        private void SelectSingle_Click(object sender, RoutedEventArgs e)
        {
            
        }

        private void ActivateToggle_Clicked(object sender, RoutedEventArgs e)
        {
            MvtButton button = sender as MvtButton;
            int jetNumber = System.Convert.ToInt32(button.Tag);
            JetCompensationViewModel.ToggleActivated(jetNumber);
            
            JetCompensationNewDataGrid.ItemsSource = JetCompensationViewModel.DisabledJets;
        }

        private void RemoveSingleJet_Clicked(object sender, RoutedEventArgs e)
        {
            MvtButton button = sender as MvtButton;
            int jetNumber = System.Convert.ToInt32(button.Tag);
            JetCompensationViewModel.RemoveJet(jetNumber);
            
            JetCompensationNewDataGrid.ItemsSource = JetCompensationViewModel.DisabledJets;
        }
    }
}
