using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
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

		//--- Property Simu ---------------------------------------
		private bool _Simu;
		public bool Simu
		{
			get { return _Simu; }
			set { SetProperty(ref _Simu,value); }
		}

		//--- Property powerStepStatus ---------------------------------------
		private UInt32 _powerStepStatus;
		public UInt32 powerStepStatus
		{
			get { return _powerStepStatus; }
			set { SetProperty(ref _powerStepStatus,value); }
		}

		//--- Property ScanPos ---------------------------------------
		private double _ScanPos;
		public double ScanPos
		{
			get { return _ScanPos; }
			set { SetProperty(ref _ScanPos, value); }
		}

		//--- Property ScanStopPos ---------------------------------------
		private double _ScanStopPos;
		public double ScanStopPos
		{
			get { return _ScanStopPos; }
			set { SetProperty(ref _ScanStopPos,value); }
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
		//--- Property ScanRefDone ---------------------------------------
		private bool _ScanRefDone;
		public bool ScanRefDone
		{
			get { return _ScanRefDone; }
			set { SetProperty(ref _ScanRefDone,value); }
		}

		//--- Property ScanMoving ---------------------------------------
		private bool _ScanMoving;
		public bool ScanMoving
		{
			get { return _ScanMoving; }
			set { SetProperty(ref _ScanMoving, value); }				
		}

		//--- Property WebMoving ---------------------------------------
		private bool _WebMoving;
		public bool WebMoving
		{
			get { return _WebMoving; }
			set { SetProperty(ref _WebMoving,value); }
		}

		//--- Property ScanMoveCnt ---------------------------------------
		private int _ScanMoveCnt;
		public int ScanMoveCnt
		{
			get { return _ScanMoveCnt; }
			set 
			{ 
				if (SetProperty(ref _ScanMoveCnt,value) )
				{
					Console.WriteLine("{0} SCAN MOVE DONE[{1}] motorPosition={2}", RxGlobals.Timer.Ticks(), _ScanMoveCnt, ScanPos);
					_OnScanMoveDone();
				}
			}
		}

		//--- Property ScanInLeft ---------------------------------------
		private bool _ScanInLeft;
		public bool ScanInLeft
		{
			get { return _ScanInLeft; }
			set { SetProperty(ref _ScanInLeft,value); }
		}

		//--- Property ScanInRight ---------------------------------------
		private bool _ScanInRight;
		public bool ScanInRight
		{
			get { return _ScanInRight; }
			set { SetProperty(ref _ScanInRight,value); }
		}

		//--- Property Connected ---------------------------------------
		private int _ConnectedTimer;
		private bool _Connected=false;
		public bool ScanConnected
		{
			get { return _Connected; }
			set { 
					SetProperty(ref _Connected, value); 
					if (_Connected) _ConnectedTimer=5;
				}
		}
		private void _Tick(int no)
		{
			if (_ConnectedTimer>0 && --_ConnectedTimer==0) ScanConnected=false;
			_checkWebMoveDone();
		}

		//--- Update ------------------------------------------------
		public void Update(TcpIp.SSetupAssistStatMsg msg)
		{
			powerStepStatus = msg.powerStepStatus;
			ScanPos	= Math.Round(msg.motorPosition/1000.0, 1);
			ScanStopPos		    = Math.Round(msg.stopPos/1000.0, 1);
			motorVoltage	= msg.motorVoltage;
			motorMoveCurrent= msg.motorMoveCurrent;
			motorHoldCurrent= msg.motorHoldCurrent;
			ScanMoveCnt		= msg.moveCnt;
			ScanRefDone			= msg.refDone!=0;
			ScanMoving			= msg.moving!=0;
			ScanInLeft			= (msg.inputs&(1<<0))!=0;
			ScanInRight			= (msg.inputs&(1<<1))!=0;
			ScanConnected		= true;
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
			if (false && _Simu) _OnScanMoveDone();
			else       RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_REFERENCE);
		}

		//--- ScanMoveTo --------------------------------------------
		public void ScanMoveTo(double pos, int speed=0)
		{
			TcpIp.SetupAssist_MoveCmd cmd = new TcpIp.SetupAssist_MoveCmd();
			cmd.steps	= (Int32)(pos*1000.0);
			cmd.speed   = speed;
			if (speed>0 && speed<20) cmd.acc = 20;
			else cmd.acc=0;
		//	Console.WriteLine("{0} SCAN MOVE TO [{1}] from {2:N3} to {3:N3}, speed={4}, acc={5}", RxGlobals.Timer.Ticks(), _ScanMoveCnt, ScanPos, pos, cmd.speed, cmd.acc);
			if (_Simu) _OnScanMoveDone();
			else RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SA_MOVE, ref cmd);
		}

		//--- ScanStop ------------------------------
		public void ScanStop()
		{
			if (_Simu) _OnScanMoveDone();
			else	   RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_STOP);
		}

		//--- Property WebPos ---------------------------------------
		private double _WebPos=0;
		public double WebPos
		{
			get { return _WebPos; }
			set { SetProperty(ref _WebPos,value); }
		}
		public void WebPos_Reset()
		{
			Console.WriteLine("WebPos_Reset");
			WebPos = 0;
		}

		//--- WebMove ----------------------------------
		public const int WebSpeed = 1;	// [m/min]
		private int _WebMoveStartCnt=-1;
		public void WebMove(double dist)
		{
			if (_Simu) _OnWebMoveDone();
			else
			{
				_checkWebMoveDone();

				TcpIp.SetupAssist_MoveCmd cmd = new TcpIp.SetupAssist_MoveCmd();
				double old=WebPos;
				WebPos	   += (double)dist;
				cmd.steps	= (Int32)(1000*dist);
				cmd.speed = WebSpeed;
				if (cmd.steps==0)
				{
			//		if (_OnWebMoveDone!=null) _OnWebMoveDone();
				}
				else
				{
					EnPlcState state = (EnPlcState)Rx.StrToInt32(RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_MACHINE_STATE"));
					if (_WebMoveStartCnt<0) _WebMoveStartCnt=_WebMoveCnt;
					string msg=string.Format("WEB MOVE start={0} done={1} PlcState={2} dist={3} oldpos={4} WebPos={5}", _WebMoveStartCnt, _WebMoveCnt, state.ToString(), cmd.steps, old, WebPos);
					if (_WebMoveCnt!=_WebMoveStartCnt)
						Console.WriteLine("WEB MOVE Error: _WebMoveCnt={0}, _WebMoveStartCnt={1}", _WebMoveCnt, _WebMoveStartCnt);
					_WebMoveStartCnt++;
					WebMoving=true;
					Console.WriteLine("{0}: {1}", RxGlobals.Timer.Ticks(), msg);
				//	RxGlobals.Events.AddItem(new LogItem(msg));
					RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SA_WEB_MOVE, ref cmd);
				}
			}
		}

		//--- WebStop ----------------------------------
		public void WebStop()
		{
			Console.WriteLine("{0}: WEB STOP", RxGlobals.Timer.Ticks());
			if (_Simu)	_OnWebMoveDone();
			else		RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_WEB_STOP);
		}
		
		//--- _checkWebMoveDone -----------------------------------------------
		private int _WebMoveCnt=0;

		private void _checkWebMoveDone()
		{
			RxGlobals.Plc.RequestVar("Application.GUI_00_001_Main" + "\n" + "STA_RELATIVE_MOVE_CNT" + "\n" + "STA_MACHINE_STATE" + "\n");
			int old=_WebMoveCnt;
			_WebMoveCnt=Rx.StrToInt32(RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_RELATIVE_MOVE_CNT"));
						
			if (_OnWebMoveDone!=null)
			{
				if (_WebMoveCnt!=old)
				{
					Console.WriteLine("{0}: WEB MOVE DONE {1} (new={2})", RxGlobals.Timer.Ticks(), old, _WebMoveCnt);
					WebMoving=false;
					_OnWebMoveDone();
				}
			}
		}

		//--- Property IsRunning ---------------------------------------
		private bool _IsRunning=false;
		public bool IsRunning
		{
			get { return _IsRunning; }
			set { SetProperty(ref _IsRunning,value); }
		}


	}
}
