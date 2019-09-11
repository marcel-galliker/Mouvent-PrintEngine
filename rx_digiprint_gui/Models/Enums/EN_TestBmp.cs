using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_TestImageList: IEnumerable
    {
        private static List<RxEnum<ETestImage>> _List;

        public EN_TestImageList()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<ETestImage>>();
                _List.Add(new RxEnum<ETestImage>( ETestImage.angle_overlap,   "Angle Overlapped"));
                _List.Add(new RxEnum<ETestImage>( ETestImage.angle_separated, "Angle Separated"));
                _List.Add(new RxEnum<ETestImage>( ETestImage.jets,            "Jets"));
                _List.Add(new RxEnum<ETestImage>( ETestImage.jet_numbers,     "Jet Numbers"));
                _List.Add(new RxEnum<ETestImage>( ETestImage.grid,            "Grid"));
                _List.Add(new RxEnum<ETestImage>( ETestImage.encoder,         "Encoder"));
                _List.Add(new RxEnum<ETestImage>( ETestImage.scanning,        "Scanning"));
                _List.Add(new RxEnum<ETestImage>( ETestImage.fullAlignment,   "Full Alignment"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<ETestImage>>(_List);
        }
    }
}
