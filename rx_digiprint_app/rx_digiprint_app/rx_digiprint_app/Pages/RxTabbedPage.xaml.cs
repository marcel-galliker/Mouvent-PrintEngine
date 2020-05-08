using DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace DigiPrint.Pages
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
    public partial class RxTabbedPage : TabbedPage
    {
        public Action<int,string> SetIcon;

        private int _EventPagePos;
        public RxTabbedPage()
        {
            InitializeComponent();
            BindingContext = AppGlobals.Printer;
            AppGlobals.Printer.PropertyChanged += Printer_PropertyChanged;

            _EventPagePos = Children.IndexOf(EventTab);
            PS_View.MainView("PrintSystem", new DigiPrint.Pages.PrintSystemView.PrintSystemView(), true);
        }

        private void Printer_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("EventIcon"))
            {
                if (SetIcon!=null) SetIcon(_EventPagePos, AppGlobals.Printer.EventIcon);       
            }
        }
    }
}