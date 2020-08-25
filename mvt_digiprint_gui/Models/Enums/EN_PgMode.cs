using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Windows.Media.Imaging;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_PgModeList: IEnumerable
    {
        private static List<RxEnum<TcpIp.EPrintGoMode>> _List;
        private static bool _LH702=false;

        public EN_PgModeList()
        {
            bool lh702= (RxGlobals.PrintSystem.PrinterType==EPrinterType.printer_LH702);

            if (_List!=null && lh702!=_LH702) _List=null;

            _LH702 = lh702;
            if (_List==null)
            {
                _List = new List<RxEnum<TcpIp.EPrintGoMode>>();
                _List.Add(new RxEnum<TcpIp.EPrintGoMode>(TcpIp.EPrintGoMode.PG_MODE_MARK,   "Mark",   new BitmapImage(new Uri("../../Resources/Bitmaps/PG_Mark.ico", UriKind.Relative))));
                if (lh702)
				{
                    _List.Add(new RxEnum<TcpIp.EPrintGoMode>(TcpIp.EPrintGoMode.PG_MODE_MARK_INV,   "MarkI",  new BitmapImage(new Uri("../../Resources/Bitmaps/PG_Mark_INV.ico", UriKind.Relative))));
                    _List.Add(new RxEnum<TcpIp.EPrintGoMode>(TcpIp.EPrintGoMode.PG_MODE_MARK_VRT,   "MarkV",  new BitmapImage(new Uri("../../Resources/Bitmaps/PG_Mark_Virtual.ico", UriKind.Relative))));
				}
                _List.Add(new RxEnum<TcpIp.EPrintGoMode>(TcpIp.EPrintGoMode.PG_MODE_LENGTH, "Length", new BitmapImage(new Uri("../../Resources/Bitmaps/PG_Length.ico", UriKind.Relative)) ));
                _List.Add(new RxEnum<TcpIp.EPrintGoMode>(TcpIp.EPrintGoMode.PG_MODE_GAP,    "Gap",    new BitmapImage(new Uri("../../Resources/Bitmaps/PG_Gap.ico", UriKind.Relative))));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<TcpIp.EPrintGoMode>>(_List);
        }
    }
}
