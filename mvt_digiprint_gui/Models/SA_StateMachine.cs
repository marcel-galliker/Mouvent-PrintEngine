﻿using rx_CamLib;
using RX_Common;
using RX_DigiPrint.Converters;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using static rx_CamLib.RxCam;

namespace RX_DigiPrint.Models
{
	public class SA_StateMachine : RxBindable
	{
		private const bool		 _Debug=true;

		public const bool		 _SimuCamera  = false;
		public const bool		 _SimuMachine = false;

		private RxCamFunctions	 _CamFunctions;
		private List<SA_Action>  _Actions;
		private int				 _ActionIdx;
		private SA_Action		 _Action;
		private int				 _StitchIdx;
		private int				 _HeadNo;
		private const double	 _AngleDist = (128.0*25.4)/1200;
		private const double	 _StitchWebDist = (140.0*25.4)/1200;
		private RxCam.CallBackDataStruct	_CallbackData;
		private int				 _StopTime;
		private bool[]			 _RobotRunning= new bool[4];

		private int				_HeadsPerColor=4;

		public SA_StateMachine()
		{
			RxGlobals.Camera.CamCallBack += new RxCam.CameraCallBack(CallBackfromCam);
			RxGlobals.SetupAssist.OnWebMoveDone  = _WebMoveDone;
			RxGlobals.SetupAssist.OnScanMoveDone = _ScanMoveDone;
			foreach (var state in RxGlobals.StepperStatus)
			{
				state.PropertyChanged += SA_StateMachine_PropertyChanged; 			
			}				
			_CamFunctions = new RxCamFunctions(RxGlobals.Camera);
			if (_SimuCamera) _CamFunctions.SimuCallback += new RxCam.CameraCallBack(CallBackfromCam);
			_CamFunctions.Simulation = _SimuCamera;
		}

		private void SA_StateMachine_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
		{
			int no;
			for (no=0; no<RxGlobals.StepperStatus.Length; no++)
			{
				if (sender==RxGlobals.StepperStatus[no])
				{
					Console.WriteLine("Stepper[{0}].{1}={2}", no, e.PropertyName, sender.GetType().GetProperty(e.PropertyName).GetValue(sender, null));
					if (e.PropertyName.Equals("Screwed") || e.PropertyName.Equals("ScrewBlocked"))
					{
						if ((bool)sender.GetType().GetProperty(e.PropertyName).GetValue(sender, null))
						{
							StepperStatus stepper = RxGlobals.StepperStatus[no];
							RxGlobals.Events.AddItem(new LogItem(string.Format("ROB PropertyChanged screwed={0}, blocked={1} ", stepper.Screwed, stepper.ScrewBlocked)));
							if (_Actions==null) return;
							foreach(SA_Action action in _Actions)
							{	
								if (action.StepperNo==no && action.State==ECamFunctionState.runningRob)
								{
									_RobotRunning[no]=false;
									RxGlobals.Events.AddItem(new LogItem(string.Format("ROB END Screwed={0}, blocked={1} ", stepper.Screwed, stepper.ScrewBlocked)));
									if (stepper.Screwed)			action.State = ECamFunctionState.done;
									else if (stepper.ScrewBlocked)  action.State = ECamFunctionState.error;
									break;
								}
							}

							_NextRobotCmd(no);
						}
					}
					return;
				}
			}
		}

		//--- _NextRobotCmd ---------------------------------------
		private void _NextRobotCmd(int stepperNo)
		{
			if (_Actions==null) return;
			if (_RobotRunning[stepperNo]) return;

			StepperStatus stepper = RxGlobals.StepperStatus[stepperNo];

			foreach(SA_Action action in _Actions)
			{	
				if (action.StepperNo==stepperNo && action.State==ECamFunctionState.waitRob)
				{
					TcpIp.SHeadAdjustmentMsg msg = new  TcpIp.SHeadAdjustmentMsg();

					_RobotRunning[stepperNo] = true;

					msg.printbarNo  = _Action.PrintbarNo;
					msg.headNo      = _Action.HeadNo;
					if      (action.Function==ECamFunction.CamMeasureAngle)  msg.axis = 0;
					else if (action.Function==ECamFunction.CamMeasureStitch) msg.axis = 1;
					msg.steps       = (Int32)(action.Correction * 6.0 + 0.5);
					action.State	= ECamFunctionState.runningRob;
					RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_ADJUST, ref msg);

					RxGlobals.Events.AddItem(new LogItem(string.Format("ROB Command, Head={0}, steps={1}", msg.headNo, msg.steps)));

					return;
				}
			}
		}

