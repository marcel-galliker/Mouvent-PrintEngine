using RX_Common;
using System.Collections;
using System.Collections.Generic;


namespace RX_LabelComposer.Models.Enums
{
    public enum FileFormatEnum : int
    {
        Undef = 0,
        Fixed = 1,
        Variable = 2
    }

    public class FileFormatList : IEnumerable
    {
        private static List<RxEnum<FileFormatEnum>> _List;

        public FileFormatList()
        {
            _List = new List<RxEnum<FileFormatEnum>>();
            _List.Add(new RxEnum<FileFormatEnum>(FileFormatEnum.Fixed, RX_DigiPrint.Resources.Language.Resources.Fixed));
            _List.Add(new RxEnum<FileFormatEnum>(FileFormatEnum.Variable, RX_DigiPrint.Resources.Language.Resources.Variable));
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<FileFormatEnum>>(_List);
        }
    }
 }
