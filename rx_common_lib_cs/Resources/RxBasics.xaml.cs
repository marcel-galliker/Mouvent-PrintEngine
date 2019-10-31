using System.Windows;

namespace RX_Common.Resources
{
    partial class   RxBasics : ResourceDictionary
    {
        public RxBasics()
        {
            InitializeComponent();

            RxScreen screen=new RxScreen();
            if (screen.Surface)
            {
                this["FontSizeXS"]    = 10.0;   
                this["FontSize"]      = 20.0;   
                this["FontSizeLarge"] = 25.0;   
                this["FontSizeXL"]    = 40.0;   
            }
            else
            {
                this["FontSizeXS"]    = 8.0;   
                this["FontSize"]      = 14.66;            
                this["FontSizeLarge"] = 20.0;   
                this["FontSizeXL"]    = 32.0;   
            }
        }
    }
}
