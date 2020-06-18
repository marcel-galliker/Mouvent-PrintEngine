using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace RX_DigiPrint.Views.PrintSystemView
{
    /// <summary>
    /// Interaction logic for DensityView.xaml
    /// </summary>
    public partial class DensityView : UserControl
    {
        private const int _cnt=12;
        private List<RxNumBox> _Numbers = new List<RxNumBox>();

        public DensityView()
        {
            InitializeComponent();
            _init_view();
            DataContext = this;
            RxGlobals.DensityView = this;
        }

        //--- LoadFile ----------------
        public void LoadFile()
        {
            TcpIp.SDensityValuesMsg msg = new TcpIp.SDensityValuesMsg();
            msg.values.head  = _HeadNo;
            RxGlobals.RxInterface.SendMsg(TcpIp.CMD_GET_DENSITY_VAL, ref msg);  
        }

        //--- Property HeadNo ---------------------------------------
        private int _HeadNo;
        public int HeadNo
        {
            get { return _HeadNo; }
            set 
            {
                _HeadNo = value-1;
                if (_HeadNo>=0 && _HeadNo<RxGlobals.HeadStat.List.Count())
                {
                    HeadStat stat = RxGlobals.HeadStat.List[_HeadNo];
                    if (stat!=null) SetValues(stat.Voltage, stat.DensityValue);
                }
            }
        }

        //--- Property Volgate ---------------------------------------
        private byte _Voltage;
        public byte Voltage
        {
            get { return _Voltage; }
            set { _Voltage=value; }
        }

        //--- SetValues ------------------------------------------
        public void SetValues(byte voltage, Int16[] values)
		{
            Voltage = voltage;
            for (int i=0; i<_Numbers.Count; i++)
            {
                _Numbers[i].Text = values[i].ToString();
            }
		}

        //--- _init_view -------------------------------
        private void _init_view()
        {
            int i;
            for (i=0; i<_cnt; i++)
            {
                MainGrid.ColumnDefinitions.Add(new ColumnDefinition(){Width=new GridLength(40)});
                _Numbers.Add(new RxNumBox());
                Grid.SetColumn(_Numbers[i], i);
                Grid.SetRow   (_Numbers[i], 2);
                MainGrid.Children.Add(_Numbers[i]);
            }
        }

        //--- Save_Clicked ---------------------------------------------
        public void Save_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxGlobals.PrintSystem.HeadsPerColor>0)
            {
                TcpIp.SDensityValuesMsg msg = new TcpIp.SDensityValuesMsg();
                msg.values.value = new Int16[_cnt];
                msg.values.head  = _HeadNo;
                msg.values.voltage = _Voltage;
                for (int i=0; i<_cnt; i++)
                {
                    msg.values.value[i] = (Int16)(Math.Round(_Numbers[i].Value, 0));
                }

                RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SET_DENSITY_VAL, ref msg);  
            }
        }
    }
}
