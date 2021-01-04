using rx_CamLib;
using RX_Common;
using RX_DigiPrint.Converters;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using static rx_CamLib.RxCam;

namespace RX_DigiPrint.Models
{
	public class SA_StateMachine : RxBindable
	{
		private const bool		 _Debug=true;

		private const bool		 _SimuMachine = false;
		private const bool		 _SimuCamera  = false;

		private RxCamFunctions	 _CamFunctions;
		private List<SA_Action>  _Actions;
		private int				 _ActionIdx;
		private SA_Action		 _Action;
		private int				 _Time=0;
		private RxCam.CallBackDataStruct	_CallbackData;

		public SA_StateMachine()
		{
			RxGlobals.Camera.CamCallBack += new RxCam.CameraCallBack(CallBackfromCam);
			RxGlobals.Timer.TimerFct += Tick;
			RxGlobals.SetupAssist.OnWebMoveDone  = _WebMoveDone;
			RxGlobals.SetupAssist.OnScanMoveDone = _ScanMoveDone; 
			_CamFunctions = new RxCamFunctions(RxGlobals.Camera);
			_CamFunctions.Simulation = _SimuCamera;
		}

		//--- _InitActions -----------------------------
		public List<SA_Action> Start()
		{
			int color, n;

			_Actions = new List<SA_Action>();
			_Actions.Add(new SA_Action(){Name="Print Image" });
			_Actions.Add(new SA_Action()
			{
				Function = ECamFunction.CamFindMark_1,
				Name="Find Mark",
				ScanPos	= 50.0,
			});

			_Actions.Add(new SA_Action()
			{
				Function = ECamFunction.CamFindMark_2,
				Name="Find Mark 2",
				WebMoveDist = -190.0,
				ScanPos	    = 50.0,
			});

			_Actions.Add(new SA_Action()
			{
				Function = ECamFunction.CamFindMark_3,
				Name="Find Mark 3",
				WebMoveDist = 0,
				ScanPos	    = 0,
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
							Function	= ECamFunction.CamMeasureAngle,
							Name		= String.Format("Measure {0}-{1}..{2}",  colorName, n+1, n+2),
						//	WebMoveDist = (n==0)? 20.0 : 0,
						//	ScanPos	    = pos0+n*headdist,
							WebMoveDist = (n==0)? 5.0 : 0,
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
		private int _CallbackNo=0;
		private void CallBackfromCam(RxCam.ENCamCallBackInfo CallBackInfo, RxCam.CallBackDataStruct CallBackData)
        {
			if(CallBackInfo <= 0)
            {
				// Errors
				RX_Common.MvtMessageBox.Information("Camera", string.Format("Callback from Camera:\n{0}", CallBackInfo.ToString() + " " + CallBackData.CamResult.ToString()));
			}
            else
            {
				// Messages
				switch(CallBackInfo)
                {
					//Start-Lines detected
					case RxCam.ENCamCallBackInfo.StartLinesDetected:
						if (_Action?.Function==ECamFunction.CamFindMark_1) _OnMarkFound();
						//	RX_Common.MvtMessageBox.Information("Camera", string.Format("{0} Start-Lines detected", ExtraInfo));
						break;

					case RxCam.ENCamCallBackInfo.StartLinesContinuous:
						if (CallBackData.CamResult==RxCam.ENCamResult.OK)
						{
							string info=string.Format("# {0}\n", ++_CallbackNo) 
									   + "StartLinesContinuous\n"
									   + string.Format("  Center position: X={0}, y={1}\n", CallBackData.DPosX, CallBackData.DPosY)
									   + string.Format("  Lines detected: {0} ", CallBackData.Value_1)
									   + string.Format("  Scanner.StopPos: {0} ", RxGlobals.SetupAssist.stopPos);

							switch(CallBackData.LineLayout)
							{
								case LineLayoutEnum.LineLayout_Covering:	info += "all"; break;
								case LineLayoutEnum.LineLayout_FromTop:		info += "top"; break;
								case LineLayoutEnum.LineLayout_FromBot:		info += "bottom"; break;
								case LineLayoutEnum.LineLayout_FromRight:	info += "right"; break;
								case LineLayoutEnum.LineLayout_FromLeft:	info += "left"; break;
								default: break;
							}
							switch(_Action?.Function)
							{
								case ECamFunction.CamFindMark_2:	if (!_MarkFound && CallBackData.LineLayout==LineLayoutEnum.LineLayout_Covering)
																	{
																		Console.WriteLine("CamFindMark_2: Mark found, dy={0}", CallBackData.DPosY);
																		_CallbackData = CallBackData;
																		_OnMarkFound();
																	}
																	break;

								case ECamFunction.CamFindMark_3:	info += string.Format("\nMarkFound={0} StopPos={1}", _MarkFound, RxGlobals.SetupAssist.stopPos);
																	if (CallBackData.LineLayout==LineLayoutEnum.LineLayout_FromRight)
																	{		
																		if (!_MarkFound)
																		{ 
																			_MarkFound=true;
																			RxGlobals.SetupAssist.ScanStop();
																			_CallbackData = CallBackData;
																		}
																	}
																	break;

							}
							CallbackInfo = info;
						}
						break;
					//Angle Correction
					case RxCam.ENCamCallBackInfo.AngleCorr:
						RX_Common.MvtMessageBox.Information("Camera", string.Format("Angle Correction {0}", CallBackData.Value_1));
						break;
					//Stitch Correction
					case RxCam.ENCamCallBackInfo.StitchCorr:
						RX_Common.MvtMessageBox.Information("Camera", string.Format("Stitch Correction {0}", CallBackData.Value_1));
						break;
					//Stitch Correction
					case RxCam.ENCamCallBackInfo.RegisterCorr:
						RX_Common.MvtMessageBox.Information("Camera", string.Format("Register Correction {0}", CallBackData.Value_1));
						break;
				}
			}
        }

		//--- Property CallbackInfo ---------------------------------------
		private string _CallbackInfo;
		public string CallbackInfo
		{
			get { return _CallbackInfo; }
			set { SetProperty(ref _CallbackInfo,value); }
		}

		//--- Abort ---------------------------------------
		public void Abort()
		{
			if (_Action!=null && _Action.State==ECamFunctionState.running) _Action.State = ECamFunctionState.aborted;
			_ActionIdx=_Actions.Count();
			_Action = null;
			RxGlobals.SetupAssist.ScanReference();
			RxGlobals.RxInterface.SendCommand(TcpIp.CMD_STOP_PRINTING);
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
			if (_Debug) CanContinue = false;
			_Action			=_Actions[_ActionIdx];
			_Action.State	= ECamFunctionState.running;
			switch(_Action.Function)
			{
				case ECamFunction.CamNoFunction:		_StartTestPrint();	break;
				case ECamFunction.CamFindMark_1:		_FindMark_1();		break;
				case ECamFunction.CamFindMark_2:		_FindMark_2();		break;
				case ECamFunction.CamFindMark_3:		_FindMark_3();		break;
				case ECamFunction.CamMeasureAngle:		_MeasureAngle();	break;
			}
		}

		//--- Property CanContinue ---------------------------------------
		private bool _CanContinue;
		public bool CanContinue
		{
			get { return _CanContinue; }
			set { SetProperty(ref _CanContinue,value); }
		}

		//--- Continue --------------------------------
		public void Continue()
		{
			ActionDone(true);
		}

		//--- ActionDone --------------------------------------------
		public void ActionDone(bool cont=false)
		{
			if (_Action!=null)
			{
				_Action.State = ECamFunctionState.done;
				if (_Debug) CanContinue = true;
				if (!_Debug || cont || _ActionIdx<3)
				{
					_Action = null;
					_ActionIdx++;
					if (_ActionIdx<_Actions.Count()) _StartAction();
					else RxGlobals.SetupAssist.ScanReference();
				}
			}
		}
		
		public void Test()
		{
			if (_Action==null)
			{
				_Action = new SA_Action()
					{
						Function = ECamFunction.CamFindMark_3,
						Name="Find Mark 3",
						WebMoveDist = 0,
						ScanPos	    = 0,
					};
			}
			_FindMark_3();
		}

		//--- _StartTestPrint --------------------------------------
		private void _StartTestPrint()
        {
			PrintQueueItem item = new PrintQueueItem();

            if (InkSupply.AnyFlushed()) return;
			RxBindable.Invoke(()=>_CamFunctions.Off());
			Console.WriteLine("{0}: _StartTestPrint", RxGlobals.Timer.Ticks());
			item.TestImage	= ETestImage.SA_Alignment;
			item.Dots		= "L";
			item.Speed		= 50;
			item.DistToStop = RxGlobals.Settings.SetupAssistCam.DistToStop;
			item.ScanMode	= EScanMode.scan_std;
			item.PageMargin = 0;
            item.ScanLength = item.Copies = 1;
            item.SendMsg(TcpIp.CMD_TEST_START);
			if (_SimuMachine) ActionDone();
			_CamFunctions.Off();
        }

		//--- _FindMark_1 ------------------------------------
		private bool _MarkFound=false;
		private void _FindMark_1()
		{
			RxGlobals.Events.AddItem(new LogItem("Camera: Finding Mark"));
			Console.WriteLine("_FindMark_1: _MarkFound = false");
			_MarkFound = false;
			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
		}

		//--- _FindMark_2 ----------------------------------------------
		private void _FindMark_2()
		{
			_MarkFound=false;
			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			Console.WriteLine("{0}: _FindMark_2, MarkFound={1}", RxGlobals.Timer.Ticks(), _MarkFound);
			_Time=2;
			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
			RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
		}

		//--- _FindMark_3 -----------------------
		private void _FindMark_3()
		{
			Console.WriteLine("{0}: Action[{1}]: _FindMark_3, ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			RxGlobals.SetupAssist.WebMove(-_CallbackData.DPosY/1000);
		}

		//--- _Camera_CamMarkFound ---------------------------------------
		private void _OnMarkFound()
		{
			if (!_MarkFound)
			{
				_MarkFound=true;
				if (_Action.Function== ECamFunction.CamFindMark_1) 
				{
					RxGlobals.SetupAssist.WebStop();
					RxGlobals.Events.AddItem(new LogItem("Camera: WebStop"));
				}
				RxGlobals.Events.AddItem(new LogItem("Camera: MARK FOUND"));
				Console.WriteLine("{0}:  Action[{1}]: Mark Found", RxGlobals.Timer.Ticks(), _ActionIdx);
			}
		}

		//--- _MeasureAngle ----------------------------------------------
		private void _MeasureAngle()
		{
			RxGlobals.Events.AddItem(new LogItem("_MeasureAngle: stopPos={0}", RxGlobals.SetupAssist.stopPos));

			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
			RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
		}

		//--- _ScanMoveDone -------------------------------
		private void _ScanMoveDone()
		{
			Console.WriteLine("{0}: Action[{1}]: _ScanMoveDone", RxGlobals.Timer.Ticks(), _ActionIdx);
			if (_Action==null) return;
			_Action.ScanMoveDone=true;
			if (_Action.Function==ECamFunction.CamFindMark_3)
			{
				RxBindable.Invoke(()=>_CamFunctions.Off());
				if (_MarkFound)
				{
					Console.WriteLine("{0}: CamFindMark_3: _ScanMoveDone motorPos={1}, stopPos={2}", RxGlobals.Timer.Ticks(), RxGlobals.SetupAssist.motorPosition, RxGlobals.SetupAssist.stopPos);
					if (_Action.State==ECamFunctionState.running)
					{
						_Actions[_ActionIdx+1].ScanPos     = RxGlobals.SetupAssist.motorPosition;
						_Actions[_ActionIdx+1].WebMoveDist = 12.0;
						ActionDone();
					}
					else if (_Action.State==ECamFunctionState.ending) ActionDone();				
				}
				else
				{
					RxGlobals.Events.AddItem(new LogItem("Camera: Mark NOT Found"));
					Console.WriteLine("{0}: Mark NOT Found", RxGlobals.Timer.Ticks());
					_Action.State=ECamFunctionState.error;
					Abort();
				}
				return;
			}
			Console.WriteLine("{0}: Action[{1}]: _ScanMoveDone: WebMoveDist={2}, WebMoveDone={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.WebMoveDist, _Action.WebMoveDone);
			if (_Action.WebMoveDist==0 || _Action.WebMoveDone) _StartCamFunction();
		}
		
		private void _WebMoveDone()
		{
			Console.WriteLine("{0}: Action[{1}]: _WebMoveDone", RxGlobals.Timer.Ticks(), _ActionIdx);
			if (_Action!=null)
			{
				_Action.WebMoveDone=true;
				switch (_Action.Function)
				{
					case ECamFunction.CamNoFunction:	ActionDone();
														break;

					case ECamFunction.CamFindMark_1:	if (_MarkFound) ActionDone();
														else
														{
															RxGlobals.Events.AddItem(new LogItem("Camera: Mark NOT Found"));
															Console.WriteLine("{0}: Mark NOT Found", RxGlobals.Timer.Ticks());
															_Action.State=ECamFunctionState.error;
															Abort();
														}
														break;

					case ECamFunction.CamFindMark_3:	RxBindable.Invoke(()=>
															{
																RxGlobals.Events.AddItem(new LogItem("Camera: Finding Mark 3"));
																_CamFunctions.Off();
																Console.WriteLine("_FindMark_3: _MarkFound = false");
																_MarkFound=false;
																_Time=2;
																_CamFunctions.FindMark(true);
																RxGlobals.SetupAssist.ScanMoveTo(0, 10);
															}
														);
														break;

					default: _StartCamFunction(); break;
				}
			}
		}

		//--- _StartCamFunction ----------------------------------------------
		private void _StartCamFunction()
		{
			if (_Action==null) 
				return;

			Console.WriteLine("Action[{0}]: _StartCamFunction={4} ScanMoveDone={1} WebMoveDist={2} WebMoveDone={3}", _ActionIdx, _Action.ScanMoveDone, _Action.WebMoveDist, _Action.WebMoveDone, _Action.Function);
			
			if (_Action.ScanMoveDone && (_Action.WebMoveDist==0 || _Action.WebMoveDone)) 
			{
				RxBindable.Invoke(()=>
				{ 
					switch(_Action.Function)
					{
						case ECamFunction.CamNoFunction:
							break;

						case ECamFunction.CamFindMark_1:							
							_CamFunctions.FindMark(false);
							RxGlobals.SetupAssist.WebMove(1000.0);
							break;

						case ECamFunction.CamFindMark_2:
							_CamFunctions.Off();
							_MarkFound=false;
							_Time=2;
							_CamFunctions.FindMark(true);
							break;

						case ECamFunction.CamMeasureAngle:
							RxBindable.Invoke(()=>_CamFunctions.MeasureAngle());
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

		//--- Tick ---------------------------------------
		private void Tick(int no)
		{
			if (_Action!=null)
			{
				if (_Action.Function==ECamFunction.CamFindMark_2)
				{
					Console.WriteLine("Time={0} MarkFound={1}", _Time, _MarkFound);
					if (_Time>0 && --_Time==0)
					{
						if (_MarkFound)
						{
							ActionDone();
						}
						else 
							RxGlobals.SetupAssist.WebMove(1);
					}
				}

			}
		}

	}
}
