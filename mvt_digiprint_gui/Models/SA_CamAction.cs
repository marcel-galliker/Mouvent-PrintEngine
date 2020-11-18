using rx_CamLib;
using RX_Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
	public class SA_Action: RxBindable
	{
		//--- Property Name ---------------------------------------
		private string _Name;
		public string Name
		{
			get { return _Name; }
			set { SetProperty(ref _Name,value); }
		}

		//--- Property PrintbarNo ---------------------------------------
		private int _PrintbarNo;
		public int PrintbarNo
		{
			get { return _PrintbarNo; }
			set { SetProperty(ref _PrintbarNo,value); }
		}

		//--- Property HeadNo ---------------------------------------
		private int _HeadNo;
		public int HeadNo
		{
			get { return _HeadNo; }
			set { SetProperty(ref _HeadNo,value); }
		}

		//--- Property WebMoveDist ---------------------------------------
		private double _WebMoveDist;
		public double WebMoveDist
		{
			get { return _WebMoveDist; }
			set { SetProperty(ref _WebMoveDist,value); }
		}

		//--- Property ScanPos ---------------------------------------
		private double _ScanPos;
		public double ScanPos
		{
			get { return _ScanPos; }
			set { SetProperty(ref _ScanPos, value); }
		}

		//--- Property ScanMoveDone ---------------------------------------
		private bool _ScanMoveDone;
		public bool ScanMoveDone
		{
			get { return _ScanMoveDone; }
			set { SetProperty(ref _ScanMoveDone,value); }
		}

		//--- Property WebMoveDone ---------------------------------------
		private bool _WebMoveDone;
		public bool WebMoveDone
		{
			get { return _WebMoveDone; }
			set { SetProperty(ref _WebMoveDone,value); }
		}

		//--- Property State ---------------------------------------
		private ECamFunctionState _State;
		public ECamFunctionState State
		{
			get { return _State; }
			set { SetProperty(ref _State,value); }
		}

		//--- Property Measured ---------------------------------------
		private SPosition _Measured;
		public SPosition Measured
		{
			get { return _Measured; }
			set { SetProperty(ref _Measured,value); }
		}
	}
}
