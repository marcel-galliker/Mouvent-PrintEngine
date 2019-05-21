
using RX_Common;
using System.Collections;
using System.Collections.Generic;


namespace RX_LabelComposer.Models.Enums
{
    public enum AlignmentEnum : int
    {
        // defines in wingdi.h
        Left    = 0,
        Right   = 2,
        Center  = 6,
        NewLine = 32
    }

    public class AlignmentList : IEnumerable
    {
        private static List<RxEnum<AlignmentEnum>> _List;

        public AlignmentList()
        {
            if (_List == null)
            {
                _List = new List<RxEnum<AlignmentEnum>>();
                _List.Add(new RxEnum<AlignmentEnum>(AlignmentEnum.Left,    "Left"));
                _List.Add(new RxEnum<AlignmentEnum>(AlignmentEnum.Right,   "Right"));
                _List.Add(new RxEnum<AlignmentEnum>(AlignmentEnum.Center,  "Center"));
                _List.Add(new RxEnum<AlignmentEnum>(AlignmentEnum.NewLine, "New Line"));
            }
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<AlignmentEnum>>(_List);
        }
    }
 }
