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

		}

		//--- Property powerStepStatus ---------------------------------------
		private UInt32 _powerStepStatus;
		public UInt32 powerStepStatus
		{
			get { return _powerStepStatus; }
			set { SetProperty(ref _powerStepStatus,value); }
		}

		//--- Property motorPosition ---------------------------------------
		private int _motorPosition;
		public int motorPosition
		{
			get { return _motorPosition; }
			set { SetProperty(ref _motorPosition,value); }
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

		//--- Update ------------------------------------------------
		public void Update(TcpIp.SSetupAssistStatMsg msg)
		{
			powerStepStatus = msg.powerStepStatus;
			motorPosition	= msg.motorPosition;
			motorVoltage	= msg.motorVoltage;
			motorMoveCurrent= msg.motorMoveCurrent;
			motorHoldCurrent= msg.motorHoldCurrent;
			refDone			= msg.refDone!=0;
			moving			= msg.moving!=0;
			InLeft			= (msg.inputs&(1<<0))!=0;
			InRight			= (msg.inputs&(1<<1))!=0;
		}

		//--- Property steps ---------------------------------------
		private int _steps;
		public int steps
		{
			get { return _steps; }
			set { SetProperty(ref _steps,value); }
		}

		//--- Property speed ---------------------------------------
		private UInt32 _speed;
		public UInt32 speed
		{
			get { return _speed; }
			set { SetProperty(ref _speed,value); }
		}

		//--- Property acc ---------------------------------------
		private UInt32 _acc;
		public UInt32 acc
		{
			get { return _acc; }
			set { SetProperty(ref _acc,value); }
		}

		//--- Property current ---------------------------------------
		private UInt32 _current;
		public UInt32 current
		{
			get { return _current; }
			set { SetProperty(ref _current,value); }
		}
		
		//--- Move --------------------------------------------
		public void Move()
		{
			TcpIp.SetupAssist_MoveCmd cmd = new TcpIp.SetupAssist_MoveCmd();
			cmd.steps	= steps;
			cmd.speed	= speed;
			cmd.acc		= acc;
			cmd.current = current;
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
