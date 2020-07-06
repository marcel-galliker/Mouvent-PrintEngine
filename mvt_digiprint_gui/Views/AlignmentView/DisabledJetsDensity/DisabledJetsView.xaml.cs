using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Linq;
using System.Windows.Controls;

namespace RX_DigiPrint.Views.AlignmentView.DisabledJets
{
    /// <summary>
    /// Interaction logic for DisabledJetsView.xaml
    /// </summary>
    public partial class DisabledJetsView : UserControl
    {
        private Models.DisabledJets _DisabledJets = RxGlobals.DisabledJets;

        public DisabledJetsView()
        {
            InitializeComponent();
            DataContext = _DisabledJets;
        }

        private void AddJet_Clicked(object sender, System.Windows.RoutedEventArgs e)
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

        private void LoadFile_Clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            var currentList = _DisabledJets.JetList.ToList();

            TcpIp.SDisabledJetsMsg msg = new TcpIp.SDisabledJetsMsg();
            msg.head = _DisabledJets.GetHeadNumber();
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_GET_DISABLED_JETS, ref msg);
        }

        static int _Time = Environment.TickCount;
        private void AddJetPopup_ButtonClicked(object sender, System.Windows.RoutedEventArgs e)
        {
            //--- filter out double clicks ---
            if (Environment.TickCount - _Time < 50) return;
            _Time = Environment.TickCount;

            Button button = sender as Button;

            if (button == null) return;
            if (button.CommandParameter == null)
            {
                if (_DisabledJets.AddJet_HandleInput(button.Content.ToString()))
                {
                    AddJetPopup.IsOpen = false;
                }
            }
            else
            {
                if (_DisabledJets.AddJet_HandleInput(button.CommandParameter.ToString()))
                {
                    AddJetPopup.IsOpen = false;
                }
            }
        }

        private void RemoveJet_Clicked(object sender, System.Windows.RoutedEventArgs e)
        {
            MvtButton button = sender as MvtButton;
            int jetNumber = System.Convert.ToInt32(button.Tag);
            _DisabledJets.RemoveJet(jetNumber);
        }

        public void Save_Clicked()
        {
            _DisabledJets.Save();
        }

    }
}
