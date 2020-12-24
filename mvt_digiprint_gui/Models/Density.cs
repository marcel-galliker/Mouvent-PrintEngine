using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.ObjectModel;
using System.Linq;

namespace RX_DigiPrint.Models
{
    public class DensityValue : RxBindable
    {
        private Int16 _value;
        public Int16 Value
        {
            get { return _value; }
            set 
            {
                if (SetProperty(ref _value, value))
                {
                    RxGlobals.Density.Changed = true;
                }
            }
        }
    }

    public class Density : RxBindable
    {
        private const int _cnt = 12;
        private int HeadNumber { get; set; }

        static private ObservableCollection<DensityValue> _DensityList = new ObservableCollection<DensityValue>();
        public ObservableCollection<DensityValue> DensityList
        {
            get { return _DensityList; }
            set { SetProperty(ref _DensityList, value); }
        }

        private byte _Voltage;
        public byte Voltage
        {
            get { return _Voltage; }
            set 
            {
                if (SetProperty(ref _Voltage, value))
                {
                    Changed = true;
                }
            }
        }

        public string DensityHeader
        {
            get { return "Density [" + '\u2030' + "]"; }
        }


        private bool _Changed = false;
        public bool Changed
        {
            get { return _Changed; }
            set { SetProperty(ref _Changed, value); }
        }

        public Density()
        {
            
        }

        public void Save(ref TcpIp.SDensityMsg msg)
        {
            if (RxGlobals.PrintSystem.HeadsPerColor > 0)
            {
                msg.head = HeadNumber;
                msg.data.densityValue = new Int16[_cnt];
                msg.data.voltage = _Voltage;
                for (int i = 0; i < _cnt; i++)
                {
                    msg.data.densityValue[i] = DensityList[i].Value;
                }
            }

            Changed = false;
        }

        public int GetHeadNumber()
        {
            return HeadNumber;
        }

        public void SetHeadNumber(int headNumber)
        {
            HeadNumber = headNumber;

            if (HeadNumber >= 0 && HeadNumber < RxGlobals.HeadStat.List.Count)
            {
                HeadStat stat = RxGlobals.HeadStat.List[HeadNumber];
                if (stat != null) SetDensityValues(stat.Voltage, stat.DensityValue);
            }
        }

        public void SetDensityValues(byte voltage, Int16[] values)
        {
            var currentList = DensityList.ToList();

            Voltage = voltage;
            DensityList.Clear();

            for (int i = 0; i < values.Length; i++)
            {
                DensityList.Add(new DensityValue { Value = values[i] });
            }

            for (int i = 0; i < (_cnt - values.Length); i++)
            {
                DensityList.Add(new DensityValue { Value = 0 });
            }

            bool changed = false;
            if (currentList.Count != DensityList.Count)
            {
                changed = true;
            }
            else
            {
                int index = 0;
                foreach (var elem in DensityList)
                {
                    if (elem != currentList[index])
                    {
                        changed = true;
                        break;
                    }
                    index++;
                }
            }
            if (changed)
            {
                Changed = true;
            }
        }
    }
}
