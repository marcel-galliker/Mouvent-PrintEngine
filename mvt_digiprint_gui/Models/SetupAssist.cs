using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
	public class SetupAssist : RxBindable
	{
		public SetupAssist()
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
			set { SetProperty(ref _moving,value); }
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
		}

		//--- Update ------------------------------------------------
		public void Update(TcpIp.SSetupAssistStatMsg msg)
		{
			powerStepStatus = msg.powerStepStatus;
			motorPosition	= Math.Round(msg.motorPosition/1000.0, 1);
			motorVoltage	= msg.motorVoltage;
			motorMoveCurrent= msg.motorMoveCurrent;
			motorHoldCurrent= msg.motorHoldCurrent;
			refDone			= msg.refDone!=0;
			moving			= msg.moving!=0;
			InLeft			= (msg.inputs&(1<<0))!=0;
			InRight			= (msg.inputs&(1<<1))!=0;
			Connected		= true;
		}
		
		//--- Move --------------------------------------------
		public void Move(double dist)
		{
			TcpIp.SetupAssist_MoveCmd cmd = new TcpIp.SetupAssist_MoveCmd();
			cmd.steps	= (Int32)(dist*1000.0);
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SA_MOVE, ref cmd);
		}

		//--- Property Dist ---------------------------------------
		private double _WebDist;
		public double WebDist
		{
			get { return _WebDist; }
			set { SetProperty(ref _WebDist,value); }
		}

		//--- WebMove ----------------------------------
		public void WebMove()
		{
			TcpIp.SetupAssist_MoveCmd cmd = new TcpIp.SetupAssist_MoveCmd();
			cmd.steps	= (Int32)(1000*WebDist);
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SA_WEB_MOVE, ref cmd);
		}
	}
}
