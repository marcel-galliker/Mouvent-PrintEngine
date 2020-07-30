using System.Windows;

namespace RX_Common.Resources
{
    partial class MvtBasics : ResourceDictionary
    {
        public MvtBasics()
        {
            InitializeComponent();

            RxScreen screen = new RxScreen();
            if (screen.Surface)
            {
                this["MvtFontSizeStd"] = 10.0;
                this["MvtFontSize"] = 16.0;
                this["MvtFontSizeLarge"] = 20.0;
                this["MvtFontSizeXL"] = 40.0;
                this["MvtFontSizeSmall"] = 9.0;
                this["MvtFontSizeXS"] = 8.0;
                this["MvtFontSizeTabHeader"] = 14.0;
                this["MvtFontSizeFooter"] = 18.0;
            }
            else
            {
                this["MvtFontSizeStd"] = 10.0;
                this["MvtFontSize"] = 14.66;
                this["MvtFontSizeLarge"] = 20.0;
                this["MvtFontSizeXL"] = 32.0;
                this["MvtFontSizeSmall"] = 9.0;
                this["MvtFontSizeXS"] = 8.0;
                this["MvtFontSizeTabHeader"] = 14.0;
                this["MvtFontSizeFooter"] = 22.0;
            }
        }
    }
}