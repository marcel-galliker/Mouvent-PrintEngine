using DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace DigiPrint.Pages.PrintSystemView
{
	[XamlCompilation(XamlCompilationOptions.Compile)]
	public partial class HeadView : ContentView
    {
        public HeadView ()
		{
			InitializeComponent ();

            var tgr = new TapGestureRecognizer { NumberOfTapsRequired = 1 };
            tgr.TappedCallback = (sender, args) => 
            {
                foreach (PrintHead head in AppGlobals.PrintHeads)
                {
                    head.IsSelected = (head==BindingContext);
                }
                if (AppGlobals.ActiveHeadChanged!=null) AppGlobals.ActiveHeadChanged();
            };

            MainGrid.GestureRecognizers.Add(tgr);
        }

        //--- BindingContextChanged -----------------------------
        private new void BindingContextChanged(object sender, EventArgs e)
        {
            PrintHead head=(BindingContext as PrintHead);
            if (head!=null) head.PropertyChanged += Head_PropertyChanged;
        }

        //--- Head_PropertyChanged ---------------------------
        private void Head_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName.Equals("IsSelected"))
            {
                MainGrid.BackgroundColor = (BindingContext as PrintHead).IsSelected ? Color.LightBlue : Color.WhiteSmoke;
            }
        }
    }
}