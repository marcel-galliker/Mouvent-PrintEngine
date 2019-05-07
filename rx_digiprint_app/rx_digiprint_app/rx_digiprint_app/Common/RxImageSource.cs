﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xamarin.Forms;
using Xamarin.Forms.Xaml;

namespace DigiPrint.Common
{
    [ContentProperty ("Source")]
    public class ImageResourceExtension : IMarkupExtension
    {
        public string Source { get; set; } 
 
        public object ProvideValue (IServiceProvider serviceProvider)
        {
            if (Source == null) return null; 
            return ImageSource.FromResource("DigiPrint.Resources.drawable."+Source);
        }
    }
}
