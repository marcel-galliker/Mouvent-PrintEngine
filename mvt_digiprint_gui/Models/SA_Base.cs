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
		private bool   _WebRunning     = false;

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

		//--- Property MoveCnt ---------------------------------------
		private int _MoveCnt;
		public int moveCnt
		{
			get { return _MoveCnt; }
			set 
			{ 
				if (SetProperty(ref _MoveCnt,value) && _OnScanMoveDone!=null)
				{
					Console.WriteLine("{0} ScanMoveDone {1} motorPosition={2}", RxGlobals.Timer.Ticks(), _MoveCnt, motorPosition);
					_OnScanMoveDone();
					_OnScanMoveDone=null;
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
			motorVoltage	= msg.motorVoltage;
			motorMoveCurrent= msg.motorMoveCurrent;
			motorHoldCurrent= msg.motorHoldCurrent;
			moveCnt			= msg.moveCnt;
			refDone			= msg.refDone!=0;
			moving			= msg.moving!=0;
			InLeft			= (msg.inputs&(1<<0))!=0;
			InRight			= (msg.inputs&(1<<1))!=0;
			Connected		= true;
			_checkWebMoveDone();
		}

		//--- Property OnWebMoveDone ---------------------------------------
		public Action OnWebMoveDone
		{
			set {
					_OnWebMoveDone = value; 
					_WebRunning = false;
				}
		}

		//--- _checkWebMoveDone -----------------------------------------------
		private void _checkWebMoveDone()
		{
			EnPlcState state = (EnPlcState)Rx.StrToInt32(RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_MACHINE_STATE"));
			Console.WriteLine("{0}: _checkWebMoveDone _WebRunning={1} state={2} ondone={3}", RxGlobals.Timer.Ticks(), _WebRunning, state, _OnWebMoveDone!=null);
			if (_WebRunning && (state==EnPlcState.plc_pause || state==EnPlcState.plc_stop))
			{
				Console.WriteLine("{0}: WEB MOVE DONE, _OnWebMoveDone={1}", RxGlobals.Timer.Ticks(), _OnWebMoveDone!=null);
				Action onDone=_OnWebMoveDone;
				_OnWebMoveDone = null;
				_WebRunning	   = false;
				if (onDone!=null) onDone();
			}
			if (state==EnPlcState.plc_run) _WebRunning=true;
		}

		//--- ScanReference ------------------------------------------
		public void ScanReference()
		{
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_REFERENCE);
		}

		/*
		//--- ScanMoveBy --------------------------------------------
		public void ScanMoveBy(double dist, Action done=null)
		{
			TcpIp.SetupAssist_MoveCmd cmd = new TcpIp.SetupAssist_MoveCmd();
			_motorPositionSet += dist;
			cmd.steps	= (Int32)(dist*1000.0);
			_OnScanMoveDone = done;
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SA_MOVE, ref cmd);
		}
		*/

		//--- ScanMoveTo --------------------------------------------
		public void ScanMoveTo(double pos, Action onDone=null)
		{
			TcpIp.SetupAssist_MoveCmd cmd = new TcpIp.SetupAssist_MoveCmd();
			cmd.steps	= (Int32)(pos*1000.0);
			_OnScanMoveDone = onDone;
			Console.WriteLine("{0} ScanMove {1} from {2:N3} to {3:N3}", RxGlobals.Timer.Ticks(), _MoveCnt, motorPosition, pos);
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
		public void WebMove(double? dist=null, Action onDone=null)
		{
			Console.WriteLine("{0}: WebMove", RxGlobals.Timer.Ticks());			
			_checkWebMoveDone();

			TcpIp.SetupAssist_MoveCmd cmd = new TcpIp.SetupAssist_MoveCmd();
			if (dist==null) cmd.steps	= (Int32)(1000*WebDist);
			else            cmd.steps	= (Int32)(1000*dist);
			Console.WriteLine("CMD_SA_WEB_MOVE dist={0}", cmd.steps);
			if (cmd.steps==0)
			{
				if (onDone!=null) onDone();
			}
			else
			{
				OnWebMoveDone = onDone;
				RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SA_WEB_MOVE, ref cmd);
				Console.WriteLine("{0}: WebMove, _OnWebMoveDone={1}", RxGlobals.Timer.Ticks(), _OnWebMoveDone!=null);			
			}
		}

		//--- WebStop ----------------------------------
		public void WebStop()
		{
			Console.WriteLine("{0}: WebStop", RxGlobals.Timer.Ticks());
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_WEB_STOP);
		}
	}
}
