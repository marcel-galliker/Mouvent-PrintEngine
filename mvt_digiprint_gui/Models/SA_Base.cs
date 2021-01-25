using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
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
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_REFERENCE);
		}

		//--- ScanMoveTo --------------------------------------------
		public void ScanMoveTo(double pos, int speed=0)
		{
			TcpIp.SetupAssist_MoveCmd cmd = new TcpIp.SetupAssist_MoveCmd();
			cmd.steps	= (Int32)(pos*1000.0);
			cmd.speed   = speed;
			Console.WriteLine("{0} SCAN MOVE TO [{1}] from {2:N3} to {3:N3}", RxGlobals.Timer.Ticks(), _ScanMoveCnt, ScanPos, pos);
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
		public const int WebSpeed = 5;	// [m/min]
		public void WebMove(double? dist=null)
		{
			_checkWebMoveDone();
			/*
			int time=0;
			do
			{
				RxGlobals.Plc.RequestVar("Application.GUI_00_001_Main" + "\n"
                + "STA_MACHINE_STATE" + "\n");

				EnPlcState state = (EnPlcState)Rx.StrToInt32(RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_MACHINE_STATE"));
				if (state==EnPlcState.plc_pause) break;
				if (++time>100) 
				{
					Console.WriteLine("WebMove Timeout");
					return;
				}
				Console.WriteLine("WebMove wait, state={0}, time={1}", state, time);
				Thread.Sleep(100);
			}
			while(true);
			*/

			TcpIp.SetupAssist_MoveCmd cmd = new TcpIp.SetupAssist_MoveCmd();
			if (dist==null) cmd.steps	= (Int32)(1000*WebDist);
			else            cmd.steps	= (Int32)(1000*dist);
			cmd.speed = WebSpeed;	
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
			/*
			int time=0;
			do
			{
				RxGlobals.Plc.RequestVar("Application.GUI_00_001_Main" + "\n"
                + "STA_MACHINE_STATE" + "\n");

				EnPlcState state = (EnPlcState)Rx.StrToInt32(RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_MACHINE_STATE"));
				if (state!=EnPlcState.plc_run) break;
				if (++time>100) 
				{
					Console.WriteLine("WebStop Timeout");
					return;
				}
				Console.WriteLine("WebStop wait, state={0}, time={1}", state, time);
				Thread.Sleep(100);
			}
			while(true);
			*/
		}
		
		//--- _checkWebMoveDone -----------------------------------------------
		private int _WebMoveCnt=0;
		private void _checkWebMoveDone()
		{
			RxGlobals.Plc.RequestVar("Application.GUI_00_001_Main" + "\n" + "STA_RELATIVE_MOVE_CNT" +"\n");
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

		//--- Property IsRunning ---------------------------------------
		private bool _IsRunning=false;
		public bool IsRunning
		{
			get { return _IsRunning; }
			set { SetProperty(ref _IsRunning,value); }
		}


	}
}
