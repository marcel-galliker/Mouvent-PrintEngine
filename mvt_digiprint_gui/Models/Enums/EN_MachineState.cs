using RX_Common;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models.Enums
{
    public enum MachineStateEnum: int
    {
        Undef = 0,
        Error = 1,
        BootUp = 2,
        Stop = 3,
        Prepare = 4,
        Pause = 5,
        Run = 6,
        Setup = 7,
        WarmUp = 8,
        WebIn = 9,
        Washing = 10,
        Cleaning = 11,
        Glue = 12,
        Referencing = 13,
        Service = 14,
        WebOut = 15,
        Maintenance = 16,
    }
public class EN_MachineStateList: IEnumerable
    {
        private static List<RxEnum<int>> _List;

        public EN_MachineStateList()
        {
            _List = new List<RxEnum<int>>();
            _List.Add(new RxEnum<int>((int) MachineStateEnum.Undef, RX_DigiPrint.Resources.Language.Resources.Undef));
            _List.Add(new RxEnum<int>((int) MachineStateEnum.Error, RX_DigiPrint.Resources.Language.Resources.Error));
            _List.Add(new RxEnum<int>((int) MachineStateEnum.BootUp, RX_DigiPrint.Resources.Language.Resources.BootUp));
            _List.Add(new RxEnum<int>((int) MachineStateEnum.Stop, RX_DigiPrint.Resources.Language.Resources.Stop));
            _List.Add(new RxEnum<int>((int) MachineStateEnum.Prepare, RX_DigiPrint.Resources.Language.Resources.Prepare));
            _List.Add(new RxEnum<int>((int) MachineStateEnum.Pause, RX_DigiPrint.Resources.Language.Resources.Pause));
            _List.Add(new RxEnum<int>((int) MachineStateEnum.Run, RX_DigiPrint.Resources.Language.Resources.Run));
            _List.Add(new RxEnum<int>((int) MachineStateEnum.Setup, RX_DigiPrint.Resources.Language.Resources.Setup));
            _List.Add(new RxEnum<int>((int) MachineStateEnum.WarmUp, RX_DigiPrint.Resources.Language.Resources.WarmUp));
            _List.Add(new RxEnum<int>((int) MachineStateEnum.WebIn, RX_DigiPrint.Resources.Language.Resources.WebIn));
            _List.Add(new RxEnum<int>((int) MachineStateEnum.Washing, RX_DigiPrint.Resources.Language.Resources.Washing));
            _List.Add(new RxEnum<int>((int) MachineStateEnum.Cleaning, RX_DigiPrint.Resources.Language.Resources.Cleaning));
            _List.Add(new RxEnum<int>((int) MachineStateEnum.Glue, RX_DigiPrint.Resources.Language.Resources.Glue));
            _List.Add(new RxEnum<int>((int) MachineStateEnum.Referencing, RX_DigiPrint.Resources.Language.Resources.Referencing));
            _List.Add(new RxEnum<int>((int) MachineStateEnum.Service, RX_DigiPrint.Resources.Language.Resources.Service));
            _List.Add(new RxEnum<int>((int) MachineStateEnum.WebOut, RX_DigiPrint.Resources.Language.Resources.WebOut));
            _List.Add(new RxEnum<int>((int) MachineStateEnum.Maintenance, RX_DigiPrint.Resources.Language.Resources.Maintenance));
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
        }

        public string GetDisplay(int value)
        {
            foreach(RxEnum<int> item in _List)
            {
                if (item.Value==value) return item.Display;
            }
            return null;
        }
    }
}
