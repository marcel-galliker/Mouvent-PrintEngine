using AdvancedForms.Droid.CustomRenderer;
using Android.Content;
using Android.Support.Design.Widget;
using System;
using System.ComponentModel;
using System.Linq;
using Xamarin.Forms;
using Xamarin.Forms.Platform.Android;
using Xamarin.Forms.Platform.Android.AppCompat;

[assembly: ExportRenderer(typeof(DigiPrint.Pages.RxTabbedPage), typeof(RxTabbedPageRenderer))]
namespace AdvancedForms.Droid.CustomRenderer
{
    public class RxTabbedPageRenderer : TabbedPageRenderer 
    {
        public RxTabbedPageRenderer(Context context) : base(context)
        {     
        }
        protected override void OnElementPropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            base.OnElementPropertyChanged(sender, e);
            (Element as DigiPrint.Pages.RxTabbedPage).SetIcon = SetIcon;
        }
        private void SetIcon(int tab, string icon)
        {
            TabLayout layout = (TabLayout)ViewGroup.GetChildAt(1);
            layout.GetTabAt(tab).SetIcon(_IdFromTitle(icon));
        }
        private int _IdFromTitle(string title)
        {
            Type type = ResourceManager.DrawableClass;
            string name = System.IO.Path.GetFileNameWithoutExtension(title);
            object value = type.GetFields().FirstOrDefault(p => p.Name == name)?.GetValue(type)
                ?? type.GetProperties().FirstOrDefault(p => p.Name == name)?.GetValue(type);
            if (value is int) return (int)value;
            return 0;
        }
    }
}