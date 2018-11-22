using RX_Common;
using rx_rip_gui.Services;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{
    public class EN_RenderIntent: IEnumerable
    {
        private static List<RxEnum<ERenderIntent>> _List;

        public EN_RenderIntent()
        {
            if (_List==null)
            {
                _List = new List<RxEnum<ERenderIntent>>();

                _List.Add(new RxEnum<ERenderIntent>( ERenderIntent.ri_undef,        "undef"));
                _List.Add(new RxEnum<ERenderIntent>( ERenderIntent.ri_perceptual,   "perceptual"));
                _List.Add(new RxEnum<ERenderIntent>( ERenderIntent.ri_colorimetric, "colorimetric"));
                _List.Add(new RxEnum<ERenderIntent>( ERenderIntent.ri_saturation,   "saturation"));
                _List.Add(new RxEnum<ERenderIntent>( ERenderIntent.ri_absolute,     "absolute colorimetric"));
            }
        }

       IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<ERenderIntent>>(_List);
        }

    }
}
