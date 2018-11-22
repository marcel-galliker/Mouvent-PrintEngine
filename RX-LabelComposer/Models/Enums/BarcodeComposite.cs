using RX_Common;
using RX_LabelComposer.External;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Windows;

namespace RX_LabelComposer.Models.Enums
{
    public class BarcodeCompositeList  : IEnumerable
    {
        private static List<RxEnum<short>> _List;
        private static BcTypeEnum _Type = 0;
        private static string[] _Names = {"None", "Auto Select", "Code-A", "Code-B", "Code-C"};

        public BarcodeCompositeList(BcTypeEnum type)
        {
            if (type != _Type)
            {
                _Type = type;
                _List = new List<RxEnum<short>>();

                for (short i = 0; i < 5; i++)
                {
                    if (TecIT.BCIsCompositeComponentAllowed((int)type, (int)i))
                    {
                        _List.Add(new RxEnum<short>(i, _Names[i]));
                    }
                }
            }
        }

        public Visibility Visible()
        {
            return (_List.Count() > 0) ? Visibility.Visible : Visibility.Collapsed;
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<short>>(_List);
        }
    }
}
