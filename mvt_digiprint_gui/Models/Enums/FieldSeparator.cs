﻿
using RX_Common;
using System.Collections;
using System.Collections.Generic;


namespace RX_LabelComposer.Models.Enums
{
    public enum FieldSeparatorEnum : byte
    {
        Undef = 0,
        Tab = 0x09,
        Semicolon = (byte)';',
        Comma = (byte)','
    }

    public class FieldSeparatorList : IEnumerable
    {
        private static List<RxEnum<FieldSeparatorEnum>> _List;

        public FieldSeparatorList()
        {
            _List = new List<RxEnum<FieldSeparatorEnum>>();
            _List.Add(new RxEnum<FieldSeparatorEnum>(FieldSeparatorEnum.Tab, RX_DigiPrint.Resources.Language.Resources.Tab));
            _List.Add(new RxEnum<FieldSeparatorEnum>(FieldSeparatorEnum.Semicolon, RX_DigiPrint.Resources.Language.Resources.Semicolon));
            _List.Add(new RxEnum<FieldSeparatorEnum>(FieldSeparatorEnum.Comma, RX_DigiPrint.Resources.Language.Resources.Comma));
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<FieldSeparatorEnum>>(_List);
        }
    }

 }
