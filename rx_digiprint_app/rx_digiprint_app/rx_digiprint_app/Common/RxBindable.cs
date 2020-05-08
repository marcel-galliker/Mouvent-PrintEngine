using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace DigiPrint.Common
{
    public class RxBindable : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        
         //--- SetProperty -------------------------------------------------------
        public bool SetProperty<type>(ref type property, type value, [CallerMemberName]String propName = null)
        {
            if (property == null || !property.Equals(value))
            {
                property = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propName));
                return true;
            }
            return false;
        }
    }
}
