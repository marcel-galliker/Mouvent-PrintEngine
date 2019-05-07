using Xamarin.Forms;

namespace DigiPrint.Common
{
    class RxError
    {
        static private bool _Error = false;
        public static void Error(Page page, string message)
        {
            if (!_Error)
            {
                _Error=true;
                Rx.Invoke(() => page.DisplayAlert("ERROR", message, "OK"));
                _Error=false;
            }
        }
    }
}
