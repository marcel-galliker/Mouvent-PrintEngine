using RX_Common;
using RX_DigiPrint.Services;
using System.Collections;
using System.Collections.Generic;

namespace RX_DigiPrint.Models.Enums
{

    public enum EUnits : uint
    {
        metric      = 0,
        imperial    = 1,
    }

    public class CUnit : RxBindable
    {
        public static CUnit Unit_mm     = new CUnit("mm");
        public static CUnit Unit_N_mm   = new CUnit("N/mm");
        public static CUnit Unit_m      = new CUnit("m");
        public static CUnit Unit_m_min  = new CUnit("m/min");

        public CUnit (string text)
	    {
            _NameMetric = text;
            if (text.Equals("mm"))         {_NameImperial="inch";   _Factor=1.0/25.4;}
            else if (text.Equals("N/mm"))  {_NameImperial="N/inch"; _Factor=1.0/25.4;}
            else if (text.Equals("m"))     {_NameImperial="ft";     _Factor=3.28084/1.0;}
            else if (text.Equals("m/min")) {_NameImperial="ft/min"; _Factor=3.28084/1.0;}
	    }

        //--- Property Factor ---------------------------------------
        private double _Factor=1.0;
        public double Factor
        {
            get 
            { 
                if (RxGlobals.Settings.Units == EUnits.imperial) return _Factor; 
                return 1.0;
            }
        }

        //--- Property Name ---------------------------------------
        private string _NameMetric;
        private string _NameImperial;
        public string Name
        {
            get 
            { 
                if (RxGlobals.Settings.Units == EUnits.imperial) return _NameImperial;
                return _NameMetric;
            }
        }
    }


    public class EN_UnitsList: IEnumerable
    {
        private static List<RxEnum<EUnits>> _List;

        public EN_UnitsList()
        {
            _List = new List<RxEnum<EUnits>>();
            _List.Add(new RxEnum<EUnits>(EUnits.metric,   "metric"));
            _List.Add(new RxEnum<EUnits>(EUnits.imperial, "imperial"));
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<EUnits>>(_List);
        }
    }
}
