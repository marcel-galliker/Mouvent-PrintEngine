using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
	public class SA_Base : RxBindable
	{
		private Action _OnScanMoveDone = null;
		private Action _OnWebMoveDone  = null;

		public SA_Base()
		{
			RxGlobals.Timer.TimerFct += _Tick;
		}

		//--- Property powerStepStatus ---------------------------------------
		private UInt32 _powerStepStatus;
		public UInt32 powerStepStatus
		{
			get { return _powerStepStatus; }
			set { SetProperty(ref _powerStepStatus,value); }
		}

		//--- Property motorPosition ---------------------------------------
		private double _motorPosition;
		public double motorPosition
		{
			get { return _motorPosition; }
			set { SetProperty(ref _motorPosition, value); }
		}

		//--- Property stopPos ---------------------------------------
		private double _stopPos;
		public double stopPos
		{
			get { return _stopPos; }
			set { SetProperty(ref _stopPos,value); }
		}

		//--- Property motorVoltage ---------------------------------------
		private UInt32 _motorVoltage;
		public UInt32 motorVoltage
		{
			get { return _motorVoltage; }
			set { SetProperty(ref _motorVoltage,value); }
		}
		//--- Property motorMoveCurrent ---------------------------------------
		private UInt32 _motorMoveCurrent;
		public UInt32 motorMoveCurrent
		{
			get { return _motorMoveCurrent; }
			set { SetProperty(ref _motorMoveCurrent,value); }
		}

		//--- Property motorHoldCurrent ---------------------------------------
		private UInt32 _motorHoldCurrent;
		public UInt32 motorHoldCurrent
		{
			get { return _motorHoldCurrent; }
			set { SetProperty(ref _motorHoldCurrent,value); }
		}
		//--- Property refDone ---------------------------------------
		private bool _refDone;
		public bool refDone
		{
			get { return _refDone; }
			set { SetProperty(ref _refDone,value); }
		}

		//--- Property moving ---------------------------------------
		private bool _moving;
		public bool moving
		{
			get { return _moving; }
			set { SetProperty(ref _moving, value); }				
		}

		//--- Property ScanMoveCnt ---------------------------------------
		private int _ScanMoveCnt;
		public int scanMoveCnt
		{
			get { return _ScanMoveCnt; }
			set 
			{ 
				if (SetProperty(ref _ScanMoveCnt,value) )
				{
					Console.WriteLine("{0} SCAN MOVE DONE[{1}] motorPosition={2}", RxGlobals.Timer.Ticks(), _ScanMoveCnt, motorPosition);
					_OnScanMoveDone();
				}
			}
		}

		//--- Property InLeft ---------------------------------------
		private bool _InLeft;
		public bool InLeft
		{
			get { return _InLeft; }
			set { SetProperty(ref _InLeft,value); }
		}

		//--- Property InRight ---------------------------------------
		private bool _InRight;
		public bool InRight
		{
			get { return _InRight; }
			set { SetProperty(ref _InRight,value); }
		}

		//--- Property Connected ---------------------------------------
		private int _ConnectedTimer;
		private bool _Connected=false;
		public bool Connected
		{
			get { return _Connected; }
			set { 
					SetProperty(ref _Connected, value); 
					if (_Connected) _ConnectedTimer=5;
				}
		}
		private void _Tick(int no)
		{
			if (_ConnectedTimer>0 && --_ConnectedTimer==0) Connected=false;
			_checkWebMoveDone();
		}

		//--- Update ------------------------------------------------
		public void Update(TcpIp.SSetupAssistStatMsg msg)
		{
			powerStepStatus = msg.powerStepStatus;
			motorPosition	= Math.Round(msg.motorPosition/1000.0, 1);
			stopPos		    = Math.Round(msg.stopPos/1000.0, 1);
			motorVoltage	= msg.motorVoltage;
			motorMoveCurrent= msg.motorMoveCurrent;
			motorHoldCurrent= msg.motorHoldCurrent;
			scanMoveCnt		= msg.moveCnt;
			refDone			= msg.refDone!=0;
			moving			= msg.moving!=0;
			InLeft			= (msg.inputs&(1<<0))!=0;
			InRight			= (msg.inputs&(1<<1))!=0;
			Connected		= true;
			_checkWebMoveDone();
		}

		//--- Property OnWebMoveDone ---------------------------------------
		public Action OnScanMoveDone
		{
			set { _OnScanMoveDone = value; }
		}

		//--- Property OnWebMoveDone ---------------------------------------
		public Action OnWebMoveDone
		{
			set { _OnWebMoveDone = value; }
		}

		//--- ScanReference ------------------------------------------
		public void ScanReference()
		{
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_REFERENCE);
		}

		//--- ScanMoveTo --------------------------------------------
		public void ScanMoveTo(double pos, int speed=0)
		{
			TcpIp.SetupAssist_MoveCmd cmd = new TcpIp.SetupAssist_MoveCmd();
			cmd.steps	= (Int32)(pos*1000.0);
			cmd.speed   = (UInt32)speed;
			Console.WriteLine("{0} SCAN MOVE TO [{1}] from {2:N3} to {3:N3}", RxGlobals.Timer.Ticks(), _ScanMoveCnt, motorPosition, pos);
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SA_MOVE, ref cmd);
		}

		//--- ScanStop ------------------------------
		public void ScanStop()
		{
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_STOP);
		}

		//--- Property Dist ---------------------------------------
		private double _WebDist;
		public double WebDist
		{
			get { return _WebDist; }
			set { SetProperty(ref _WebDist,value); }
		}

		//--- WebMove ----------------------------------
		public void WebMove(double? dist=null)
		{
			_checkWebMoveDone();

			TcpIp.SetupAssist_MoveCmd cmd = new TcpIp.SetupAssist_MoveCmd();
			if (dist==null) cmd.steps	= (Int32)(1000*WebDist);
			else            cmd.steps	= (Int32)(1000*dist);
			if (cmd.steps==0)
			{
		//		if (_OnWebMoveDone!=null) _OnWebMoveDone();
			}
			else
			{
				Console.WriteLine("{0}: WEB MOVE {1} dist={2}", RxGlobals.Timer.Ticks(), _WebMoveCnt,  cmd.steps);			
				RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SA_WEB_MOVE, ref cmd);
			}
		}

		//--- WebStop ----------------------------------
		public void WebStop()
		{
			Console.WriteLine("{0}: WEB STOP", RxGlobals.Timer.Ticks());
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_WEB_STOP);
		}
		
		//--- _checkWebMoveDone -----------------------------------------------
		private int _WebMoveCnt=0;
		private void _checkWebMoveDone()
		{
			RxGlobals.Plc.RequestVar("Application.GUI_00_001_Main" + "\n" + "STA_RELATIVE_MOVE_CNT" + "\n");
			int old=_WebMoveCnt;
			_WebMoveCnt=Rx.StrToInt32(RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_RELATIVE_MOVE_CNT")); 
						
			if (_OnWebMoveDone!=null)
			{
				if (_WebMoveCnt!=old) 
				{
					Console.WriteLine("{0}: WEB MOVE DONE {1} (new={2})", RxGlobals.Timer.Ticks(), old, _WebMoveCnt);
					_OnWebMoveDone();
				}
			}
		}

	}
}
