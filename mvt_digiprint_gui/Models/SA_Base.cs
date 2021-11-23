using RX_Common;
using RX_DigiPrint.Services;
using System;

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

		//--- Up -------------------------------------------
		public void Up()
		{
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_UP);
		}

		//--- Down -------------------------------------------
		public void Down()
		{
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_SA_DOWN);
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
		public void Update(TcpIp.SStepperStat msg)
		{
			ScanPos				= Math.Round(msg.posX/1000.0, 1);
			ScanStopPos		    = Math.Round(msg.posX/1000.0, 1);
			ScanMoveCnt			= msg.posY[0];
			ScanRefDone			= (msg.info & 0x00000001)!=0;
			ScanMoving			= (msg.info & 0x00000002)!=0;
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
			Console.WriteLine("{0} SCAN MOVE [{1}] FROM {2} TO {3}", RxGlobals.Timer.Ticks(), _ScanMoveCnt+1, ScanPos, pos);
			cmd.pos = (int)(1000.0*pos);
			cmd.speed = speed;
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

		//--- WebMoveInit ------------------------
		public void WebMoveInit()
		{
			_WebMoveStartCnt = _WebMoveCnt;
		}

		//--- WebMove ----------------------------------
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
				cmd.pos	= (Int32)(1000*dist);
				EnPlcState state = (EnPlcState)Rx.StrToInt32(RxGlobals.Plc.GetVar("Application.GUI_00_001_Main", "STA_MACHINE_STATE"));
				string msg=string.Format("WEB MOVE start={0} done={1} PlcState={2} dist={3} oldpos={4} WebPos={5}", _WebMoveStartCnt, _WebMoveCnt, state.ToString(), cmd.pos, old, WebPos);
				if (_WebMoveCnt!=_WebMoveStartCnt)
					Console.WriteLine("WEB MOVE Error: _WebMoveCnt={0}, _WebMoveStartCnt={1}", _WebMoveCnt, _WebMoveStartCnt);
				_WebMoveStartCnt++;
				WebMoving =true;
				Console.WriteLine("{0}: {1}", RxGlobals.Timer.Ticks(), msg);
			//	RxGlobals.Events.AddItem(new LogItem(msg));
				RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SA_WEB_MOVE, ref cmd);
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
					Console.WriteLine("{0}: WEB MOVE DONE {1} (next={2})", RxGlobals.Timer.Ticks(), old, _WebMoveCnt);
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
