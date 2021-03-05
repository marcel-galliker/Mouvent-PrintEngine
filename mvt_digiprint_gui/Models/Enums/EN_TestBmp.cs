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
            _List = new List<RxEnum<ETestImage>>();
            _List.Add(new RxEnum<ETestImage>( ETestImage.jets, RX_DigiPrint.Resources.Language.Resources.Jets));
            _List.Add(new RxEnum<ETestImage>( ETestImage.jet_numbers, RX_DigiPrint.Resources.Language.Resources.JetNumbers));
            _List.Add(new RxEnum<ETestImage>( ETestImage.encoder, RX_DigiPrint.Resources.Language.Resources.Encoder));
            if (RxGlobals.User.UserType>=EUserType.usr_service)
			{
                _List.Add(new RxEnum<ETestImage>( ETestImage.fullAlignment, RX_DigiPrint.Resources.Language.Resources.FullAlignment));
                _List.Add(new RxEnum<ETestImage>( ETestImage.density, RX_DigiPrint.Resources.Language.Resources.Density));   
                if (RxGlobals.PrintSystem.IsTx)
                    _List.Add(new RxEnum<ETestImage>( ETestImage.scanning, RX_DigiPrint.Resources.Language.Resources.Scanning));
                _List.Add(new RxEnum<ETestImage>( ETestImage.angle_overlap, RX_DigiPrint.Resources.Language.Resources.AngleOverlapped));
                _List.Add(new RxEnum<ETestImage>( ETestImage.angle_separated, RX_DigiPrint.Resources.Language.Resources.AngleSeparated));
                _List.Add(new RxEnum<ETestImage>( ETestImage.grid, RX_DigiPrint.Resources.Language.Resources.Grid));
			}
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<ETestImage>>(_List);
        }
    }
}
