using RX_Common;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Windows;

namespace RX_LabelComposer.Models.Enums
{

    public class BarcodeSizeList : IEnumerable
    {
        private static List<RxEnum<short>> _List;
        private static BcTypeEnum _Type=0;

        public BarcodeSizeList(BcTypeEnum type)
        {
            if (type!=_Type)
            {
                _Type = type;
                _List = new List<RxEnum<short>>();
                if (type == BcTypeEnum.eBC_DataMatrix)
                {
                    _List.Add(new RxEnum<short>(0, "Default Square"));
                    _List.Add(new RxEnum<short>(1234, "Default Rect"));
                    _List.Add(new RxEnum<short>( 1, "10  x 10"));
                    _List.Add(new RxEnum<short>( 2, "12  x 12"));
                    _List.Add(new RxEnum<short>( 3, "14  x 14"));
                    _List.Add(new RxEnum<short>( 4, "16  x 16"));
                    _List.Add(new RxEnum<short>( 5, "18  x 18"));
                    _List.Add(new RxEnum<short>( 6, "20  x 20"));
                    _List.Add(new RxEnum<short>( 7, "22  x 22"));
                    _List.Add(new RxEnum<short>( 8, "24  x 24"));
                    _List.Add(new RxEnum<short>( 9, "26  x 26"));
                    _List.Add(new RxEnum<short>(10, "32  x 32"));
                    _List.Add(new RxEnum<short>(11, "36  x 36"));
                    _List.Add(new RxEnum<short>(12, "40  x 40"));
                    _List.Add(new RxEnum<short>(13, "44  x 44"));
                    _List.Add(new RxEnum<short>(14, "48  x 48"));
                    _List.Add(new RxEnum<short>(15, "52  x 52"));
                    _List.Add(new RxEnum<short>(16, "64  x 64"));
                    _List.Add(new RxEnum<short>(17, "72  x 72"));
                    _List.Add(new RxEnum<short>(18, "80  x 80"));
                    _List.Add(new RxEnum<short>(19, "88  x 88"));
                    _List.Add(new RxEnum<short>(20, "96  x 96"));
                    _List.Add(new RxEnum<short>(21, "104 x 104"));
                    _List.Add(new RxEnum<short>(22, "120 x 120"));
                    _List.Add(new RxEnum<short>(23, "132 x 132"));
                    _List.Add(new RxEnum<short>(24, "144 x 144"));
                    _List.Add(new RxEnum<short>(25, "8   x 18"));
                    _List.Add(new RxEnum<short>(26, "8   x 32"));
                    _List.Add(new RxEnum<short>(27, "12  x 26"));
                    _List.Add(new RxEnum<short>(28, "12  x 36"));
                    _List.Add(new RxEnum<short>(29, "16  x 36"));
                    _List.Add(new RxEnum<short>(30, "16  x 48"));
                }
                else if (type == BcTypeEnum.eBC_QRCode)
                {
                    _List.Add(new RxEnum<short>( 0, "Default"));
                    _List.Add(new RxEnum<short>( 1, "21 x 21"));
                    _List.Add(new RxEnum<short>( 2, "25 x 25"));
                    _List.Add(new RxEnum<short>( 3, "29 x 29"));
                    _List.Add(new RxEnum<short>( 4, "33 x 33"));
                    _List.Add(new RxEnum<short>( 5, "37 x 37"));
                    _List.Add(new RxEnum<short>( 6, "41 x 41"));
                    _List.Add(new RxEnum<short>( 7, "45 x 45"));
                    _List.Add(new RxEnum<short>( 8, "49 x 49"));
                    _List.Add(new RxEnum<short>( 9, "53 x 53"));
                    _List.Add(new RxEnum<short>(10, "57 x 57"));
                    _List.Add(new RxEnum<short>(11, "61 x 61"));
                    _List.Add(new RxEnum<short>(12, "65 x 65"));
                    _List.Add(new RxEnum<short>(13, "69 x 69"));
                    _List.Add(new RxEnum<short>(14, "73 x 73"));
                    _List.Add(new RxEnum<short>(15, "77 x 77"));
                    _List.Add(new RxEnum<short>(16, "81 x 81"));
                    _List.Add(new RxEnum<short>(17, "85 x 85"));
                    _List.Add(new RxEnum<short>(18, "89 x 89"));
                    _List.Add(new RxEnum<short>(29, "93 x 93"));
                    _List.Add(new RxEnum<short>(20, "97 x 97"));
                    _List.Add(new RxEnum<short>(21, "101 x 101"));
                    _List.Add(new RxEnum<short>(22, "105 x 105"));
                    _List.Add(new RxEnum<short>(23, "109 x 109"));
                    _List.Add(new RxEnum<short>(24, "113 x 113"));
                    _List.Add(new RxEnum<short>(25, "117 x 117"));
                    _List.Add(new RxEnum<short>(26, "121 x 121"));
                    _List.Add(new RxEnum<short>(27, "125 x 125"));
                    _List.Add(new RxEnum<short>(28, "129 x 129"));
                    _List.Add(new RxEnum<short>(29, "133 x 133"));
                    _List.Add(new RxEnum<short>(30, "137 x 137"));
                    _List.Add(new RxEnum<short>(31, "141 x 141"));
                    _List.Add(new RxEnum<short>(32, "145 x 145"));
                    _List.Add(new RxEnum<short>(33, "149 x 149"));
                    _List.Add(new RxEnum<short>(34, "153 x 153"));
                    _List.Add(new RxEnum<short>(35, "157 x 157"));
                    _List.Add(new RxEnum<short>(36, "161 x 161"));
                    _List.Add(new RxEnum<short>(37, "165 x 165"));
                    _List.Add(new RxEnum<short>(38, "169 x 169"));
                    _List.Add(new RxEnum<short>(39, "173 x 173"));
                    _List.Add(new RxEnum<short>(40, "177 x 177"));
                }
                else if (type == BcTypeEnum.eBC_Aztec)
                {
                    _List.Add(new RxEnum<short>( 0, "Default"));
                    _List.Add(new RxEnum<short>( 1, "15  x 15"));
                    _List.Add(new RxEnum<short>( 2, "19  x 19"));
                    _List.Add(new RxEnum<short>( 3, "23  x 23"));
                    _List.Add(new RxEnum<short>( 4, "27  x 27"));
                    _List.Add(new RxEnum<short>( 5, "31  x 31"));
                    _List.Add(new RxEnum<short>( 6, "37  x 37"));
                    _List.Add(new RxEnum<short>( 7, "41  x 41"));
                    _List.Add(new RxEnum<short>( 8, "45  x 45"));
                    _List.Add(new RxEnum<short>( 9, "49  x 49"));
                    _List.Add(new RxEnum<short>(10, "53  x 53"));
                    _List.Add(new RxEnum<short>(11, "57  x 57"));
                    _List.Add(new RxEnum<short>(12, "61  x 61"));
                    _List.Add(new RxEnum<short>(13, "67  x 67"));
                    _List.Add(new RxEnum<short>(14, "71  x 71"));
                    _List.Add(new RxEnum<short>(15, "75  x 75"));
                    _List.Add(new RxEnum<short>(16, "79  x 79"));
                    _List.Add(new RxEnum<short>(17, "83  x 83"));
                    _List.Add(new RxEnum<short>(18, "87  x 87"));
                    _List.Add(new RxEnum<short>(29, "91  x 91"));
                    _List.Add(new RxEnum<short>(20, "95  x 95"));
                    _List.Add(new RxEnum<short>(21, "101 x 101"));
                    _List.Add(new RxEnum<short>(22, "105 x 105"));
                    _List.Add(new RxEnum<short>(23, "109 x 109"));
                    _List.Add(new RxEnum<short>(24, "113 x 113"));
                    _List.Add(new RxEnum<short>(25, "117 x 117"));
                    _List.Add(new RxEnum<short>(26, "121 x 121"));
                    _List.Add(new RxEnum<short>(27, "125 x 125"));
                    _List.Add(new RxEnum<short>(28, "131 x 131"));
                    _List.Add(new RxEnum<short>(29, "135 x 135"));
                    _List.Add(new RxEnum<short>(30, "139 x 139"));
                    _List.Add(new RxEnum<short>(31, "143 x 143"));
                    _List.Add(new RxEnum<short>(32, "147 x 147"));
                    _List.Add(new RxEnum<short>(33, "151 x 151"));
                    _List.Add(new RxEnum<short>(34, "19  x 19 reader"));
                    _List.Add(new RxEnum<short>(35, "23  x 23 reader"));
                    _List.Add(new RxEnum<short>(36, "27  x 27 reader"));
                    _List.Add(new RxEnum<short>(37, "Aztec Rune"));
                }
                else if (type == BcTypeEnum.eBC_MicroPDF417)
                {
                    _List.Add(new RxEnum<short>( 0, "Default"));
                    _List.Add(new RxEnum<short>( 1, "1 x 11"));
                    _List.Add(new RxEnum<short>( 2, "1 x 14"));
                    _List.Add(new RxEnum<short>( 3, "1 x 17"));
                    _List.Add(new RxEnum<short>( 4, "1 x 20"));
                    _List.Add(new RxEnum<short>( 5, "1 x 24"));
                    _List.Add(new RxEnum<short>( 6, "1 x 28"));
                    _List.Add(new RxEnum<short>( 7, "2 x 8" ));
                    _List.Add(new RxEnum<short>( 8, "2 x 11"));
                    _List.Add(new RxEnum<short>( 9, "2 x 14"));
                    _List.Add(new RxEnum<short>(10, "2 x 17"));
                    _List.Add(new RxEnum<short>(11, "2 x 20"));
                    _List.Add(new RxEnum<short>(12, "2 x 23"));
                    _List.Add(new RxEnum<short>(13, "2 x 26"));
                    _List.Add(new RxEnum<short>(14, "3 x 6" ));
                    _List.Add(new RxEnum<short>(15, "3 x 8" ));
                    _List.Add(new RxEnum<short>(16, "3 x 10"));
                    _List.Add(new RxEnum<short>(17, "3 x 12"));
                    _List.Add(new RxEnum<short>(18, "3 x 15"));
                    _List.Add(new RxEnum<short>(29, "3 x 20"));
                    _List.Add(new RxEnum<short>(20, "3 x 26"));
                    _List.Add(new RxEnum<short>(21, "3 x 32"));
                    _List.Add(new RxEnum<short>(22, "3 x 38"));
                    _List.Add(new RxEnum<short>(23, "3 x 44"));
                    _List.Add(new RxEnum<short>(24, "4 x 4" ));
                    _List.Add(new RxEnum<short>(25, "4 x 6" ));
                    _List.Add(new RxEnum<short>(26, "4 x 8" ));
                    _List.Add(new RxEnum<short>(27, "4 x 10"));
                    _List.Add(new RxEnum<short>(28, "4 x 12"));
                    _List.Add(new RxEnum<short>(29, "4 x 15"));
                    _List.Add(new RxEnum<short>(30, "4 x 20"));
                    _List.Add(new RxEnum<short>(31, "4 x 26"));
                    _List.Add(new RxEnum<short>(32, "4 x 32"));
                    _List.Add(new RxEnum<short>(33, "4 x 38"));
                    _List.Add(new RxEnum<short>(34, "4 x 44"));
                }
                else if (type == BcTypeEnum.eBC_DotCode)
                {
                    _List.Add(new RxEnum<short>(-1, "Default"));
                    _List.Add(new RxEnum<short>(0, "Ratio Width/Height"));
                    _List.Add(new RxEnum<short>(1, "Fixed Width"));
                    _List.Add(new RxEnum<short>(2, "Fixed Height"));
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
