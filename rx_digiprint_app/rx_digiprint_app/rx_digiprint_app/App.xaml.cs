using DigiPrint.Models;
using DigiPrint.Pages;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Xamarin.Forms;

namespace DigiPrint
{
    public partial class App : Application 
    {
        private DigiPrint.Pages.MainPage _MainPage;

        //--- constructor -----------------------------------------------
        public App()
        {
            InitializeComponent();
            MainPage = new MainPage();
            _MainPage = MainPage as DigiPrint.Pages.MainPage;
        }

        //--- OnStart ---------------------------------------------------
        protected override void OnStart()
        {
            // Handle when your app starts
        }

        //--- OnSleep ----------------------------------------------------
        protected override void OnSleep()
        {
            _MainPage.OnSleep();
        }

        //--- OnResume ---------------------------------------------------
        protected override void OnResume()
        {
            _MainPage.OnResume();
        }

    }
}