		//--- Property SimuMachine ---------------------------------------
		public bool SimuMachine
		{
			get { return _SimuMachine; }
		}

		//--- _InitActions -----------------------------
		public List<SA_Action> Start()
		{
			int color, n;

			_Actions = new List<SA_Action>();
			_Actions.Add(new SA_Action(){Name="Print Image"});

			_Actions.Add(new SA_Action()
			{
				Function = ECamFunction.CamFindMark_1,
				Name="Find 3 Vert Lines",
				ScanPos	= 30.0,
			});

			_Actions.Add(new SA_Action()
			{
				Function = ECamFunction.CamFindMark_2,
				Name="Find Horiz Line",
				WebMoveDist = -20.0,
				ScanPos	    = 30.0,
			});

			_Actions.Add(new SA_Action()
			{
				Function = ECamFunction.CamFindMark_3,
				Name="Find Line End",
				WebMoveDist = 0,
				ScanPos	    = 30.0,
			});

			_Actions.Add(new SA_Action()
			{
				Function = ECamFunction.CamConfirmFocus,
				Name="Confirm",
				WebMoveDist = 12.0,
			});

			//--- measurmentfunctions -----------------------------
			for (color=0; color<RxGlobals.PrintSystem.ColorCnt; color++)
			{
				InkType ink = RxGlobals.InkSupply.List[color*RxGlobals.PrintSystem.InkCylindersPerColor].InkType;
				if (ink!=null)
				{
				//	_HeadsPerColor = RxGlobals.PrintSystem.HeadsPerColor;
					string colorName = new ColorCode_Str().Convert(ink.ColorCode, null, color*RxGlobals.PrintSystem.InkCylindersPerColor, null).ToString();
					for (n=0; n<_HeadsPerColor; n++)
					{
						SA_Action action=new SA_Action()
						{
							PrintbarNo	= color,
							StepperNo   = color/2,
							HeadNo		= n,
							Function	= ECamFunction.CamMeasureAngle,
							Name		= String.Format("Angle {0}-{1}",  colorName, n+1),
						};						
						_Actions.Add(action);
					}
					_StitchIdx = _Actions.Count();
					for (n=0; n<_HeadsPerColor-1; n++)
					{
						SA_Action action=new SA_Action()
						{
							PrintbarNo	= color,
							StepperNo   = color/2,
							HeadNo		= n,
							Function	= ECamFunction.CamMeasureStitch,
							Name		= String.Format("Stitch {0}-{1}..{2}",  colorName, n+1, n+2),
							WebMoveDist	= n==0? 10.5 : 0,
						};						
						_Actions.Add(action);
					}
				}
			}

			_ActionIdx = 0;
			_StartAction();
			return _Actions;
		}

		//--- ConfirmPosAndFocus -------------------------------------
		public void ConfirmPosAndFocus()
		{
			if (_Action.Function==ECamFunction.CamConfirmFocus)
			{
				_Action.ConfirmVisibile = false;
				ActionDone();
			}
		}

		//--- CallBackfromCam ----------------------------------------
		private int _CallbackNo=0;
		private static Mutex callbackMutex = new Mutex();

