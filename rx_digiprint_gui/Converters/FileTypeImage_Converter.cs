using RX_DigiPrint.Models;
using System;
using System.IO;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Media.Imaging;

namespace RX_DigiPrint.Converters
{
    public class FileTypeImage_Converter : IValueConverter
    {
        private static BitmapImage _dirImage  = new BitmapImage(new Uri("..\\..\\Resources\\Bitmaps\\Directory.ico", UriKind.RelativeOrAbsolute));
        private static BitmapImage _fileImage = new BitmapImage(new Uri("..\\..\\Resources\\Bitmaps\\ImageFile.ico", UriKind.RelativeOrAbsolute));

        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {   
            switch((DirItem.ENFileType)value)
            {
                case DirItem.ENFileType.Directory:   return _dirImage;
                case DirItem.ENFileType.SourceFile:  return _fileImage; 
                case DirItem.ENFileType.DataFile:    return _fileImage; 
            }
            return null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
