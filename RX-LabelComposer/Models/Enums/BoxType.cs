
using RX_Common;
using System.Collections;
using System.Collections.Generic;


namespace RX_LabelComposer.Models.Enums
{
    public enum BoxTypeEnum : int
    {
        Text    = 0,
        Barcode = 1,
        Bitmap  = 2
    }

    public class BoxTypeList : IEnumerable
    {
        private static List<RxEnum<BoxTypeEnum>> _List;

        public BoxTypeList()
        {
            if (_List == null)
            {
                _List = new List<RxEnum<BoxTypeEnum>>();
                _List.Add(new RxEnum<BoxTypeEnum>(BoxTypeEnum.Text,       "Text"));
                _List.Add(new RxEnum<BoxTypeEnum>(BoxTypeEnum.Barcode,    "Barcode"));
 //               _List.Add(new RxEnum<BoxTypeEnum>(BoxTypeEnum.Bitmap,     "Bitmap"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<BoxTypeEnum>>(_List);
        }
    }
 }
