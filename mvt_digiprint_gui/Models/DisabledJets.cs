using RX_Common;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Services;
using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Runtime.InteropServices;
using System.Windows.Input;
using System.Windows.Navigation;

namespace RX_DigiPrint.Models
{
    public class DisabledJets : RxBindable
    {
        static private ObservableCollection<JetCompensation> _JetList = new ObservableCollection<JetCompensation>();
        public ObservableCollection<JetCompensation> JetList
        {
            get { return _JetList; }
            set { SetProperty(ref _JetList, value); }
        }

        private int HeadNumber { get; set; }

        private string _AddJetPadResult = "";
        public string AddJetPadResult
        {
            get { return _AddJetPadResult; }
            set { SetProperty(ref _AddJetPadResult , value); }
        }

        private bool _Changed = false;
        public bool Changed
        {
            get { return _Changed; }
            set { SetProperty(ref _Changed, value); }
        }
        
        public DisabledJets()
        {
            
        }

        public void SetHeadNumber(int headNumber)
        {
            HeadNumber = headNumber;
           
            if (HeadNumber >= 0 && HeadNumber < RxGlobals.HeadStat.List.Count)
            {
                HeadStat stat = RxGlobals.HeadStat.List[HeadNumber];
                if (stat != null) SetDisablesJets(stat.DisabledJets);
            }

        }

        public int GetHeadNumber()
        {
            return HeadNumber;
        }

        public void SetDisablesJets(UInt16[] disabledJets)
        {
            var currentList = JetList.ToList();

            _JetList.Clear();
            for (int i = 0; i < disabledJets.Length && i < TcpIp.MAX_DISABLED_JETS; i++)
            {
                if (!(disabledJets[i] <= 0 || disabledJets[i] > 2048 + 128))
                {
                    _JetList.Add(new JetCompensation { JetNumber = disabledJets[i], Index = i + 1 });
                }
            }
            JetList = new ObservableCollection<JetCompensation>(JetList.OrderBy(x => x.JetNumber).ToList());

            bool changed = false;
            if (currentList.Count != JetList.Count)
            {
                changed = true;
            }
            else
            {
                int index = 0;
                foreach (var elem in JetList)
                {
                    if (elem.JetNumber != currentList[index].JetNumber)
                    {
                        changed = true;
                        break;
                    }
                    index++;
                }
            }
            if (changed)
            {
                Changed = true;
            }
        }

        public bool AddJet_HandleInput(string key)
        {
            bool closePopup = false;
            if (key.Equals("Escape"))
            {
                AddJetPadResult = "";
                closePopup = true;
            }
            else if (key.Equals("Return"))
            {
                int jetNumber = System.Convert.ToInt32(AddJetPadResult);
                AddJetPadResult = "";

                AddJet(jetNumber);

                closePopup = true;
            }
            else if (key.Equals("Back"))
            {
                if (AddJetPadResult.Length > 0) AddJetPadResult = AddJetPadResult.Remove(AddJetPadResult.Length - 1);
            }
            else if (AddJetPadResult.Equals("0")) AddJetPadResult = key;
            else AddJetPadResult += key;

            return closePopup;
        }

        private void AddJet(int jetNumber)
        {
            if(jetNumber < 0  || jetNumber > 2048 + 128 - 1) // jet number from 0 to 20175!
            {
                MvtMessageBox.Information("", RX_DigiPrint.Resources.Language.Resources.InvalidJetNr + jetNumber);
                return;
            }

            if (JetList.Count >= TcpIp.MAX_DISABLED_JETS)
            {
                MvtMessageBox.Information("", RX_DigiPrint.Resources.Language.Resources.MaxDisabledJets + TcpIp.MAX_DISABLED_JETS.ToString() + "!");
                return;
            }

            try
            {
                bool alreadyListed = _JetList.Any(x => x.JetNumber == jetNumber);
                if (!alreadyListed)
                {
                    JetList.Add(new JetCompensation() { JetNumber = jetNumber });
                    JetList = new ObservableCollection<JetCompensation>(JetList.OrderBy(x => x.JetNumber).ToList());
                    int index = 1;
                    foreach (var elem in JetList)
                    {
                        elem.Index = index;
                        index++;
                    }
                    Changed = true;
                }
                else
                {
                    MvtMessageBox.Information("", string.Format(RX_DigiPrint.Resources.Language.Resources.JetNumberAlreadyListed, jetNumber));
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
            }
        }

        public void RemoveJet(int jetNumber)
        {
            bool removed = false;
            var itemToRemove = JetList.Single(x => x.JetNumber == jetNumber);
            if (itemToRemove != null)
            {
                removed = JetList.Remove(itemToRemove);
            }
            if (removed)
            {
                JetList = new ObservableCollection<JetCompensation>(JetList.OrderBy(x => x.JetNumber).ToList());
                int index = 1;
                foreach (var elem in JetList)
                {
                    elem.Index = index;
                    index++;
                }
                Changed = true;
            }
        }

        public void Save()
        {
            int i;
            TcpIp.SDisabledJetsMsg msg = new TcpIp.SDisabledJetsMsg();
            msg.head = HeadNumber;
            msg.disabledJets = new UInt16[TcpIp.MAX_DISABLED_JETS];
            for (i = 0; i < TcpIp.MAX_DISABLED_JETS; i++) msg.disabledJets[i] = 0xffff;
            for (i = 0; i < TcpIp.MAX_DISABLED_JETS && i < _JetList.Count(); i++)
            {
                msg.disabledJets[i] = (UInt16)_JetList[i].JetNumber;
            }
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SET_DISABLED_JETS, ref msg);

            Changed = false;
        }
    }
}
