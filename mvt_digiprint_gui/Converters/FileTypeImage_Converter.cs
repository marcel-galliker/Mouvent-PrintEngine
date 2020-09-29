using iTextSharp.text.pdf.parser;
using MahApps.Metro.IconPacks;
using RX_DigiPrint.Models;
using System;
using System.IO;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace RX_DigiPrint.Converters
{
    public class FileTypeImage_Converter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            switch ((DirItem.ENFileType)value)
            {
                case DirItem.ENFileType.Directory:  return PackIconMaterialKind.FolderOpenOutline;
                case DirItem.ENFileType.SourceFile: return PackIconMaterialKind.ImageOutline;
                case DirItem.ENFileType.DataFile:   return PackIconMaterialKind.ImageOutline;
                case DirItem.ENFileType.RunList:    return PackIconMaterialKind.ImageMultipleOutline;
                default: return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
