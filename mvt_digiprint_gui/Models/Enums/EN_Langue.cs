using RX_Common;
using System.Collections;
using System.Collections.Generic;
using System.Globalization;

namespace RX_DigiPrint.Models.Enums
{
    public enum ELangues : int
    {
        English,
        French,
        German,
        Spanish,
        Italian,
        Dutch,
        Polish,
        Portuguese,
        Romanian,
    }

    public class EN_LanguesList : IEnumerable
    {
        private static List<RxEnum<ELangues>> _List;

        private static Dictionary<ELangues, CultureInfo> _Culture;

        public EN_LanguesList()
        {
            if (_List == null)
            {
                _List = new List<RxEnum<ELangues>>();
                _List.Add(new RxEnum<ELangues>(ELangues.English, "English"));
                _List.Add(new RxEnum<ELangues>(ELangues.French, "Français"));
                _List.Add(new RxEnum<ELangues>(ELangues.German, "Deutsch"));
                _List.Add(new RxEnum<ELangues>(ELangues.Spanish, "Español"));
                _List.Add(new RxEnum<ELangues>(ELangues.Italian, "Italiano"));
                /* Other Language that are not yet shown... 
                _List.Add(new RxEnum<ELangues>(ELangues.Dutch, "Nederlands"));
                _List.Add(new RxEnum<ELangues>(ELangues.Polish, "Polski"));
                _List.Add(new RxEnum<ELangues>(ELangues.Portuguese, "Português"));
                _List.Add(new RxEnum<ELangues>(ELangues.Romanian, "Română"));
                */
                _Culture = new Dictionary<ELangues, CultureInfo>() {
                    {ELangues.English, CultureInfo.GetCultureInfo("en-US") },
                    {ELangues.French, CultureInfo.GetCultureInfo("fr-FR")},
                    {ELangues.German, CultureInfo.GetCultureInfo("de-DE")},
                    {ELangues.Spanish, CultureInfo.GetCultureInfo("es-ES")},
                    {ELangues.Italian, CultureInfo.GetCultureInfo("it-IT")},
                    {ELangues.Dutch, CultureInfo.GetCultureInfo("nl-NL")},
                    {ELangues.Polish, CultureInfo.GetCultureInfo("pl-PL")},
                    {ELangues.Portuguese, CultureInfo.GetCultureInfo("pt-PT")},
                    {ELangues.Romanian, CultureInfo.GetCultureInfo("ro-RO") }
                };
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<ELangues>>(_List);
        }

        public CultureInfo GetCultureInfo(ELangues lang) 
        {
            return _Culture[lang];
        }
    }
}

