using Infragistics.Controls.Grids;
using RX_Common;
using RX_DigiPrint.Models;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
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
	/// Interaction logic for DisabledJetsView.xaml
	/// </summary>
	public partial class DisabledJetsView :UserControl
	{
		static private ObservableCollection<UInt16> _List = new ObservableCollection<UInt16>();

		public DisabledJetsView()
		{
			InitializeComponent();

			while (_List.Count<TcpIp.MAX_DISABLED_JETS)	_List.Add(0xffff);
			DisabledJetsGrid.ItemsSource = _List;
			RxGlobals.DisabledJetsView = this;
		}

		//--- LoadFile ----------------
        public void LoadFile()
        {
            TcpIp.SDisabledJetsMsg msg = new TcpIp.SDisabledJetsMsg();
            msg.head = _HeadNo;
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_GET_DISABLED_JETS, ref msg);
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
                    if (stat!=null) SetDisablesJets(stat.DisabledJets);
                }
			}
		}

		//--- SetDisablesJets ------------------------------------------
		public void SetDisablesJets(UInt16[] disabledJets)
		{
            for (int i=0; i<_List.Count && i<TcpIp.MAX_DISABLED_JETS; i++)
            {
                _List[i] = disabledJets[i];
            }
		}

		//--- DisabledJetsGrid_ActiveCellChanged ------------------------------
		private void DisabledJetsGrid_ActiveCellChanged(object sender,EventArgs e)
		{
			Infragistics.Controls.Grids.Primitives.UnboundCell cell = DisabledJetsGrid.ActiveCell as Infragistics.Controls.Grids.Primitives.UnboundCell;
			if (cell!=null)
			{
				RxNumPad pad = new RxNumPad(Edit);
                if((bool)pad.ShowDialog())
				{
					UInt16 val=(UInt16)Rx.StrToUInt32(pad.Result);
					if (val>2048+128 || (val==0 && cell.Row.Index>0)) val=0xffff;
					_List[cell.Row.Index] = val;
					_List=new ObservableCollection<UInt16>(_List.OrderBy(i => i));
					DisabledJetsGrid.ItemsSource = _List;
				}
				DisabledJetsGrid.ActiveCell = null;
			}
		}

		//--- Save_Clicked ---------------------------------------------
        public void Save_Clicked(object sender, RoutedEventArgs e)
        {
            if (RxGlobals.PrintSystem.HeadsPerColor>0)
            {
				int i;
                TcpIp.SDisabledJetsMsg msg = new TcpIp.SDisabledJetsMsg();
                msg.head = _HeadNo;
                msg.disabledJets = new UInt16[TcpIp.MAX_DISABLED_JETS];
                for (i=0; i<TcpIp.MAX_DISABLED_JETS; i++) msg.disabledJets[i] = 0xffff;
                for (i=0; i<TcpIp.MAX_DISABLED_JETS && i<_List.Count(); i++)
                {
                    msg.disabledJets[i] = (UInt16)_List[i];
                }
				RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SET_DISABLED_JETS, ref msg);  
            }
        }

	}
}
