using DigiPrint.Models;

using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace DigiPrint.Pages.PrintSystemView
{
    [XamlCompilation(XamlCompilationOptions.Compile)]
	public partial class PrintSystemView : ContentView
	{
        //--- PrintSystemView ------------------------------
        public PrintSystemView ()
		{
			InitializeComponent ();

            AppGlobals.ActiveHeadChanged += _activeHeadChanged;
            AppGlobals.PrintHeads.CollectionChanged += PrintHeads_CollectionChanged;
            PrintHeads_CollectionChanged(null, null);
        }

        //--- PrintHeads_CollectionChanged -------------------------------------
        private void PrintHeads_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            while (HeadList.Children.Count < AppGlobals.PrintHeads.Count)
            {
                int no= HeadList.Children.Count;
                int headCnt = AppGlobals.Printer.ColorCnt * AppGlobals.Printer.HeadsPerColor;
                HeadList.Children.Add(new HeadView());
                if (AppGlobals.Printer.Reverse) HeadList.Children[no].BindingContext = AppGlobals.PrintHeads[headCnt-no-1];
                else                            HeadList.Children[no].BindingContext = AppGlobals.PrintHeads[no];
            }
            while (HeadList.Children.Count > AppGlobals.PrintHeads.Count) HeadList.Children.RemoveAt(AppGlobals.PrintHeads.Count);
        }

        //--- _activeHeadChanged ----------------------------------------
        private void _activeHeadChanged()
        {
            foreach(PrintHead head in AppGlobals.PrintHeads)
            {
                if (head.IsSelected)
                {
                    ActiveHead.BindingContext = head;
                    ActiveHead.IsVisible = true;
                    return;
                }
            }
            ActiveHead.IsVisible = false;
        }

        //--- ScrollView_SizeChanged ---------------------------------
        private void ScrollView_SizeChanged(object sender, System.EventArgs e)
        {
            ScrollView s = sender as ScrollView;
            double gap=4;
            double w=s.Width/8-gap; 
            int i;
            for (i=0; i< HeadList.Children.Count; i++) HeadList.Children[i].WidthRequest=w;
        }
    }
}