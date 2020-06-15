using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
    public class JetCompensation : INotifyPropertyChanged
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

        public int JetNumber { get; set; }

        private bool _isActivated; // false -> jet is listed but should not be disabled at the moment
        public bool IsActivated
        {
            get
            { return _isActivated; }
            set
            { _isActivated = value; OnPropertyChanged("IsActivated"); }
        } 

        private bool _isSelected;
        public bool IsSelected 
        {
            get { return _isSelected; }
            set { _isSelected = value; OnPropertyChanged("IsSelected"); } 
        }

        // for xam grid only
        public bool Remove
        {
            get { return true; }
        }
    }
}
