using System;
using System.Collections.Generic;
using System.Linq;
using ImageCircle.Forms.Plugin.iOS;

using Foundation;
using UIKit;

namespace DigiPrint.iOS
{
    public class Application
    {
        // This is the main entry point of the application.
        static void Main(string[] args)
        {
            // if you want to use a different Application Delegate class from "AppDelegate"
            // you can specify it here.
            ImageCircleRenderer.Init();
            UIApplication.Main(args, null, "AppDelegate");
        }
    }
}
