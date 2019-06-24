using System;
using System.Windows.Input;

namespace RX_Common
{
    public class RxCommand:ICommand
    {
        private readonly Action<object> _executeHandler;
        private readonly Func<object, bool> _canExecuteHandler;

        //--- creator -----------------------------------------------------
        public RxCommand(Action<object> execute)
        {
            if (execute == null)
            throw new ArgumentNullException("Execute cannot be null");
            _executeHandler = execute;
            _canExecuteHandler = CanAlwaysExecute;
        }

        //--- creator -------------------------------------------------------
        public RxCommand(Action<object> execute, Func<object, bool> canExecute)
        {
            if (execute == null)
            throw new ArgumentNullException("Execute cannot be null");
            _executeHandler = execute;
            _canExecuteHandler = canExecute;
        }

        //--- Event handler -------------------------------------------------
        public event EventHandler CanExecuteChanged
        {
            add { CommandManager.RequerySuggested += value; }
            remove { CommandManager.RequerySuggested -= value; }
        }

        //--- Execute ---------------------------------------------------------
        public void Execute(object parameter)
        {
            _executeHandler(parameter);
        }

        //--- CanExecute -------------------------------------------------------
        public bool CanExecute(object parameter)
        {
            if (_canExecuteHandler == null)
            return true;
            return _canExecuteHandler(parameter);
        }

        //--- CanAlwaysExecute --------------------------------------------
        public bool CanAlwaysExecute(object parameter)
        {
            return true;
        }

    }
}
