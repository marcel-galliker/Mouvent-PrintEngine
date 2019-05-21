using RX_GUI.Helpers;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_GUI.Models.Enums
{
    public class EN_ColorCodeList: IEnumerable
    {
        private static List<RX_Enum<int>> _List;

        public EN_ColorCodeList()
        {
            if (_List==null)
            {
                _List = new List<RX_Enum<int>>();
                _List.Add(new RX_Enum<int>( 0,   "Black (K)"  ));
                _List.Add(new RX_Enum<int>( 1,   "Cyan (C)" ));
                _List.Add(new RX_Enum<int>( 2,   "Magenta(M)"));
                _List.Add(new RX_Enum<int>( 3,   "Yellow (Y)"));
                _List.Add(new RX_Enum<int>( 4,   "Red/Orange (R)"));
                _List.Add(new RX_Enum<int>( 5,   "Green (G)"));
                _List.Add(new RX_Enum<int>( 6,   "Blue (B)"));
                _List.Add(new RX_Enum<int>( 7,   "Light Black (LK)"));
                _List.Add(new RX_Enum<int>( 8,   "Light Cyan (LC)"));
                _List.Add(new RX_Enum<int>( 9,   "Light Magenta (LM)"));
                _List.Add(new RX_Enum<int>(10,   "Light Black 2 (LLK)"));
                _List.Add(new RX_Enum<int>(11,   "Light Cyan 2 (LLC)"));
                _List.Add(new RX_Enum<int>(12,   "Light Magenta 2 (LLM)"));
                _List.Add(new RX_Enum<int>(13,   "Light Yellow (LY)"));
                _List.Add(new RX_Enum<int>(14,   "Light Red (LR)"));
                _List.Add(new RX_Enum<int>(15,   "Light Green (LG)"));
                _List.Add(new RX_Enum<int>(16,   "Light Blue (LB)"));
                _List.Add(new RX_Enum<int>(17,   "Spot Color 6 (S6)"));
                _List.Add(new RX_Enum<int>(18,   "Spot Color 5 (S5)"));
                _List.Add(new RX_Enum<int>(19,   "Spot Color 4 (S4)"));
                _List.Add(new RX_Enum<int>(20,   "Spot Color 3 (S3)"));
                _List.Add(new RX_Enum<int>(21,   "Spot Color 2 (S2)"));
                _List.Add(new RX_Enum<int>(22,   "Spot Color 1 (S1)"));
                _List.Add(new RX_Enum<int>(23,   "Light Yellow 2 (LLY)"));
                _List.Add(new RX_Enum<int>(24,   "Midlayer (MID)"));
                _List.Add(new RX_Enum<int>(25,   "Primer 1 (PRI1)"));
                _List.Add(new RX_Enum<int>(26,   "Primer 2 (PRI2)"));
                _List.Add(new RX_Enum<int>(27,   "Varnish / Finish 4 (V4)"));
                _List.Add(new RX_Enum<int>(28,   "Varnish / Finish 3 (V3)"));
                _List.Add(new RX_Enum<int>(29,   "Varnish / Finish 2 (V2)"));
                _List.Add(new RX_Enum<int>(30,   "Varnish / Finish 1 (V1)"));
                _List.Add(new RX_Enum<int>(31,   "White 4 (W4)"));
                _List.Add(new RX_Enum<int>(32,   "White 3 (W3)"));
                _List.Add(new RX_Enum<int>(33,   "White 2 (W2)"));
                _List.Add(new RX_Enum<int>(34,   "White 1 (W1)"));
                _List.Add(new RX_Enum<int>(35,   "Profile Channel 8 (P8)"));
                _List.Add(new RX_Enum<int>(36,   "Profile Channel 9 (P9)"));
                _List.Add(new RX_Enum<int>(37,   "Profile Channel 10 (P10)"));
                _List.Add(new RX_Enum<int>(38,   "Profile Channel 11 (P11)"));
                _List.Add(new RX_Enum<int>(39,   "Profile Channel 12 (P12)"));
                _List.Add(new RX_Enum<int>(40,   "Light Black 3 (3LK)"));
                _List.Add(new RX_Enum<int>(41,   "Light Black 4 (4LK)"));
                _List.Add(new RX_Enum<int>(42,   "Light Black 5 (5LK)"));
                _List.Add(new RX_Enum<int>(43,   "Light Black 6 (6LK)"));
                _List.Add(new RX_Enum<int>(44,   "Light Profile Channel 8 (LP8)"));
                _List.Add(new RX_Enum<int>(45,   "Light Profile Channel 9 (LP9)"));
                _List.Add(new RX_Enum<int>(46,   "Light Profile Channel 10 (LP10)"));
                _List.Add(new RX_Enum<int>(47,   "Light Profile Channel 11 (LP11)"));
                _List.Add(new RX_Enum<int>(48,   "Light Profile Channel 12 (LP12)"));
                _List.Add(new RX_Enum<int>(49,   "Light Spot Color 6 (LS6)"));
                _List.Add(new RX_Enum<int>(50,   "Light Spot Color 5 (LS5)"));
                _List.Add(new RX_Enum<int>(51,   "Light Spot Color 4 (LS4)"));
                _List.Add(new RX_Enum<int>(52,   "Light Spot Color 3 (LS3)"));
                _List.Add(new RX_Enum<int>(53,   "Light Spot Color 2 (LS2)"));
                _List.Add(new RX_Enum<int>(54,   "Light Spot Color 1 (LS1)"));
             }
        }

        //--- List -------------------
        public List<RX_Enum<int>> List
        {
            get {return _List;}
        }
        
        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RX_Enumerator<RX_Enum<int>>(_List);
        }

    }
}
