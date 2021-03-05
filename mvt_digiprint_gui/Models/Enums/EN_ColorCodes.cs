using RX_Common;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_ColorCodeList: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_ColorCodeList()
        {
            _List = new List<RxEnum<int>>();
            _List.Add(new RxEnum<int>( 0, RX_DigiPrint.Resources.Language.Resources.BlackK));
            _List.Add(new RxEnum<int>( 1, RX_DigiPrint.Resources.Language.Resources.CyanC));
            _List.Add(new RxEnum<int>( 2, RX_DigiPrint.Resources.Language.Resources.MagentaM));
            _List.Add(new RxEnum<int>( 3, RX_DigiPrint.Resources.Language.Resources.YellowY));
            _List.Add(new RxEnum<int>( 4, RX_DigiPrint.Resources.Language.Resources.RedOrangeR));
            _List.Add(new RxEnum<int>( 5, RX_DigiPrint.Resources.Language.Resources.GreenG));
            _List.Add(new RxEnum<int>( 6, RX_DigiPrint.Resources.Language.Resources.BlueB));
            _List.Add(new RxEnum<int>( 7, RX_DigiPrint.Resources.Language.Resources.LightBlackLK));
            _List.Add(new RxEnum<int>( 8, RX_DigiPrint.Resources.Language.Resources.LightCyanLC));
            _List.Add(new RxEnum<int>( 9, RX_DigiPrint.Resources.Language.Resources.LightMagentaLM));
            _List.Add(new RxEnum<int>(10, RX_DigiPrint.Resources.Language.Resources.LightBlack2LLK));
            _List.Add(new RxEnum<int>(11, RX_DigiPrint.Resources.Language.Resources.LightCyan2LLC));
            _List.Add(new RxEnum<int>(12, RX_DigiPrint.Resources.Language.Resources.LightMagenta2LLM));
            _List.Add(new RxEnum<int>(13, RX_DigiPrint.Resources.Language.Resources.LightYellowLY));
            _List.Add(new RxEnum<int>(14, RX_DigiPrint.Resources.Language.Resources.LightRedLR));
            _List.Add(new RxEnum<int>(15, RX_DigiPrint.Resources.Language.Resources.LightGreenLG));
            _List.Add(new RxEnum<int>(16, RX_DigiPrint.Resources.Language.Resources.LightBlueLB));
            _List.Add(new RxEnum<int>(17, RX_DigiPrint.Resources.Language.Resources.SpotColor6S6));
            _List.Add(new RxEnum<int>(18, RX_DigiPrint.Resources.Language.Resources.SpotColor5S5));
            _List.Add(new RxEnum<int>(19, RX_DigiPrint.Resources.Language.Resources.SpotColor4S4));
            _List.Add(new RxEnum<int>(20, RX_DigiPrint.Resources.Language.Resources.SpotColor3S3));
            _List.Add(new RxEnum<int>(21, RX_DigiPrint.Resources.Language.Resources.SpotColor2S2));
            _List.Add(new RxEnum<int>(22, RX_DigiPrint.Resources.Language.Resources.SpotColor1S1));
            _List.Add(new RxEnum<int>(23, RX_DigiPrint.Resources.Language.Resources.LightYellow2LLY));
            _List.Add(new RxEnum<int>(24, RX_DigiPrint.Resources.Language.Resources.MidlayerMID));
            _List.Add(new RxEnum<int>(25, RX_DigiPrint.Resources.Language.Resources.Primer1PRI1));
            _List.Add(new RxEnum<int>(26, RX_DigiPrint.Resources.Language.Resources.Primer2PRI2));
            _List.Add(new RxEnum<int>(27, RX_DigiPrint.Resources.Language.Resources.VarnishFinish4V4));
            _List.Add(new RxEnum<int>(28, RX_DigiPrint.Resources.Language.Resources.VarnishFinish3V3));
            _List.Add(new RxEnum<int>(29, RX_DigiPrint.Resources.Language.Resources.VarnishFinish2V2));
            _List.Add(new RxEnum<int>(30, RX_DigiPrint.Resources.Language.Resources.VarnishFinish1V1));
            _List.Add(new RxEnum<int>(31, RX_DigiPrint.Resources.Language.Resources.White4W4));
            _List.Add(new RxEnum<int>(32, RX_DigiPrint.Resources.Language.Resources.White3W3));
            _List.Add(new RxEnum<int>(33, RX_DigiPrint.Resources.Language.Resources.White2W2));
            _List.Add(new RxEnum<int>(34, RX_DigiPrint.Resources.Language.Resources.White1W1));
            _List.Add(new RxEnum<int>(35, RX_DigiPrint.Resources.Language.Resources.ProfileChannel8P8));
            _List.Add(new RxEnum<int>(36, RX_DigiPrint.Resources.Language.Resources.ProfileChannel9P9));
            _List.Add(new RxEnum<int>(37, RX_DigiPrint.Resources.Language.Resources.ProfileChannel10P10));
            _List.Add(new RxEnum<int>(38, RX_DigiPrint.Resources.Language.Resources.ProfileChannel11P11));
            _List.Add(new RxEnum<int>(39, RX_DigiPrint.Resources.Language.Resources.ProfileChannel12P12));
            _List.Add(new RxEnum<int>(40, RX_DigiPrint.Resources.Language.Resources.LightBlack33LK));
            _List.Add(new RxEnum<int>(41, RX_DigiPrint.Resources.Language.Resources.LightBlack44LK));
            _List.Add(new RxEnum<int>(42, RX_DigiPrint.Resources.Language.Resources.LightBlack55LK));
            _List.Add(new RxEnum<int>(43, RX_DigiPrint.Resources.Language.Resources.LightBlack66LK));
            _List.Add(new RxEnum<int>(44, RX_DigiPrint.Resources.Language.Resources.LightProfileChannel8LP8));
            _List.Add(new RxEnum<int>(45, RX_DigiPrint.Resources.Language.Resources.LightProfileChannel9LP9));
            _List.Add(new RxEnum<int>(46, RX_DigiPrint.Resources.Language.Resources.LightProfileChannel10LP10));
            _List.Add(new RxEnum<int>(47, RX_DigiPrint.Resources.Language.Resources.LightProfileChannel11LP11));
            _List.Add(new RxEnum<int>(48, RX_DigiPrint.Resources.Language.Resources.LightProfileChannel12LP12));
            _List.Add(new RxEnum<int>(49, RX_DigiPrint.Resources.Language.Resources.LightSpotColor6LS6));
            _List.Add(new RxEnum<int>(50, RX_DigiPrint.Resources.Language.Resources.LightSpotColor5LS5));
            _List.Add(new RxEnum<int>(51, RX_DigiPrint.Resources.Language.Resources.LightSpotColor4LS4));
            _List.Add(new RxEnum<int>(52, RX_DigiPrint.Resources.Language.Resources.LightSpotColor3LS3));
            _List.Add(new RxEnum<int>(53, RX_DigiPrint.Resources.Language.Resources.LightSpotColor2LS2));
            _List.Add(new RxEnum<int>(54, RX_DigiPrint.Resources.Language.Resources.LightSpotColor1LS1));
        }

        //--- List -------------------
        public List<RxEnum<int>> List
        {
            get {return _List;}
        }
        
        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

    }
}
