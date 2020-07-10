using RX_Common;
using RX_DigiPrint.Views.Density;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{

    public class JetCompensationDensityViewModel : RxBindable
    {
        private Models.DisabledJets _DisabledJets = RxGlobals.DisabledJets;
        private Models.Density _Density = RxGlobals.Density;

        private bool _Changed;
        public bool Changed
        {
            get
            {
                return _DisabledJets.Changed || _Density.Changed;
            }
            set
            {
                SetProperty(ref _Changed, value);
            }
        }

        private string _HeadName;
        public string HeadName
        {
            get { return _HeadName; }
            set { SetProperty(ref _HeadName, value); }
        }

        public JetCompensationDensityViewModel(int globalHeadNumber)
        {
            HeadName = "";
            if (globalHeadNumber < RxGlobals.HeadStat.List.Count)
            {
                if (!string.IsNullOrEmpty(RxGlobals.HeadStat.List[globalHeadNumber].Name))
                {
                    HeadName = "Print Head: " + RxGlobals.HeadStat.List[globalHeadNumber].Name;
                }
            }
            if (string.IsNullOrEmpty(HeadName))
            {
                HeadName = "Print Head: ?";
            }

            _DisabledJets.PropertyChanged +=
                new PropertyChangedEventHandler(JetCompensationDensityPropertyChanged);
            _Density.PropertyChanged += new PropertyChangedEventHandler(JetCompensationDensityPropertyChanged);
        }

        private void JetCompensationDensityPropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (!(e.PropertyName == "Changed"))
            {
                return;
            }

            Changed = _DisabledJets.Changed || _Density.Changed;
        }

        }
    }
