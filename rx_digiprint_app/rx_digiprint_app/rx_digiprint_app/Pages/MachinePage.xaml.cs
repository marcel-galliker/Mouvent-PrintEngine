using DigiPrint.Models;
using DigiPrint.Pages.MachineViews;
using RX_DigiPrint.Services;

using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace DigiPrint.Pages
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class MachinePage : ContentPage
    {
        private RxBtDef.EBTState _State=RxBtDef.EBTState.state_undef;
        private bool             _FixedView=false;

        //--- constructor ------------------------------------
        public MachinePage()
        {
            InitializeComponent();
            BindingContext = AppGlobals.Printer;
            AppGlobals.Printer.PropertyChanged += Printer_PropertyChanged;
            UpdateMachineState();
        }

        //--- Printer_PropertyChanged ----------------------
        private void Printer_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("State")) Device.BeginInvokeOnMainThread(() => UpdateMachineState());
        }

        //--- Property MyProperty ---------------------------------------
        public void MainView(string title, ContentView view, bool fix)
        {
            _Title.Text = title;
            _Content.Children[0] = view;
            _FixedView = fix;
        }

        //--- UpdateMachineState -------------------------------
        private void UpdateMachineState()
        {
            if (!_FixedView)
            {
                RxBtDef.EBTState state = AppGlobals.Printer.State;
                if (state!=_State)
                {
                    switch (state)
                    {
                    case RxBtDef.EBTState.state_undef:      break;
                    case RxBtDef.EBTState.state_stop:       MainView("Stepper", new DigiPrint.Pages.StepperView.StepperView(), false); break;
                    case RxBtDef.EBTState.state_offline:    MainView("TEST", new TestView(), false);  break;
                    case RxBtDef.EBTState.state_webin:      MainView("WEB-IN", new WebInView(), false); break;
                    case RxBtDef.EBTState.state_prouction:  break;
                    }
                    _State = state;
                }
            }
        }
    }
}