		private void CallBackfromCam(RxCam.ENCamCallBackInfo CallBackInfo, RxCam.CallBackDataStruct CallBackData)
        {
			bool handled=false;

			callbackMutex.WaitOne();

			if (_Action!=null)
			{
				Console.WriteLine("CALLBACK: info={0} Action[{1}].function={2}, cnt={3}, NumMeasures={4}, value={5}", CallBackInfo.ToString(), _ActionIdx, _Action.Function.ToString(), _Action.MeasureCnt, CallBackData.NumMeasures, CallBackData.Value_1);
				if (CallBackData.DPosX.Equals(float.NaN)) CallBackData.DPosX=0;
				if (CallBackData.DPosY.Equals(float.NaN)) CallBackData.DPosY=0;
				switch(_Action.Function)
				{
					case ECamFunction.CamFindMark_1:
						if (CallBackInfo== RxCam.ENCamCallBackInfo.StartLinesDetected) 
						{
							if (!_MarkFound)
							{
								_MarkFound=true;
								_CamFunctions.Off();
								if (_SimuMachine) _WebMoveDone();
								else              RxGlobals.SetupAssist.WebStop();
								_StopTime = RxTimer.GetTicks();
								RxGlobals.Events.AddItem(new LogItem("Camera: WebStop"));
							}
							handled=true;
						}
						break;

					case ECamFunction.CamFindMark_2:
						if (CallBackInfo==RxCam.ENCamCallBackInfo.StartLinesDetected)
						{
							Console.WriteLine("CamFindMark_2: _MarkFound={0}, LineLayout={1}, LineAttatch={2}, value={3}, dx={4}, dy={5}", _MarkFound, CallBackData.LineLayout.ToString(), CallBackData.LineAttach.ToString(), CallBackData.Value_1, CallBackData.DPosX, CallBackData.DPosY);
							_CamFunctions.Off();
							if (!_MarkFound
								&& !CallBackData.Value_1.Equals(float.NaN)
								&& CallBackData.LineAttach==LineAttachEnum.LineAttach_None
								&& CallBackData.LineLayout==LineLayoutEnum.LineLayout_Covering 
								&& Math.Abs(CallBackData.DPosY)<600)
							{
								Console.WriteLine("CamFindMark_2: Mark found, dy={0}", CallBackData.DPosY);
								_MarkFound = true;
								_CallbackData = CallBackData;
								 ActionDone();
							}
							else RxGlobals.SetupAssist.WebMove(1);
							handled=true;
						}
						break;

					case ECamFunction.CamFindMark_3:
						if (CallBackInfo==RxCam.ENCamCallBackInfo.StartLinesContinuous && CallBackData.CamResult==RxCam.ENCamResult.OK)
						{
							string info=string.Format("# {0}\n", ++_CallbackNo) 
										+ "StartLinesContinuous\n"
										+ string.Format("  Center position: X={0}, y={1}\n", CallBackData.DPosX, CallBackData.DPosY)
										+ string.Format("  Lines detected: {0} ", CallBackData.Value_1)
										+ string.Format("  Scanner.StopPos: {0} ", RxGlobals.SetupAssist.ScanStopPos);

							switch(CallBackData.LineLayout)
							{
								case LineLayoutEnum.LineLayout_Covering:	info += "all"; break;
								case LineLayoutEnum.LineLayout_FromTop:		info += "top"; break;
								case LineLayoutEnum.LineLayout_FromBot:		info += "bottom"; break;
								case LineLayoutEnum.LineLayout_FromRight:	info += "right"; break;
								case LineLayoutEnum.LineLayout_FromLeft:	info += "left"; break;
								default: break;
							}
							info += string.Format("\nMarkFound={0} StopPos={1}", _MarkFound, RxGlobals.SetupAssist.ScanStopPos);
							Console.WriteLine("{0}: CamFindMark_3: LineLayout={1}, motorPos={2}, MarkFound={3}", RxGlobals.Timer.Ticks(), CallBackData.LineLayout.ToString(),  RxGlobals.SetupAssist.ScanPos, _MarkFound);
							if (CallBackData.LineLayout==LineLayoutEnum.LineLayout_FromRight || _SimuCamera)
							{		
								if (!_MarkFound)
								{ 
									_CamFunctions.Off();
									Console.WriteLine("{0}: CamFindMark_3 Mark FOUND: Stop scan, motorPos={1}", RxGlobals.Timer.Ticks(), RxGlobals.SetupAssist.ScanPos);
									_MarkFound=true;
									_CallbackData = CallBackData;
									if (_SimuMachine) _ScanMoveDone();
									else              RxGlobals.SetupAssist.ScanStop();
								}
							}
							CallbackInfo = info;
							handled=true;
						}
						break;

					//--- Measure angle ----------------------------------------------------
					case ECamFunction.CamMeasureAngle: 
							_Action.CamWorking = false;
							_CallbackData = CallBackData;
							CallbackInfo=string.Format("# {0}\n", ++_CallbackNo) 
										+ "Measure Angle\n"
										+ string.Format("  Correction: X={0}\n", CallBackData.Value_1)
										+ string.Format("  Center position: X={0}, y={1}\n", CallBackData.DPosX, CallBackData.DPosY);
							if (CallBackInfo==RxCam.ENCamCallBackInfo.AngleCorr)
							{
								Console.WriteLine("Angle: corr[{0}]={1}, DPosX={2}", _Action.MeasureCnt, CallBackData.Value_1, CallBackData.DPosX);
								_Action.Measured(CallBackData.Value_1);
							}

							if (_Action.MeasureCnt < 15)
							{
								if (_SimuMachine) _ScanMoveDone();
								else              RxGlobals.SetupAssist.ScanMoveTo(RxGlobals.SetupAssist.ScanPos+_AngleDist + CallBackData.DPosX/1000);
							}
							else 
							{
								Console.WriteLine("CALLBACK idx={0}, _OnAngleMeasured", _ActionIdx);
								_OnAngleMeasured();
								_HeadNo=0;
							}
							handled=true;
							break;

					//--- measure stitch ----------------------------------------------------------------------------
					case ECamFunction.CamMeasureStitch:
							SA_Action action=_Actions[_StitchIdx+_HeadNo];
							action.CamWorking = false;
							CallbackInfo=string.Format("# {0}\n", ++_CallbackNo) 
										+ "Measure Stitch\n"
										+ string.Format("  Correction: X={0}\n", CallBackData.Value_1)
										+ string.Format("  Center position: X={0}, y={1}\n", CallBackData.DPosX, CallBackData.DPosY);
							if (CallBackInfo==RxCam.ENCamCallBackInfo.StitchCorr)
							{
								Console.WriteLine("CALLBACK: Stitch[{0}]: corr[{1}]={2}, Pos={3}, DPosX={4}", _HeadNo, action.MeasureCnt, CallBackData.Value_1, RxGlobals.SetupAssist.ScanPos, CallBackData.DPosX);
								action.Measured(CallBackData.Value_1);
								if (!CallBackData.Value_1.Equals(float.NaN))
								{
									Console.WriteLine("Action[{0}]: ScanPos from {1} to {2}", _StitchIdx+_HeadNo, action.ScanPos, action.ScanPos + CallBackData.DPosX/1000);
									action.ScanPos += CallBackData.DPosX/1000;
								}
							}
							_HeadNo++;
							if (_HeadNo+1<_HeadsPerColor)
							{
								_ActionIdx=_StitchIdx+_HeadNo;
								_Action=_Actions[_ActionIdx];
								if (_SimuMachine) _ScanMoveDone();
								else              RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos, 500);
							}
							else
							{
								_HeadNo=0;
								if (action.MeasureCnt < 15)
								{
									_ActionIdx=_StitchIdx+_HeadNo;
									_Action=_Actions[_ActionIdx];
									_Action.WebMoveDone=false;
									Console.WriteLine("CamMeasureStitch next actinIdx={0}", _ActionIdx);
									if (_SimuMachine)
									{
										_ScanMoveDone();
										_WebMoveDone();
									}
									else
									{	
										_Action.ScanMoveDone=false;
										RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos-10, 500);
										RxGlobals.SetupAssist.WebMove(_StitchWebDist);
									}
								}
								else 
								{
									_OnStitchMeasured();
								}
							}
							handled=true;
							break;
				}
			}

			if (!handled)
			{
				switch(CallBackInfo)
				{
				case  RxCam.ENCamCallBackInfo.CameraStarted: break;
				case  RxCam.ENCamCallBackInfo.CameraStopped: break;

				default: 
				//	RX_Common.MvtMessageBox.Information("Camera", string.Format("CallbackInfo {0}", CallBackInfo.ToString()));
					break;
				}
			}
			
			callbackMutex.ReleaseMutex();
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
			if (_Action!=null && _Action.State==ECamFunctionState.runningCam) _Action.State = ECamFunctionState.aborted;
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
				CanContinue = false;
				_Action = null;
				return;
			}
			if (_Debug) CanContinue = false;
			_Action			=_Actions[_ActionIdx];
			_Action.State = (_Action.Function<ECamFunction.CamFindMark_1)? ECamFunctionState.printing : ECamFunctionState.runningCam;			
			
			switch(_Action.Function)
			{
				case ECamFunction.CamNoFunction:		_StartTestPrint();			break;
				case ECamFunction.CamFindMark_1:		_Start_CamFindMark_1();		break;
				case ECamFunction.CamFindMark_2:		_start_CamFindMark_2();		break;
				case ECamFunction.CamFindMark_3:		_start_CamFindMark_3();		break;
				case ECamFunction.CamConfirmFocus:		_start_CamConfirmFocus();	break;
				case ECamFunction.CamMeasureAngle:		_start_CamMeasureAngle();	break;
				case ECamFunction.CamMeasureStitch:		_start_CamMeasureStitch();	break;
			}
		}

		//--- Property IsRunning ---------------------------------------
		public bool IsRunning
		{
			get { return _Action!=null; }
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
			if (_Action!=null && _Actions!=null)
			{
				if (_Action.State!=ECamFunctionState.waitRob && _Action.State!=ECamFunctionState.runningRob) 
					_Action.State = ECamFunctionState.done;
				if (_Debug && _Action!=_Actions.Last()) CanContinue = true;
			//	if (!_Debug || cont)// || _ActionIdx<3)
				{
					_Action = null;
					_ActionIdx++;
					if (_ActionIdx<_Actions.Count()) _StartAction();
					else RxGlobals.SetupAssist.ScanReference();
				}
			}
		}
		
		public List<SA_Action> Test()
		{
			/*
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
			*/

			RxBindable.Invoke(()=>
			{
				if (_Actions==null)
				{
					_Actions = new List<SA_Action>();
					for(int i=0; i<4; i++)
					{
						_Actions.Add(new SA_Action()
							{
								Name		= String.Format("Angle {0}-{1}",  "K", i+1),
								Function	= ECamFunction.CamMeasureAngle,
								PrintbarNo	= 0,
								HeadNo		= i,
							}
						);
					}
					for(int i=0; i<1; i++)
					{
						_Actions.Add(new SA_Action()
							{
								Name		= String.Format("Stitch {0}-{1}..{2}",  "K", i+1, i+2),
								Function	= ECamFunction.CamMeasureStitch,
								PrintbarNo	= 0,
								HeadNo		= i,
								WebMoveDist	= (i==0)?10.5:0,
							}
						);
					}

					_Action=_Actions[0];
					_Action.ScanPos	= RxGlobals.SetupAssist.ScanPos;
					_ActionIdx=0;
				}
				_CamFunctions.MeasureAngle();
			});

			return _Actions;
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

		//--- _Start_CamFindMark_1 ------------------------------------
		private bool _MarkFound=false;
		private void _Start_CamFindMark_1()
		{
		//	RxGlobals.Events.AddItem(new LogItem("Camera: Finding Mark"));
			Console.WriteLine("_FindMark_1: _MarkFound = false");
			_MarkFound = false;
			if (_SimuMachine) _ScanMoveDone();
			else              RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
		}

		//--- _start_CamFindMark_2 ----------------------------------------------
		private void _start_CamFindMark_2()
		{
			_MarkFound=false;
			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			Console.WriteLine("{0}: _start_CamFindMark_2, MarkFound={1}", RxGlobals.Timer.Ticks(), _MarkFound);
			if (_SimuMachine)
			{
				_ScanMoveDone();
				_WebMoveDone();
			}
			else
			{
				RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
				RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
			}
		}

		//--- _start_CamFindMark_3 -----------------------
		private void _start_CamFindMark_3()
		{
		//	Console.WriteLine("{0}: Action[{1}]: _start_CamFindMark_3, ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			
			if (_SimuMachine) _WebMoveDone();
			else RxGlobals.SetupAssist.WebMove(-_CallbackData.DPosY/1000);
		}

		//--- _start_CamConfirmFocus ----------------------------------------------
		private void _start_CamConfirmFocus()
		{
			RxGlobals.SetupAssist.ScanReference();
			RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
		}

		//--- _start_CamMeasureAngle ----------------------------------------------
		private void _start_CamMeasureAngle()
		{
		//	RxGlobals.Events.AddItem(new LogItem("_start_CamMeasureAngle: stopPos={0}", RxGlobals.SetupAssist.ScanStopPos));

			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			if (_SimuMachine)
			{
				_ScanMoveDone();
				_WebMoveDone();
			}
			else
			{
				if (_Actions[_ActionIdx-1].Function==ECamFunction.CamConfirmFocus) _StartCamFunction();
				else
				{
					if (_Action.HeadNo==0) RxGlobals.SetupAssist.ScanReference();
					else RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
					RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
				}
			}
		}

		//--- _start_CamMeasureStitch ----------------------------------------------
		private void _start_CamMeasureStitch()
		{
		//	RxGlobals.Events.AddItem(new LogItem("_start_CamMeasureStitch"));

			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			if (_SimuMachine)
			{
				_ScanMoveDone();
				_WebMoveDone();
			}
			else
			{
				if (_HeadNo==0) RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos-10);
				else			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
				RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
			}
		}

		//--- _OnAngleMeasured -----------------------------
		private void _OnAngleMeasured()
		{
		//	RxGlobals.Events.AddItem(new LogItem("Camera: Send Angle Correction {0} to head{1}", _Action.Correction, _Action.HeadNo));
			_Action.State = ECamFunctionState.waitRob;
			_NextRobotCmd(_Action.StepperNo);

			if (_ActionIdx+1<_Actions.Count && _Actions[_ActionIdx+1].Function==ECamFunction.CamMeasureAngle && _Actions[_ActionIdx+1].HeadNo>0)
			{
				_Actions[_ActionIdx+1].ScanPos = RxGlobals.SetupAssist.ScanPos+2*_AngleDist;
				RxGlobals.SetupAssist.ScanMoveTo(_Actions[_ActionIdx+1].ScanPos);

				if (_StitchIdx+_Action.HeadNo<_Actions.Count())
				{					
					_Actions[_StitchIdx+_Action.HeadNo].ScanPos = RxGlobals.SetupAssist.ScanPos+_AngleDist-0.2;//+_CallbackData.DPosX/1000;
					Console.WriteLine("_OnAngleMeasured: Set Stitch Action[{0}].ScanPos={1}, DPosX={2}", _StitchIdx+_Action.HeadNo, _Actions[_StitchIdx+_Action.HeadNo].ScanPos, _CallbackData.DPosX);
				}
			}
			ActionDone();
		}

		//--- _OnStitchMeasured -----------------------------
		private void _OnStitchMeasured()
		{
		//	RxGlobals.Events.AddItem(new LogItem("Camera: Send Stitch Correction {0} to head{1}", _Action.Correction, _Action.HeadNo));
			for(int i=0; i+1<_HeadsPerColor; i++)
			{
				SA_Action action=_Actions[_StitchIdx+i];
				action.SendCorrection();
				action.State = ECamFunctionState.done;
			}
			ActionDone();
		}

		//--- _ScanMoveDone -------------------------------
		private void _ScanMoveDone()
		{
			Console.WriteLine("{0}: Action[{1}]: _ScanMoveDone", RxGlobals.Timer.Ticks(), _ActionIdx);
			if (_Action==null) return;
			_Action.ScanMoveDone=true;
			if (_Action.Function==ECamFunction.CamFindMark_3)
			{
				if (_MarkFound)
				{
					Console.WriteLine("{0}: CamFindMark_3: _ScanMoveDone motorPos={1}, stopPos={2}, DPosX={3}", RxGlobals.Timer.Ticks(), RxGlobals.SetupAssist.ScanPos, RxGlobals.SetupAssist.ScanStopPos, _CallbackData.DPosX);
					Console.WriteLine("Action[{0}].State={1}", _ActionIdx, _Action.State.ToString());
					if (_Action.State==ECamFunctionState.runningCam)
					{
						if (_ActionIdx+1<_Actions.Count())
						{
							_Actions[_ActionIdx+1].ScanPos     = RxGlobals.SetupAssist.ScanPos+0.2;
							if (_Actions[_ActionIdx+1].Function==ECamFunction.CamConfirmFocus && _ActionIdx+2<_Actions.Count())
								_Actions[_ActionIdx+2].ScanPos = RxGlobals.SetupAssist.ScanPos;
						}
						Console.WriteLine("Action[{0}].ScanPos={1}", _ActionIdx+1, _Actions[_ActionIdx+1].ScanPos);
						ActionDone();
					}
					else if (_Action.State==ECamFunctionState.endingCam) ActionDone();				
				}
				else
				{
				//	RxGlobals.Events.AddItem(new LogItem("Camera: Mark NOT Found"));
					Console.WriteLine("{0}: Mark NOT Found", RxGlobals.Timer.Ticks());
					_Action.State=ECamFunctionState.error;
					Abort();
				}
				return;
			}

			if (_Action.Function==ECamFunction.CamConfirmFocus)
			{
				Console.WriteLine("Action[{0}].CamConfirmFocus, ScanPos={1}", _ActionIdx, RxGlobals.SetupAssist.ScanPos);
				if (RxGlobals.SetupAssist.ScanPos+2<_Action.ScanPos)
				{
					_Action.ScanMoveDone=false;
					RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
				}
				else _Action.ConfirmVisibile = true;
			}

			if (_Action.Function==ECamFunction.CamMeasureAngle)
			{
				Console.WriteLine("Action[{0}].CamMeasureAngle, HeadNo={1}, ScanPos={2}", _ActionIdx, _Action.HeadNo, RxGlobals.SetupAssist.ScanPos);
				if (_Action.HeadNo==0 && RxGlobals.SetupAssist.ScanPos+2<_Action.ScanPos)
				{
					_Action.ScanMoveDone=false;
					RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
				}
			}
			if (_Action.Function==ECamFunction.CamMeasureStitch  && _Action.HeadNo==0)
			{
				Console.WriteLine("Action[{0}].CamMeasureStitch, HeadNo={1}, ScanPos={2}", _ActionIdx, _Action.HeadNo, RxGlobals.SetupAssist.ScanPos);
				if (RxGlobals.SetupAssist.ScanPos+2<_Action.ScanPos)
				{
					_Action.ScanMoveDone=false;
					RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
				}
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

					case ECamFunction.CamFindMark_1:	if (_MarkFound) 
														{															
															_StopTime = RxTimer.GetTicks()-_StopTime;
															if (_ActionIdx+1<_Actions.Count)
															{
															//	_Actions[_ActionIdx+1].WebMoveDist = -(20.0+_StopTime*(SA_Base.WebSpeed/60.0));
																string msg=string.Format("CamFindMark_1: Mark Found. WebStopTime={0} ms, move Web {1}mm", _StopTime, _Actions[_ActionIdx+1].WebMoveDist);
																Console.WriteLine(msg);
															//	RxGlobals.Events.AddItem(new LogItem(msg));
															}
															ActionDone();
														}
														else
														{
														//	RxGlobals.Events.AddItem(new LogItem("Camera: Mark NOT Found"));
															Console.WriteLine("{0}: Mark NOT Found", RxGlobals.Timer.Ticks());
															_Action.State=ECamFunctionState.error;
															Abort();
														}
														break;

					case ECamFunction.CamFindMark_3:	RxBindable.Invoke(()=>
															{
															//	RxGlobals.Events.AddItem(new LogItem("Camera: Finding Mark 3"));
																_CamFunctions.Off();
																Console.WriteLine("CamFindMark_3: _MarkFound = false");
																_MarkFound=false;
																_CamFunctions.FindMark(true);
																if (!_SimuMachine) RxGlobals.SetupAssist.ScanMoveTo(0, 10);
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

			Console.WriteLine("Action[{0}]: _StartCamFunction={4} cnt={5} ScanMoveDone={1} WebMoveDist={2} WebMoveDone={3}", _ActionIdx, _Action.ScanMoveDone, _Action.WebMoveDist, _Action.WebMoveDone, _Action.Function, _Action.MeasureCnt);

			if (_Action.ScanMoveDone && (_Action.WebMoveDist==0 || _Action.WebMoveDone))// &&  !_Action.CamWorking) 
			{
				if (_Action.Function==ECamFunction.CamMeasureStitch)
				{
					_ActionIdx=_StitchIdx+_HeadNo;
					_Action=_Actions[_ActionIdx];
				}
				_Action.State = (_Action.Function<ECamFunction.CamFindMark_1) ? ECamFunctionState.printing : ECamFunctionState.runningCam;

				_Action.CamWorking = true;
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
							_MarkFound=false;
							_CamFunctions.FindMark(true, 5);
							break;

						case ECamFunction.CamMeasureAngle:
							Console.WriteLine("Action[{0}]: Start AngleCorr, cnt={1}", _ActionIdx, _Actions[_ActionIdx].MeasureCnt);
							_CamFunctions.MeasureAngle();
							break;

						case ECamFunction.CamMeasureStitch:
							Console.WriteLine("Action[{0}]: Start StitchCorr, cnt={1}, ScanPos={2}", _ActionIdx, _Actions[_ActionIdx].MeasureCnt, RxGlobals.SetupAssist.ScanPos);
							_CamFunctions.MeasureStitch();
							break;

						default: 
							RxGlobals.Events.AddItem(new LogItem("Unknown Camera function")); 
							break;
					}
				});
			}
		}
	}
}
