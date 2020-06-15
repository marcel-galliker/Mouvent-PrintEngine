using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint
{
    public class JetCompensationViewModel : INotifyPropertyChanged
    {
        public const int MaxDisabledJets = 3;

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

        private ObservableCollection<JetCompensation> _DisabledJets;
        public ObservableCollection<JetCompensation> DisabledJets
        {
            get { return _DisabledJets; }
            set
            {
                _DisabledJets = value; OnPropertyChanged("DisabledJets");
                CanAddJetNumber = (_DisabledJets.Count < MaxDisabledJets);
            }
        }

        
        private int _headNumber;
        public int HeadNumber
        {
            get { return _headNumber; }
            set { _headNumber = value; OnPropertyChanged("HeadNumber"); }
        }

        private string _clusterName;
        public string ClusterName
        {
            get { return _clusterName; }
            set { _clusterName = value; OnPropertyChanged("ClusterName"); }
        }

        private string _headName;
        public string HeadName
        {
            get { return _headName; }
            set { _headName = value; OnPropertyChanged("HeadName"); }
        }

        private int FirstClusterNumber;
        private int LastClusterNumber;
        private int ClusterNumber;
        private int InkCylinderIndex;

        private int FirstPrintHeadNumber;
        private int LastPrintHeadNumber;
        
        private bool _SingleCluster;
        public bool SingleCluster
        {
            get { return _SingleCluster; }
            set { _SingleCluster = value; OnPropertyChanged("SingleCluster"); }
        }

        private bool _isFirstCluster;
        public bool IsFirstCluster
        {
            get { return _isFirstCluster; }
            set { _isFirstCluster = value; OnPropertyChanged("IsFirstCluster"); }
        }

        private bool _isLastCluster;
        public bool IsLastCluster
        {
            get { return _isLastCluster; }
            set { _isLastCluster = value; OnPropertyChanged("IsLastCluster"); }
        }

        private bool _isFirstPrintHead;
        public bool IsFirstPrintHead
        {
            get { return _isFirstPrintHead; }
            set { _isFirstPrintHead = value; OnPropertyChanged("IsFirstPrintHead"); }
        }

        private bool _isLastPrintHead;
        public bool IsLastPrintHead
        {
            get { return _isLastPrintHead; }
            set { _isLastPrintHead = value; OnPropertyChanged("IsLastPrintHead"); }
        }

        private bool _allSelectedChanging;
        private bool? _allSelected;

        private bool _canAddJetNumber;
        public bool CanAddJetNumber
        {
            get { return _canAddJetNumber; }
            set { _canAddJetNumber = value; OnPropertyChanged("CanAddJetNumber"); }
        }

        public JetCompensationViewModel()
        {
            HeadNumber = 0;
            SetInkCylinderIndex(0);
        }

        private void AllSelectedChanged()
        {
            // Has this change been caused by some other change, the return
            if (_allSelectedChanging) return;

            try
            {
                _allSelectedChanging = true;

                if (AllSelected == true)
                {
                    foreach (var jet in DisabledJets)
                    {
                        jet.IsSelected = true;
                    }
                }
                else if (AllSelected == false)
                {
                    foreach (var jet in DisabledJets)
                    {
                        jet.IsSelected = false;
                    }
                }
            }
            finally
            {
                _allSelectedChanging = false;
            }
        }

        private void RecheckAllSelected()
        {
            // Has this change been caused by some other change, then return
            if (_allSelectedChanging) return;

            try
            {
                _allSelectedChanging = true;

                if (DisabledJets.All(x => x.IsSelected))
                {
                    AllSelected = true;
                }
                else if (DisabledJets.All(x => !x.IsSelected))
                {
                    AllSelected = false; 
                }
                else
                {
                    AllSelected = null;
                }
            }
            finally
            {
                _allSelectedChanging = false;
            }
        }

        public bool? AllSelected
        {
            get { return _allSelected; }
            set
            {
                if (value == _allSelected) return;
                _allSelected = value;

                // Set all other CheckBoxes
                AllSelectedChanged();
                OnPropertyChanged("AllSelected");
            }
        }

        public void SetInkCylinderIndex(int inkCylinderIndex)
        {
            // find first print head of this ink cylinder:
            int firstHead  = inkCylinderIndex * _PrintSystem.HeadsPerInkCylinder;
            InkCylinderIndex = inkCylinderIndex;

            FirstClusterNumber = 0;
            LastClusterNumber  = 0;
            int inkCylinderCnt = _PrintSystem.ColorCnt * _PrintSystem.InkCylindersPerColor;
            {
                FirstClusterNumber = _PrintSystem.Cluster(InkCylinderIndex * _PrintSystem.HeadsPerInkCylinder);
                LastClusterNumber  = _PrintSystem.Cluster(InkCylinderIndex * _PrintSystem.HeadsPerInkCylinder + _PrintSystem.HeadsPerInkCylinder - 1);
            }
            
            SingleCluster = (FirstClusterNumber == LastClusterNumber);

            SetClusterNumber(FirstClusterNumber);
        }

        private void SetClusterNumber(int clusterNumber)
        {
            ClusterNumber = clusterNumber;

            int firstHeadOfCluster = TcpIp.HEAD_CNT * ClusterNumber;
            FirstPrintHeadNumber = firstHeadOfCluster;
            LastPrintHeadNumber = firstHeadOfCluster + TcpIp.HEAD_CNT - 1;
            SetHeadNumber(FirstPrintHeadNumber);
        }
        
        private void DisabledJetListEntryOnPropertyChanged(object sender, PropertyChangedEventArgs args)
        {
            // Only re-check if the IsChecked property changed
            if (args.PropertyName == "IsSelected")
            {
                RecheckAllSelected();
            }
        }

        private void CleanUpDisabledJets()
        {
            if(DisabledJets != null)
            {
                foreach (var elem in DisabledJets)
                {
                    elem.PropertyChanged -= DisabledJetListEntryOnPropertyChanged;
                }
            }
        }

        private void SetHeadNumber(int headNumber)
        {
            HeadNumber = headNumber;
            try
            {
                CleanUpDisabledJets();
                DisabledJets = RxGlobals.HeadStat.List[HeadNumber].DisabledJets;
                foreach (var jet in DisabledJets)
                {
                    jet.PropertyChanged += DisabledJetListEntryOnPropertyChanged;
                }

                ClusterNumber = HeadNumber / 4;
                ClusterName = "Cluster " + (ClusterNumber + 1).ToString();
                if (RxGlobals.HeadStat.List[HeadNumber].Name != null)
                {
                    HeadName = "Print Head " + RxGlobals.HeadStat.List[HeadNumber].Name;
                }
                else
                {
                    HeadName = "Print Head " + (HeadNumber + 1).ToString();
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                DisabledJets = null;
            }

            UpdateVisibility();
        }

        private void UpdateVisibility()
        {
            IsFirstCluster = ClusterNumber == FirstClusterNumber;
            IsLastCluster = ClusterNumber == LastClusterNumber;
            IsFirstPrintHead = HeadNumber == FirstPrintHeadNumber;
            IsLastPrintHead = HeadNumber == LastPrintHeadNumber;
        }

        public void AddJet(int jetNumber)
        {
            try
            {
                bool alreadyListed = RxGlobals.HeadStat.List[HeadNumber].DisabledJets.Any(x => x.JetNumber == jetNumber);
                if (!alreadyListed)
                {
                    var newJetCompensationDefinition = new JetCompensation { JetNumber = jetNumber, IsActivated = true };
                    newJetCompensationDefinition.PropertyChanged += DisabledJetListEntryOnPropertyChanged;
                    RxGlobals.HeadStat.List[HeadNumber].DisabledJets.Add(newJetCompensationDefinition);
                    RxGlobals.HeadStat.List[HeadNumber].DisabledJets = new ObservableCollection<JetCompensation>(
                    RxGlobals.HeadStat.List[HeadNumber].DisabledJets.OrderBy(x => x.JetNumber).ToList());
                    DisabledJets = RxGlobals.HeadStat.List[HeadNumber].DisabledJets;

                    _PrintSystem.Changed = true;
                }
                else
                {
                    MvtMessageBox.Information("", "Jet Nr. " + jetNumber + " already listed!");
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
            var itemToRemove = RxGlobals.HeadStat.List[HeadNumber].DisabledJets.Single(x => x.JetNumber == jetNumber);
            if (itemToRemove != null)
            {
                removed = RxGlobals.HeadStat.List[HeadNumber].DisabledJets.Remove(itemToRemove);
            }
            if (removed)
            {
                RxGlobals.HeadStat.List[HeadNumber].DisabledJets = new ObservableCollection<JetCompensation>(
                      RxGlobals.HeadStat.List[HeadNumber].DisabledJets.OrderBy(x => x.JetNumber).ToList());
                DisabledJets = RxGlobals.HeadStat.List[HeadNumber].DisabledJets;

                _PrintSystem.Changed = true;
            }
        }

        public void RemoveSelectedJets()
        {
            bool removed = false;
            for (int i = RxGlobals.HeadStat.List[HeadNumber].DisabledJets.Count - 1; i >= 0; i--)
            {
                try
                {
                    if (RxGlobals.HeadStat.List[HeadNumber].DisabledJets[i].IsSelected)
                    {
                        RxGlobals.HeadStat.List[HeadNumber].DisabledJets.RemoveAt(i);
                        removed = true;
                    }
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.Message);
                }
            }
            if (removed)
            {
                RxGlobals.HeadStat.List[HeadNumber].DisabledJets = new ObservableCollection<JetCompensation>(
                                      RxGlobals.HeadStat.List[HeadNumber].DisabledJets.OrderBy(x => x.JetNumber).ToList());
                DisabledJets = RxGlobals.HeadStat.List[HeadNumber].DisabledJets;

                _PrintSystem.Changed = true;
            }
        }

        public void ToggleActivated(int jetNumber)
        {
            var item = RxGlobals.HeadStat.List[HeadNumber].DisabledJets.Single(x => x.JetNumber == jetNumber);
            if (item != null)
            {
                item.IsActivated = !item.IsActivated;
                _PrintSystem.Changed = true;

                RxGlobals.HeadStat.List[HeadNumber].DisabledJets = new ObservableCollection<JetCompensation>(
                          RxGlobals.HeadStat.List[HeadNumber].DisabledJets.OrderBy(x => x.JetNumber).ToList());
                DisabledJets = RxGlobals.HeadStat.List[HeadNumber].DisabledJets;
            }
        }

        public void ActivatedSelectedJets(bool activate)
        {
            bool changed = false;
            for (int i = 0; i < RxGlobals.HeadStat.List[HeadNumber].DisabledJets.Count; i++)
            {
                if (RxGlobals.HeadStat.List[HeadNumber].DisabledJets[i].IsSelected)
                {
                    if (RxGlobals.HeadStat.List[HeadNumber].DisabledJets[i].IsActivated != activate)
                    {
                        changed = true;
                        RxGlobals.HeadStat.List[HeadNumber].DisabledJets[i].IsActivated = activate;
                    }
                }
            }
            if (changed)
            {
                RxGlobals.HeadStat.List[HeadNumber].DisabledJets = new ObservableCollection<JetCompensation>(
                                     RxGlobals.HeadStat.List[HeadNumber].DisabledJets.OrderBy(x => x.JetNumber).ToList());
                DisabledJets = RxGlobals.HeadStat.List[HeadNumber].DisabledJets;
                _PrintSystem.Changed = true;
            }
        }

        public void SelectAllJets(bool select)
        {
            RxGlobals.HeadStat.List[HeadNumber].DisabledJets.ToList().ConvertAll(x => x.IsSelected = select);
             
                RxGlobals.HeadStat.List[HeadNumber].DisabledJets = new ObservableCollection<JetCompensation>(
                       RxGlobals.HeadStat.List[HeadNumber].DisabledJets.OrderBy(x => x.JetNumber).ToList());
                DisabledJets = RxGlobals.HeadStat.List[HeadNumber].DisabledJets;
        }

        public void NextPrintHead()
        {
            SetHeadNumber(HeadNumber + 1);
        }

        public void PreviousPrintHead()
        {
            SetHeadNumber(HeadNumber - 1);
        }

        public void NextCluster()
        {
            SetClusterNumber(ClusterNumber + 1);
        }

        public void PreviousCluster()
        {
            SetClusterNumber(ClusterNumber - 1);
        }
    }
}
