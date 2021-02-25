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
		private int				 _DistIdx;
		private int				 _HeadNo;
		private const double	 _AngleDist = (128.0*25.4)/1200;
		private const double	 _StitchWebDist = (140.0*25.4)/1200;
//		private const double	 _DistWebDist   = (120.0*25.4)/1200;
		private const double	 _DistWebDist   = 0.5;
		private int				 _DistStepCnt   = 0;
		private RxCam.CallBackDataStruct	_CallbackData;
		private int				 _StopTime;
		private bool[]			 _RobotRunning= new bool[4];

		private int				_HeadsPerColor=2;

		public SA_StateMachine()
		{
			RxGlobals.Camera.CamCallBack += new RxCam.CameraCallBack(CallBackfromCam);
			RxGlobals.SetupAssist.OnWebMoveDone  = _WebMoveDone;
			RxGlobals.SetupAssist.OnScanMoveDone = _ScanMoveDone;
			RxGlobals.SetupAssist.Simu			 = _SimuMachine;
			foreach (var state in RxGlobals.StepperStatus)
			{
				state.PropertyChanged += SA_StateMachine_PropertyChanged; 			
			}				
			_CamFunctions = new RxCamFunctions(RxGlobals.Camera);
			if (_SimuCamera) _CamFunctions.SimuCallback	+= CallBackfromCam;
		}

		private void SA_StateMachine_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
		{
			for (int no=0; no<RxGlobals.StepperStatus.Length; no++)
			{
				if (sender==RxGlobals.StepperStatus[no])
				{
					StepperStatus stepper = RxGlobals.StepperStatus[no];
					Console.WriteLine("Stepper[{0}].{1}={2}", no, e.PropertyName, sender.GetType().GetProperty(e.PropertyName).GetValue(sender, null));
					if (e.PropertyName.Equals("ScrewCnt") && _Actions!=null)
					{
						foreach(SA_Action action in _Actions)
						{	
							if (action.StepperNo==no && action.State==ECamFunctionState.runningRob)
							{
								_RobotRunning[no]=false;
								if (stepper.ScrewedOk) action.State = ECamFunctionState.done;
								else				   action.State = ECamFunctionState.error;
								break;
							}
						}

						_NextRobotCmd(no);
					}
					if (e.PropertyName.Equals("ScrewerReady"))
						_NextRobotCmd(no);
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

			if (!stepper.ScrewerReady) return;

			foreach(SA_Action action in _Actions)
			{	
				if (action.StepperNo==stepperNo && action.State==ECamFunctionState.waitRob)
				{
					TcpIp.SHeadAdjustmentMsg msg = new  TcpIp.SHeadAdjustmentMsg();

					msg.printbarNo  = action.PrintbarNo;
					msg.headNo      = action.HeadNo;
					if      (action.Function==ECamFunction.CamMeasureAngle)  msg.axis = 0;
					else if (action.Function==ECamFunction.CamMeasureStitch) msg.axis = 1;
					msg.steps       = (Int32)(action.Correction * 6.0 + 0.5);
					if (Math.Abs(msg.steps)<=3)
					{
						action.State	= ECamFunctionState.done;
					}
					else
					{
						_RobotRunning[stepperNo] = true;

						action.State	= ECamFunctionState.runningRob;
						RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_ADJUST, ref msg);
					//	RxGlobals.Events.AddItem(new LogItem(string.Format("ROB Command, Head={0}, axis={1}, steps={2}", msg.headNo, msg.steps, msg.steps)));
					}
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
		private float _DPosY=0;
		private int   _DPosY_cnt=0;
		public List<SA_Action> Start()
		{
			int color, n;

			for (n=0; n<_RobotRunning.Length; n++) _RobotRunning[n]=false;

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
							Name		= String.Format(" {0}-{1}",  colorName, n+1),
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
							Name		= String.Format(" {0}-{1}..{2}",  colorName, n+1, n+2),
							WebMoveDist	= n==0? 10.0 : 0,
						};						
						_Actions.Add(action);
					}

					_DistIdx = _Actions.Count();
					for (n=0; n<_HeadsPerColor-1; n++)
					{
						SA_Action action=new SA_Action()
						{
							PrintbarNo	= color,
							StepperNo   = color/2,
							HeadNo		= n,
							Function	= ECamFunction.CamMeasureDist,
							Name		= String.Format(" {0}-{1}..{2}",  colorName, n+1, n+2),
							WebMoveDist	= n==0? 7.5 : 0,
						};						
						_Actions.Add(action);
					}
				}
			}

			_DistStepCnt = 0;
			_ActionIdx = 0;
			_StartAction();
			return _Actions;
		}

		//--- ConfirmPosAndFocus -------------------------------------
		public void ConfirmPosAndFocus()
		{
			if (_Action.Function==ECamFunction.CamConfirmFocus)
			{
				_CamFunctions.Off();
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
			SA_Action action;

			callbackMutex.WaitOne();

			if (_Action!=null)
			{
				Console.WriteLine("CALLBACK: info={0} Action[{1}].function={2}, cnt={3}, NumMeasures={4}, value={5}, DPosX={6}, DPosY={7}", CallBackInfo.ToString(), _ActionIdx, _Action.Function.ToString(), _Action.MeasureCnt, CallBackData.NumMeasures, CallBackData.Value_1, CallBackData.DPosX, CallBackData.DPosY);
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
								RxGlobals.SetupAssist.WebStop();
								_StopTime = RxTimer.GetTicks();
							//	RxGlobals.Events.AddItem(new LogItem("Camera: WebStop"));
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
										+ string.Format("  Center position: X={0:0.00}, y={1:0.00}\n", CallBackData.DPosX, CallBackData.DPosY)
										+ string.Format("  Lines detected: {0} ", CallBackData.Value_1)
										+ string.Format("  Scanner.StopPos: {0:0.00} ", RxGlobals.SetupAssist.ScanStopPos);

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
							if (CallBackData.LineLayout==LineLayoutEnum.LineLayout_FromRight)
							{		
								if (!_MarkFound)
								{ 
									_CamFunctions.Off();
									Console.WriteLine("{0}: CamFindMark_3 Mark FOUND: Stop scan, motorPos={1}", RxGlobals.Timer.Ticks(), RxGlobals.SetupAssist.ScanPos);
									_MarkFound=true;
									_CallbackData = CallBackData;
									RxGlobals.SetupAssist.ScanStop();
								}
							}
							CallbackInfo = info;
							handled=true;
						}
						break;

					//--- Measure angle ----------------------------------------------------
					case ECamFunction.CamMeasureAngle: 
							_CallbackData = CallBackData;
							CallbackInfo=string.Format("# {0}\n", ++_CallbackNo) 
										+ "Measure Angle\n"
										+ string.Format("  Correction: X={0:0.00}\n", CallBackData.Value_1)
										+ string.Format("  Center position: X={0:0.00}, y={1:0.00}\n", CallBackData.DPosX, CallBackData.DPosY);
							if (CallBackInfo==RxCam.ENCamCallBackInfo.AngleCorr)
							{
								Console.WriteLine("Angle: corr[{0}]={1}, DPosX={2}", _Action.MeasureCnt, CallBackData.Value_1, CallBackData.DPosX);
								_Action.Measured(CallBackData.Value_1);
							}

							if (_Action.MeasureCnt < 15)
							{
								RxGlobals.SetupAssist.ScanMoveTo(RxGlobals.SetupAssist.ScanPos+_AngleDist + CallBackData.DPosX/1000);
							}
							else 
							{
								Console.WriteLine("CALLBACK idx={0}, _CamMeasureAngle_done", _ActionIdx);
								_CamMeasureAngle_done();
								_HeadNo=0;
							}
							handled=true;
							break;

					//--- measure stitch ----------------------------------------------------------------------------
					case ECamFunction.CamMeasureStitch:
							action=_Actions[_StitchIdx+_HeadNo];
							if (CallBackInfo!=RxCam.ENCamCallBackInfo.StitchCorr)
								break;

							CallbackInfo=string.Format("# {0}\n", ++_CallbackNo) 
										+ "Measure Stitch\n"
										+ string.Format("  Correction: X={0:0.00}\n", CallBackData.Value_1)
										+ string.Format("  Center position: X={0:0.00}, y={1:0.00}\n", CallBackData.DPosX, CallBackData.DPosY);
							
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
								RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos, 1000);
							}
							else
							{
								_HeadNo=0;
								if (action.MeasureCnt < 19)
								{
									_ActionIdx=_StitchIdx+_HeadNo;
									_Action=_Actions[_ActionIdx];
									Console.WriteLine("CamMeasureStitch next actinIdx={0}", _ActionIdx);									
									_Action.WebMoveDone=false;
									_Action.ScanMoveDone=false;
									RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos-10, 1000);
									RxGlobals.SetupAssist.WebMove(_StitchWebDist);
								}
								else 
								{
									_CamMeasureStitch_done();
								}
							}
							handled=true;
							break;

					//--- measure distance ----------------------------------------------------------------------------
					case ECamFunction.CamMeasureDist:
							action=_Actions[_DistIdx+_HeadNo];
							CallbackInfo=string.Format("# {0}\n", ++_CallbackNo) 
										+ "Measure Dist\n"
										+ string.Format("  Correction: X={0:0.00}\n", CallBackData.Value_1)
										+ string.Format("  Center position: X={0:0.00}, y={1:0.00}\n", CallBackData.DPosX, CallBackData.DPosY);
							Console.WriteLine(CallbackInfo);
							
							_CamFunctions.Off();
							if (CallBackInfo!=RxCam.ENCamCallBackInfo.RegisterCorr)
								break;
							
							if (!float.IsNaN(CallBackData.Value_1) || _DistStepCnt>20)
							{ 
								if (!float.IsNaN(CallBackData.Value_1))
									action.Correction = 1;
								else action.State = ECamFunctionState.error;
								_HeadNo++;
								if (_HeadNo+1<_HeadsPerColor)
								{
									_ActionIdx=_DistIdx+_HeadNo;
									_Action=_Actions[_ActionIdx];
									RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos, 1000);
									RxGlobals.SetupAssist.WebMove(-_DistWebDist*_DistStepCnt);
									_DistStepCnt=0;								
								}
								else _CamMeasureDist_done();
							}
							else
							{
								action.WebMoveDone = false;
								RxGlobals.SetupAssist.WebMove(_DistWebDist);
								_DistStepCnt++;
							}
							break;

							//---- ---------------------------------

							if (_SimuCamera && action.MeasureCnt<2) CallBackData.Value_1 = float.NaN;
							if (action.MeasureCnt<5)
							{
							//	if (!CallBackData.Value_1.Equals(float.NaN) && Math.Abs(CallBackData.Value_1)<100 && action.BaseIdx<0)
								if (!CallBackData.Value_1.Equals(float.NaN) && action.BaseIdx<0)
								{
									action.BaseIdx = action.MeasureCnt;
									if (action.MeasureCnt!=2) 
									{
										CallBackData.Value_1 += (float)((action.MeasureCnt-2)*(20.0*25.4)/1200);
										action.State = ECamFunctionState.waitRob;
										CallBackData.DPosY = 0;
									}
								}
								else
								{
									CallBackData.Value_1 = float.NaN;
								}
							} 
							else 
							{
								if (action.State != ECamFunctionState.runningCam) CallBackData.Value_1 = float.NaN;
							}

							action.Measured(CallBackData.Value_1);
							if (!CallBackData.Value_1.Equals(float.NaN))
							{
								_DPosY += CallBackData.DPosY;
								_DPosY_cnt++;
							}

							_HeadNo++;
							if (_HeadNo+1<_HeadsPerColor)
							{
								_ActionIdx=_DistIdx+_HeadNo;
								_Action=_Actions[_ActionIdx];
								RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos, 1000);
							}
							else
							{
								_HeadNo=0;
								if (action.MeasureCnt<5)
								{
									int cnt=0;
									for (int i=0; i<_HeadsPerColor-1; i++)
										if (_Actions[_DistIdx+i].State==ECamFunctionState.runningCam) cnt++;
									if (cnt==0)
									{ 
										_CamMeasureDist_done();
										break;
									}
								}
								if (action.MeasureCnt < 15)
								{
									
									_ActionIdx=_DistIdx+_HeadNo;
									_Action=_Actions[_ActionIdx];

									Console.WriteLine("CamMeasureDist next actinIdx={0}", _ActionIdx);
									Console.WriteLine("CamMeasureDist.WebMove({0})", _DistWebDist - _DPosY/1000);

									_Action.WebMoveDone=false;									
									_Action.ScanMoveDone=false;
									if (_DPosY_cnt>0) _DPosY = _DPosY/_DPosY_cnt;
									RxGlobals.SetupAssist.WebMove(_DistWebDist - _DPosY/1000);
									RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos-10, 1000);
									_DPosY	   = 0;
									_DPosY_cnt = 0;
								}
								else 
								{
									_CamMeasureDist_done();
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
			if (_Actions!=null)
			{
				foreach(SA_Action action in _Actions)
				{
					if (action.State==ECamFunctionState.runningCam) action.State = ECamFunctionState.aborted;
				}
			}
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
			if (_Action.State==ECamFunctionState.undef)
				_Action.State = (_Action.Function<ECamFunction.CamFindMark_1)? ECamFunctionState.printing : ECamFunctionState.runningCam;			
			
			switch(_Action.Function)
			{
				case ECamFunction.CamNoFunction:		_TestPrint_start();			break;
				case ECamFunction.CamFindMark_1:		_CamFindMark_1_start();		break;
				case ECamFunction.CamFindMark_2:		_CamFindMark_2_start();		break;
				case ECamFunction.CamFindMark_3:		_CamFindMark_3_start();		break;
				case ECamFunction.CamConfirmFocus:		_CamConfirmFocus_start();	break;
				case ECamFunction.CamMeasureAngle:		_CamMeasureAngle_start();	break;
				case ECamFunction.CamMeasureStitch:		_CamMeasureStitch_start();	break;
				case ECamFunction.CamMeasureDist:		_CamMeasureDist_start();	break;
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
			RxCam.ENCamResult result=_CamFunctions.MeasureDist();
			if (result!=RxCam.ENCamResult.OK) RxGlobals.Events.AddItem(new LogItem("MeasureDist Error {0}", result.ToString()));
			return _Actions;

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
			result=_CamFunctions.MeasureAngle();
			if (result!=RxCam.ENCamResult.OK) RxGlobals.Events.AddItem(new LogItem("MeasureAngle Error {0}", result.ToString()));

			return _Actions;
		}

		//--- _TestPrint_start --------------------------------------
		private void _TestPrint_start()
        {
			PrintQueueItem item = new PrintQueueItem();

            if (InkSupply.AnyFlushed()) return;
			_CamFunctions.Off();
			Console.WriteLine("{0}: _TestPrint_start", RxGlobals.Timer.Ticks());
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

		//--- _CamFindMark_1_start ------------------------------------
		private bool _MarkFound=false;
		private void _CamFindMark_1_start()
		{
		//	RxGlobals.Events.AddItem(new LogItem("Camera: Finding Mark"));
			Console.WriteLine("_FindMark_1: _MarkFound = false");
			_MarkFound = false;
			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
		}

		//--- _CamFindMark_2_start ----------------------------------------------
		private void _CamFindMark_2_start()
		{
			_MarkFound=false;
			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			Console.WriteLine("{0}: _CamFindMark_2_start, MarkFound={1}", RxGlobals.Timer.Ticks(), _MarkFound);			
			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
			RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
		}

		//--- _CamFindMark_3_start -----------------------
		private void _CamFindMark_3_start()
		{
		//	Console.WriteLine("{0}: Action[{1}]: _CamFindMark_3_start, ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			
			RxGlobals.SetupAssist.WebMove(-_CallbackData.DPosY/1000);
		}

		//--- _CamConfirmFocus_start ----------------------------------------------
		private void _CamConfirmFocus_start()
		{
			RxGlobals.SetupAssist.ScanReference();
			RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
		}

		//--- _CamMeasureAngle_start ----------------------------------------------
		private void _CamMeasureAngle_start()
		{
		//	RxGlobals.Events.AddItem(new LogItem("_CamMeasureAngle_start: stopPos={0}", RxGlobals.SetupAssist.ScanStopPos));

			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			if (_Actions[_ActionIdx-1].Function==ECamFunction.CamConfirmFocus)
			{
				_Action.ScanMoveDone = _Action.WebMoveDone = true;
				_StartCamFunction();
			}
			else
			{
				if (_Action.HeadNo==0) RxGlobals.SetupAssist.ScanReference();
				else RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
				RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
			}
		}

		//--- _CamMeasureStitch_start ----------------------------------------------
		private void _CamMeasureStitch_start()
		{
		//	RxGlobals.Events.AddItem(new LogItem("_CamMeasureStitch_start"));

			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			if (_HeadNo==0) RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos-10);
			else			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
			RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
		}

		//--- _CamMeasureDist_start ----------------------------------------------
		private void _CamMeasureDist_start()
		{
		//	RxGlobals.Events.AddItem(new LogItem("_CamMeasureDist_start"));

			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			if (_HeadNo==0) RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos-10);
			else			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
			RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
		}

		//--- _CamMeasureAngle_done -----------------------------
		private void _CamMeasureAngle_done()
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
					Console.WriteLine("_CamMeasureAngle_done: Set Stitch Action[{0}].ScanPos={1}, DPosX={2}", _StitchIdx+_Action.HeadNo, _Actions[_StitchIdx+_Action.HeadNo].ScanPos, _CallbackData.DPosX);
				}
				if (_DistIdx+_Action.HeadNo<_Actions.Count())
				{					
					_Actions[_DistIdx+_Action.HeadNo].ScanPos = RxGlobals.SetupAssist.ScanPos+_AngleDist;//+_CallbackData.DPosX/1000;
					Console.WriteLine("_CamMeasureAngle_done: Set Dist Action[{0}].ScanPos={1}, DPosX={2}", _DistIdx+_Action.HeadNo, _Actions[_DistIdx+_Action.HeadNo].ScanPos, _CallbackData.DPosX);
				}

			}
			ActionDone();
		}

		//--- _CamMeasureStitch_done -----------------------------
		private void _CamMeasureStitch_done()
		{
		//	RxGlobals.Events.AddItem(new LogItem("Camera: Send Stitch Correction {0} to head{1}", _Action.Correction, _Action.HeadNo));
			_CamFunctions.Off();
			for(int i=0; i+1<_HeadsPerColor; i++)
			{
				SA_Action action=_Actions[_StitchIdx+i];
				action.State = ECamFunctionState.waitRob;
				_NextRobotCmd(action.StepperNo);
			}
			ActionDone();
		}

		//--- _CamMeasureDist_done -----------------------------
		private void _CamMeasureDist_done()
		{
		//	RxGlobals.Events.AddItem(new LogItem("Camera: Send Stitch Correction {0} to head{1}", _Action.Correction, _Action.HeadNo));
			for(int i=0; i+1<_HeadsPerColor; i++)
			{
				SA_Action action=_Actions[_DistIdx+i];
				if (action.Correction!=null && Math.Abs((double)action.Correction)>=5)
				{
					RxGlobals.PrintSystem.HeadDist[action.PrintbarNo*RxGlobals.PrintSystem.HeadsPerColor+ action.HeadNo+1] += (double)action.Correction/1000;
				}
				action.State = ECamFunctionState.done;
			}
			RxGlobals.PrintSystem.SendMsg(TcpIp.CMD_SET_PRINTER_CFG);
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
			if (_Action.Function==ECamFunction.CamMeasureStitch && _Action.HeadNo==0 && !_SimuMachine)
			{
				Console.WriteLine("Action[{0}].CamMeasureStitch, HeadNo={1}, ScanPos={2}", _ActionIdx, _Action.HeadNo, RxGlobals.SetupAssist.ScanPos);
				if (RxGlobals.SetupAssist.ScanPos+2<_Action.ScanPos)
				{
					_Action.ScanMoveDone=false;
					RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
				}
			}
			if (_Action.Function==ECamFunction.CamMeasureDist && _Action.HeadNo==0 && !_SimuMachine)
			{
				Console.WriteLine("Action[{0}].CamMeasureDist, HeadNo={1}, ScanPos={2}", _ActionIdx, _Action.HeadNo, RxGlobals.SetupAssist.ScanPos);
				if (RxGlobals.SetupAssist.ScanPos+2<_Action.ScanPos)
				{
					_Action.ScanMoveDone=false;
					RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
				}
			}
			
			Console.WriteLine("{0}: Action[{1}]: _ScanMoveDone: WebMoveDist={2}, WebMoveDone={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.WebMoveDist, _Action.WebMoveDone);
			if (_Action.WebMoveDist==0 || _Action.WebMoveDone) _StartCamFunction();
		}
		
		//--- _WebMoveDone -----------------------------------------------------------------------------------------------
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

					case ECamFunction.CamFindMark_3:	//	RxGlobals.Events.AddItem(new LogItem("Camera: Finding Mark 3"));
														_CamFunctions.Off();
														Console.WriteLine("CamFindMark_3: _MarkFound = false");
														_MarkFound=false;
														_CamFunctions.FindMark(true);
														RxGlobals.SetupAssist.ScanMoveTo(0, 10);
														break;

					default: _StartCamFunction(); break;
				}
			}
		}

		//--- _StartCamFunction ----------------------------------------------
		private void _StartCamFunction()
		{
			RxCam.ENCamResult result;

			if (_Action==null) 
				return;

			Console.WriteLine("Action[{0}]: _StartCamFunction={4} cnt={5} ScanMoveDone={1} WebMoveDist={2} WebMoveDone={3}", _ActionIdx, _Action.ScanMoveDone, _Action.WebMoveDist, _Action.WebMoveDone, _Action.Function, _Action.MeasureCnt);

			if (_Action.ScanMoveDone && (_Action.WebMoveDist==0 || _Action.WebMoveDone)) 
			{
				if (_Action.Function==ECamFunction.CamMeasureStitch)
				{
					_ActionIdx=_StitchIdx+_HeadNo;
					_Action=_Actions[_ActionIdx];
				}
				if (_Action.Function==ECamFunction.CamMeasureDist)
				{
					_ActionIdx=_DistIdx+_HeadNo;
					_Action=_Actions[_ActionIdx];
				}
				if (_Action.State==ECamFunctionState.undef)
					_Action.State = (_Action.Function<ECamFunction.CamFindMark_1) ? ECamFunctionState.printing : ECamFunctionState.runningCam;

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

					case ECamFunction.CamConfirmFocus:
						_CamFunctions.MeasureAngle();
						break;

					case ECamFunction.CamMeasureAngle:
						Console.WriteLine("Action[{0}]: Start MeasureAngle, cnt={1}", _ActionIdx, _Actions[_ActionIdx].MeasureCnt);
						result=_CamFunctions.MeasureAngle();
						if (result!=RxCam.ENCamResult.OK) RxGlobals.Events.AddItem(new LogItem("MeasureAngle Error {0}", result.ToString()));
						break;

					case ECamFunction.CamMeasureStitch:
						Console.WriteLine("Action[{0}]: Start MeasureStitch, cnt={1}, ScanPos={2}", _ActionIdx, _Actions[_ActionIdx].MeasureCnt, RxGlobals.SetupAssist.ScanPos);
						result=_CamFunctions.MeasureStitch();
						if (result!=RxCam.ENCamResult.OK) RxGlobals.Events.AddItem(new LogItem("MeasureStitch Error {0}", result.ToString()));
						break;

					case ECamFunction.CamMeasureDist:
						Console.WriteLine("Action[{0}]: Start MeasureDist, cnt={1}, ScanPos={2}", _ActionIdx, _Actions[_ActionIdx].MeasureCnt, RxGlobals.SetupAssist.ScanPos);
					//	if (_Action.State == ECamFunctionState.runningCam)
						if (false)
						{
							result=_CamFunctions.MeasureDist();
							if (result!=RxCam.ENCamResult.OK) RxGlobals.Events.AddItem(new LogItem("MeasureDist Error {0}", result.ToString()));
						}
						break;

					default: 
						RxGlobals.Events.AddItem(new LogItem("Unknown Camera function")); 
						break;
				}
			}
		}
	}
}
