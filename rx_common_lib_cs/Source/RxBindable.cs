using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Threading;

namespace RX_Common
{
    /*
    public interface IBindableBase : INotifyPropertyChanged
    {
        internal bool SetProperty<type>(ref type property, type val, [CallerMemberName]String propName = null);
    }
    */

    public class RxBindable : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        
        //--- Property Dispatcher ---------------------------------------
        public static Dispatcher _Dispatcher;
        public static Dispatcher Dispatcher
        {
            set { _Dispatcher= value; }
            get { return _Dispatcher; }
        }

        //--- Invoke -------------------------
        public static void Invoke(Action action)
        {
            if (action==null) return;
            if (RxBindable._Dispatcher != null)
            {
                try
                {
                    RxBindable._Dispatcher.Invoke(action);
                }
                catch (Exception e)
                {
                    Console.WriteLine(e.Message);
                }
            }
            else
            {
                try { action(); }
                catch { };
            }
        }

        //--- InvokeDelayed -------------------------
        public static void InvokeDelayed(int ms, Action action)
        {
            if (action==null) return;
            Task.Run(()=>
            {
                Thread.Sleep(ms);
                RxBindable.Invoke(action);
            });
        }

        /// <summary>
        /// GetProperty Interlocked on double to ensure atomic access
        /// </summary>
        /// <param name="property">ref to the double property to read</param>
        /// <returns>value of the property</returns>
        public double GetProperty(ref double property)
        {
            return Interlocked.CompareExchange(ref property, 0, 0);
        }

        /// <summary>
        /// SetProperty Interlocked (for double) and call PropertyChanged
        /// </summary>
        /// <param name="property">ref to the property (double) </param>
        /// <param name="val">new value</param>
        /// <param name="propName">property name to pass to PropertyChanged (if exists)</param>
        /// <returns>True if property changed</returns>
        public bool SetProperty(ref double property, double val, [CallerMemberName] String propName = null)
        {
            if (Interlocked.CompareExchange(ref property, val, val) != val)
            {
                Interlocked.Exchange(ref property, val);
                if (PropertyChanged != null)
                    Invoke(() => PropertyChanged(this, new PropertyChangedEventArgs(propName)));
                return true;
            }
            return false;
        }

        //--- SetProperty -------------------------------------------------------
        public bool SetProperty<type>(ref type property, type val, [CallerMemberName]String propName = null)
        {
            if (property == null || !property.Equals(val))
            {
                property = val;
                if (PropertyChanged != null)
                    Invoke(()=>PropertyChanged(this, new PropertyChangedEventArgs(propName)));
                return true;
            }
            return false;
        }

        //--- OnPropertyChanged --------------------------------------------------------------
        public void OnPropertyChanged([CallerMemberName]String info = null)
        {
            if (PropertyChanged != null)
                Invoke(()=>PropertyChanged(this, new PropertyChangedEventArgs(info)));
        }

        //--- AllPropertiesChanged --------------------------------------
        public void AllPropertiesChanged(object obj)
        {
            if (PropertyChanged != null)
            {
                Invoke(()=>
                    {
                        foreach(var prop in obj.GetType().GetProperties())
                        {
                            PropertyChanged(this, new PropertyChangedEventArgs(prop.Name));
                        }   
                    }
                  );
            }
        }
    }
}
