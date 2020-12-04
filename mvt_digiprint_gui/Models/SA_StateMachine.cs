using rx_CamLib;
using RX_Common;
using RX_DigiPrint.Converters;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
	public class SA_StateMachine : RxBindable
	{
		private const bool		 _SimuMachine = false;
		private const bool		 _SimuCamera  = false;

		private RxCamFunctions	 _CamFunctions;
		private List<SA_Action>  _Actions;
		private int				 _ActionIdx;
		private SA_Action		 _Action;

		//--- _InitActions -----------------------------
		public List<SA_Action> Start()
		{
			int color, n;
			_CamFunctions = new RxCamFunctions(RxGlobals.Camera);
			_CamFunctions.Simulation = _SimuCamera;
			RxGlobals.Camera.CamCallBack += new RxCam.CameraCallBack(CallBackfromCam);

			_Actions = new List<SA_Action>();
			_Actions.Add(new SA_Action(){Name="Print Image" });
			_Actions.Add(new SA_Action()
			{
				Function = ECamFunction.CamFindMark,
				Name="Find Mark",
				ScanPos	= 50.0,
			});

			//--- measurmentfunctions -----------------------------
			const double pos0 = 20.0;
			const double headdist = 43.328;

			for (color=0; color<RxGlobals.PrintSystem.ColorCnt; color++)
			{
				InkType ink = RxGlobals.InkSupply.List[color*RxGlobals.PrintSystem.InkCylindersPerColor].InkType;
				if (ink!=null)
				{
					string colorName = new ColorCode_Str().Convert(ink.ColorCode, null, color*RxGlobals.PrintSystem.InkCylindersPerColor, null).ToString();
					for (n=0; n<RxGlobals.PrintSystem.HeadsPerColor-1; n++)
					{
						SA_Action action=new SA_Action()
						{
							PrintbarNo	= color,
							HeadNo		= n,
							Function	= ECamFunction.CamMeasurePosition,
							Name		= String.Format("Measure {0}-{1}..{2}",  colorName, n+1, n+2),
						//	WebMoveDist = (n==0)? 20.0 : 0,
						//	ScanPos	    = pos0+n*headdist,
							WebMoveDist = (n==0)? -150.0 : 0,
							ScanPos		= 50,
						};
						
						_Actions.Add(action);
					}
				}
			}

			_ActionIdx = 0;
			_StartAction();
			return _Actions;
		}

		//--- CallBackfromCam ----------------------------------------
		private void CallBackfromCam(RxCam.ENCamCallBackInfo CallBackInfo, string ExtraInfo = "")
        {
			if(CallBackInfo <= 0)
            {
				// Errors
				RX_Common.MvtMessageBox.Information("Camera", string.Format("Callback from Camera:\n{0}", CallBackInfo.ToString() + " " + ExtraInfo));
			}
            else
            {
				// Messages
				switch(CallBackInfo)
                {
					//Start-Lines detected
					case RxCam.ENCamCallBackInfo.StartLinesDetected:
						if (_Action?.Function==ECamFunction.CamFindMark) _OnMarkFound();
					//	RX_Common.MvtMessageBox.Information("Camera", string.Format("{0} Start-Lines detected", ExtraInfo));
						break;
                }
            }
        }

		//--- Abort ---------------------------------------
		public void Abort()
		{
			if (_Action!=null && _Action.State==ECamFunctionState.running) _Action.State = ECamFunctionState.aborted;
			_ActionIdx=_Actions.Count();
			_Action = null;
			RxGlobals.SetupAssist.ScanReference();
		}

		//--- Property Running ---------------------------------------
		public bool Running
		{
			get { return (_Action!=null); }
		}

		//--- _StartAction -----------------------------------------
		private void _StartAction()
		{
			if (_ActionIdx>=_Actions.Count()) 
			{
				_Action = null;
				return;
			}
			_Action			=_Actions[_ActionIdx];
			_Action.State	= ECamFunctionState.running;
			switch(_Action.Function)
			{
				case ECamFunction.CamNoFunction:		_StartTestPrint();	break;
				case ECamFunction.CamFindMark:			_FindMark();		break;
				case ECamFunction.CamMeasurePosition:	_Measure();			break;
			}
		}

		//--- ActionDone --------------------------------------------
		public void ActionDone()
		{
			if (_Action!=null)
			{
				_Action.State = ECamFunctionState.done;
				_Action = null;
				_ActionIdx++;
				if (_ActionIdx<_Actions.Count()) _StartAction();
				else RxGlobals.SetupAssist.ScanReference();
			}
		}
		
		public void Test()
		{
			RxCamFunctions fct = new RxCamFunctions(RxGlobals.Camera);
			fct.FindMark(_OnMarkFound);
		}

		//--- _StartTestPrint --------------------------------------
		private void _StartTestPrint()
        {
			PrintQueueItem item = new PrintQueueItem();

            if (InkSupply.AnyFlushed()) return;

			Console.WriteLine("{0}: _StartTestPrint", RxGlobals.Timer.Ticks());

			item.TestImage	= ETestImage.SA_Alignment;
			item.Dots		= "L";
			item.Speed		= 50;
			item.DistToStop = RxGlobals.Settings.SetupAssistCam.DistToStop;
			item.ScanMode	= EScanMode.scan_std;
			item.PageMargin = 0;
            item.ScanLength = item.Copies = 1;
            item.SendMsg(TcpIp.CMD_TEST_START);
			RxGlobals.SetupAssist.OnWebMoveDone=ActionDone;
			if (_SimuMachine) ActionDone();
        }

		//--- _FindMark ------------------------------------
		private void _FindMark()
		{
			if (!_SimuMachine) RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos, _ScanMoveDone);
			else _ScanMoveDone();
		}

		//--- _Camera_CamMarkFound ---------------------------------------
		private void _OnMarkFound()
		{
			Console.WriteLine("{0}: Mark Found", RxGlobals.Timer.Ticks());
			RxGlobals.SetupAssist.WebStop();
		}

		//--- _Camera_CamMarkNotFound -------------------------------------------
		private void _OnMarkNotFound()
		{
			Console.WriteLine("{0}: Mark NOT Found", RxGlobals.Timer.Ticks());
			if (_Action==null) return;
			if (_Action.Function==ECamFunction.CamFindMark) 
			{ 
				_Action.State=ECamFunctionState.error;
				Abort();
			}
		}

		//--- _Measure ----------------------------------------------
		private void _Measure()
		{
			if (_SimuMachine)
			{
				_CamFunctions.MeasurePosition(_OnPositionMeasured);
			}
			else
			{
				Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
				RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos,  _ScanMoveDone);
				RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist, _WebMoveDone);
			}
		}

		//--- _ScanMoveDone -------------------------------
		private void _ScanMoveDone()
		{
			Console.WriteLine("{1}: Action[{1}]: _ScanMoveDone", RxGlobals.Timer.Ticks(), _ActionIdx);
			if (_Action!=null) _Action.ScanMoveDone=true;
			_StartCamFunction();
		}

		//--- _WebMoveDone ---------------------------------
		private void _WebMoveDone()
		{
			Console.WriteLine("{0}: Action[{1}]: _WebMoveDone", RxGlobals.Timer.Ticks(), _ActionIdx);
			if (_Action!=null)
			{
				_Action.WebMoveDone=true;
				_StartCamFunction();
			}
		}

		//--- _StartCamFunction ----------------------------------------------
		private void _StartCamFunction()
		{
			if (_Action==null) 
				return;
			Console.WriteLine("Action[{0}]: _StartCamFunction ScanMoveDone={1} WebMoveDist={2} WebMoveDone={3}", _ActionIdx, _Action.ScanMoveDone, _Action.WebMoveDist, _Action.WebMoveDone);

			if (_Action.ScanMoveDone && (_Action.WebMoveDist==0 || _Action.WebMoveDone)) 
			{
				RxBindable.Invoke(()=>
				{ 
					switch(_Action.Function)
					{
						case ECamFunction.CamNoFunction: 
							break;

						case ECamFunction.CamFindMark: 
							_CamFunctions.FindMark(_OnMarkFound);
							RxGlobals.SetupAssist.WebMove(1000.0, _OnMarkNotFound);
							break;

						case ECamFunction.CamMeasurePosition:
							_CamFunctions.MeasurePosition(_OnPositionMeasured);
							break;

						default: 
							RxGlobals.Events.AddItem(new LogItem("Unknown Camera function")); 
							break;
					}
				});
			}
		}

		//--- _OnPositionMeasured -------------------------------------
		private void _OnPositionMeasured(SHeadPosition pos)
		{
			if (_Action==null) return;
			_Action.Measured = pos;
			ActionDone();
		}
	}
}
