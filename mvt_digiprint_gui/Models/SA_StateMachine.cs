using rx_CamLib;
using RX_Common;
using RX_DigiPrint.Converters;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using static rx_CamLib.RxCam;
using RX_DigiPrint.Devices;

namespace RX_DigiPrint.Models
{
	public class SA_StateMachine : RxBindable
	{
		private const bool		 _Debug=true;

		public const bool		 _SimuCamera  = false;
		public const bool		 _SimuMachine = true;

		private enum ENAssistMode
		{
			undef = 0,
			align = 1,
			density = 2,
		};

		private ENAssistMode	 _AssistMode=ENAssistMode.undef;
		private RxCamFunctions	 _CamFunctions;
		private List<SA_Action>  _Actions;
		private int				 _ActionIdx;
		private SA_Action		 _Action;
		private int				 _StitchIdx;
		private int				 _DistIdx;
		private int				 _HeadNo;
		private const double	 _AngleDist		= (128.0*25.4)/1200;
		private const double	 _StitchWebDist = 2*(140.0*25.4)/1200;
		private readonly double	 _DistWebDist   = Math.Round((24.0*25.4)/1200, 3);	// 
		private int				 _DistStepCnt   = 0;
		private double			 _DistCorr		= 0;
		private bool			 _Confirmed		= false;
		private int				_CamMeasureAngle_Step;
		private RxCam.CallBackDataStruct	_CallbackData;
		private int				 _StopTime;
		private bool[]			 _RobotRunning= new bool[4];

		private readonly int[]	 _DensitiyLevels = { 20, 30, 40, 50};
		private readonly int[]	 _DensitiyDist   = { 42, 42, 108, 64, 128, 64, 128, 64, 128, 64, 128, 64, 128, 64, 128, 64, 128, 64, 128, 64, 128, 64, 108, 42, 42};


		private int				_HeadsPerColor=4;

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
					if (action.Correction!=null)
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
		public List<SA_Action> Start()
		{
			int color, n;

			for (n=0; n<_RobotRunning.Length; n++) _RobotRunning[n]=false;

			_AssistMode = ENAssistMode.align;

			_Actions = new List<SA_Action>();
			_Actions.Add(new SA_Action(){Name="Print Image"});

			_Actions.Add(new SA_Action()
			{
				Function = ECamFunction.CamFindLines_Vertical,
				Name="Find 3 Vert Lines",
				ScanPos	= 30.0,
			});

			_Actions.Add(new SA_Action()
			{
				Function = ECamFunction.CamFindLine_Horzizontal,
				Name="Find Horiz Line",
				WebMoveDist = -5,
				ScanPos	    = 30.0,
			});

			_Actions.Add(new SA_Action()
			{
				Function = ECamFunction.CamFindFirstAngle,
				Name="Find First Angle",
				WebMoveDist = 12.0,
				WebPos		= 12.0,
				ScanPos	    = 0,
			});

			if (!_Confirmed)
			{
				_Actions.Add(new SA_Action()
				{
					Function = ECamFunction.CamConfirmFocus,
					Name="Confirm",
				//	WebMoveDist = 12.0,
					WebMoveDist = 0,
					WebPos = 12.0
				});
			}

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
							WebPos		= 22.0,
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
							WebPos		= 82.840,
						};						
						_Actions.Add(action);
					}
				}
			}

			_DistStepCnt = 0;
			_DistCorr    = 0;
			_ActionIdx	 = 0;
			_HeadNo		 = 0;
			_StartAction();
			return _Actions;
		}

		//--- _InitActions -----------------------------
		public List<SA_Action> StartDensity()
		{
			int color, n;

			for (n=0; n<_RobotRunning.Length; n++) _RobotRunning[n]=false;

			_AssistMode = ENAssistMode.density;

			_Actions = new List<SA_Action>();
			_Actions.Add(new SA_Action(){Name="Print Image"});

			_Actions.Add(new SA_Action()
			{
				Function = ECamFunction.CamFindLines_Vertical,
				Name="Find 3 Vert Lines",
				ScanPos	= 30.0,
			});

			_Actions.Add(new SA_Action()
			{
				Function = ECamFunction.CamFindLine_Horzizontal,
				Name="Find Horiz Line",
				WebMoveDist = 0,
				ScanPos	    = 30.0,
			});

			_Actions.Add(new SA_Action()
			{
				Function = ECamFunction.CamFindFirstAngle,
				Name="Find First Angle",
				WebMoveDist = 12.0,
				WebPos		= 12.0,
				ScanPos	    = 0,
			});

			if (!_Confirmed)
			{
				_Actions.Add(new SA_Action()
				{
					Function = ECamFunction.CamConfirmFocus,
					Name="Confirm",
				//	WebMoveDist = 12.0,
					WebMoveDist = 0,
					WebPos = 12.0
				});
			}

			if (!RxGlobals.I1Pro3.IsWhiteCalibrated)
			{
				_Actions.Add(new SA_Action()
				{
					Function = ECamFunction.Cam_I1_calibrate,
					Name="White Calibration",
				//	WebMoveDist = 12.0,
					WebMoveDist = 0,
					WebPos = 12.0
				});
			}


			int dist=0;
			for (n=0; n<_DensitiyDist.Length; n++)
			{
				dist+=_DensitiyDist[n];
				Console.WriteLine("DsnsityDist cnt={0:F00}, dist={1:F000}, jet={2:F0000}", n, _DensitiyDist[n], dist);
			}

			//--- measurmentfunctions -----------------------------
			for (color=0; color<RxGlobals.PrintSystem.ColorCnt; color++)
			{
				InkType ink = RxGlobals.InkSupply.List[color*RxGlobals.PrintSystem.InkCylindersPerColor].InkType;
				if (ink!=null)
				{
				//	_HeadsPerColor = RxGlobals.PrintSystem.HeadsPerColor;
					string colorName = new ColorCode_Str().Convert(ink.ColorCode, null, color*RxGlobals.PrintSystem.InkCylindersPerColor, null).ToString();
					for (int d=0; d<_DensitiyLevels.Length; d++)
					{
						SA_Action action=new SA_Action()
						{
							WebMoveDist = (700.0*25.4)/1200,
							PrintbarNo	= color,
							StepperNo   = color/2,
							HeadNo		= n,
							Function	= ECamFunction.Cam_I1_measure,
							Name		= String.Format(" {0}-{1}",  colorName, n+1),
						};						
						_Actions.Add(action);
					}
				}
			}

			_DistStepCnt = 0;
			_DistCorr    = 0;
			_ActionIdx	 = 0;
			_HeadNo		 = 0;
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
				
				if (false) // start at distance measurement
				{
					_Actions[_DistIdx].ScanPos = RxGlobals.SetupAssist.ScanPos+40;
					for (int i=1; i+1<_HeadsPerColor; i++)
						_Actions[_DistIdx+i].ScanPos = _Actions[_DistIdx+i-1].ScanPos+(2000*25.4/1200);
					_ActionIdx = _DistIdx-1;
				}
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
					case ECamFunction.CamFindLines_Vertical:
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

					case ECamFunction.CamFindLine_Horzizontal:
						if (CallBackInfo==RxCam.ENCamCallBackInfo.StartLinesDetected)
						{
							Console.WriteLine("CamFindLine_Horzizontal: _MarkFound={0}, LineLayout={1}, LineAttatch={2}, value={3}, dx={4}, dy={5}", _MarkFound, CallBackData.LineLayout.ToString(), CallBackData.LineAttach.ToString(), CallBackData.Value_1, CallBackData.DPosX, CallBackData.DPosY);
							_CamFunctions.Off();
							if (!_MarkFound
								&& !CallBackData.Value_1.Equals(float.NaN)
								&& CallBackData.LineAttach==LineAttachEnum.LineAttach_None
								&& CallBackData.LineLayout==LineLayoutEnum.LineLayout_Covering 
								&& Math.Abs(CallBackData.DPosY)<600)
							{
								Console.WriteLine("CamFindLine_Horzizontal: Mark found, dy={0}", CallBackData.DPosY);
								_MarkFound = true;
								_CallbackData = CallBackData;
								RxGlobals.SetupAssist.WebMove(-CallBackData.DPosY/1000);
							}
							else RxGlobals.SetupAssist.WebMove(1);
							handled=true;
						}
						break;
					
					//--- Find first angle ----------------------------------------------------
					case ECamFunction.CamFindFirstAngle: 
							_CallbackData = CallBackData;
							CallbackInfo=string.Format("# {0}\n", ++_CallbackNo) 
										+ "Measure Angle\n"
										+ string.Format("  Correction: X={0:0.00}\n", CallBackData.Value_1)
										+ string.Format("  Center position: X={0:0.00}, y={1:0.00}\n", CallBackData.DPosX, CallBackData.DPosY);

							if (CallBackInfo==RxCam.ENCamCallBackInfo.AngleCorr && !Equals(CallBackData.Value_1, float.NaN))
							{
								RxGlobals.SetupAssist.ScanStop();
								_MarkFound=true;
							}
							handled=true;
							break;

					//--- Measure angle ----------------------------------------------------
					case ECamFunction.CamMeasureAngle: 
							_CallbackData = CallBackData;
							CallbackInfo=string.Format("# {0}\n", ++_CallbackNo) 
										+ "Measure Angle\n"
										+ string.Format("  Correction: {0:0.00} cnt={1}\n", CallBackData.Value_1, CallBackData.NumMeasures)
										+ string.Format("  Center position: X={0:0.00}, y={1:0.00}\n", CallBackData.DPosX, CallBackData.DPosY);

							if (_CamMeasureAngle_Step==1 && CallBackInfo==RxCam.ENCamCallBackInfo.AngleCorr && !Equals(CallBackData.Value_1, float.NaN))
							{
								_CamMeasureAngle_Step++;
								RxGlobals.SetupAssist.ScanStop();
								_MarkFound=true;
								_CamFunctions.Off();
							}
							else if (_CamMeasureAngle_Step==3)
							{
								if (CallBackInfo==RxCam.ENCamCallBackInfo.AngleCorr)
								{
									Console.WriteLine("Angle: corr[{0}]={1}, DPosX={2}", _Action.MeasureCnt, CallBackData.Value_1, CallBackData.DPosX);
									_Action.Measured(CallBackData.Value_1);
								}
								_Action.ScanPos = RxGlobals.SetupAssist.ScanPos - CallBackData.DPosX/1000;
								_CamFunctions.Off();
								_CamMeasureAngle_Step = 0;
								Console.WriteLine("Callback: _CamMeasureAngle_Step=3, Cnt={0}, move from {1} to {2}", _Action.MeasureCnt, RxGlobals.SetupAssist.ScanPos, RxGlobals.SetupAssist.ScanPos+2*_AngleDist-0.5);
								RxGlobals.SetupAssist.ScanMoveTo(RxGlobals.SetupAssist.ScanPos+2*_AngleDist-0.5);
								if (_Action.MeasureCnt >= 8)
								{
									Console.WriteLine("CALLBACK idx={0}, _CamMeasureAngle_done", _ActionIdx);
									_CamMeasureAngle_done();
									_HeadNo=0;
								}
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
							//	if (action.MeasureCnt < 19)
								if (action.MeasureCnt < 5)
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
							double offset=-((_DistStepCnt/4)-2) * ((18.0*25.4)/1200);
							CallbackInfo=string.Format("# {0}\n", ++_CallbackNo) 
										+ "Measure Dist\n"
										+ string.Format("  Correction: X={0:0.00} offset={1:0.00}\n", CallBackData.Value_1, offset)
										+ string.Format("  Center position: X={0:0.00}, y={1:0.00}\n", CallBackData.DPosX, CallBackData.DPosY);
							Console.WriteLine("Step {0}, WebPos={1:0.000}: {2}", _DistStepCnt, RxGlobals.SetupAssist.WebPos, CallbackInfo);

							_CamFunctions.Off();
							if (CallBackInfo!=RxCam.ENCamCallBackInfo.RegisterCorr)
								break;
							action.Measured(CallBackData.Value_1);
							if (!float.IsNaN(CallBackData.Value_1) || _DistStepCnt>25)
							{ 
								if (!float.IsNaN(CallBackData.Value_1))
								{ 
									double corr=offset+CallBackData.Value_1/1000;
									Console.WriteLine("Measure Dist: corr={0:0.00}, _DistCorr={1:0.00}, sum={2:0.00}", corr, _DistCorr, _DistCorr+corr);
								//	if (action.Correction!=null && Math.Abs((double)action.Correction)>0.010) 
									{
										action.Correction = corr;
										_DistCorr += corr;
										RxGlobals.PrintSystem.HeadDist[action.PrintbarNo*RxGlobals.PrintSystem.HeadsPerColor+ action.HeadNo+1] += _DistCorr;
									}
									action.State = ECamFunctionState.done;
									RxGlobals.PrintSystem.SendMsg(TcpIp.CMD_SET_PRINTER_CFG);									
								}
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
								else 
								{
									_DistCorr = 0;
									_CamMeasureDist_done();
								}
							}
							else
							{
								action.WebMoveDone = false;
								RxGlobals.SetupAssist.WebMove(_DistWebDist);
								_DistStepCnt++;
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
				_Action.State = (_Action.Function<ECamFunction.CamFindLines_Vertical)? ECamFunctionState.printing : ECamFunctionState.runningCam;			
			
			switch(_Action.Function)
			{
				case ECamFunction.CamNoFunction:			_TestPrint_start();					break;
				case ECamFunction.CamFindLines_Vertical:	_CamFindLines_Vertical_start();		break;
				case ECamFunction.CamFindLine_Horzizontal:	_CamFindLine_Horzizontal_start();	break;
				case ECamFunction.CamFindFirstAngle:		_CamFindFirstAngle_start();			break;
				case ECamFunction.CamConfirmFocus:			_CamConfirmFocus_start();			break;
				case ECamFunction.CamMeasureAngle:			_CamMeasureAngle_start();			break;
				case ECamFunction.CamMeasureStitch:			_CamMeasureStitch_start();			break;
				case ECamFunction.CamMeasureDist:			_CamMeasureDist_start();			break;
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
			RxCam.ENCamResult result;

			_Action = new SA_Action()
				{
					Function = ECamFunction.CamMeasureDist,
					Name="Test",
					WebMoveDist = 0,
					ScanPos	    = 0,
				};

			_ActionIdx=0;
			_Actions = new List<SA_Action>();
			_Actions.Add(_Action);
			_CamFunctions.MeasureDist();

			return _Actions;


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
			result=_CamFunctions.MeasureAngle(false);
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
			if (_AssistMode==ENAssistMode.density)
				item.TestImage	= ETestImage.SA_Density;
			else
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

		//--- _CamFindLines_Vertical_start ------------------------------------
		private bool _MarkFound=false;
		private void _CamFindLines_Vertical_start()
		{
		//	RxGlobals.Events.AddItem(new LogItem("Camera: Finding Mark"));
			Console.WriteLine("_FindMark_1: _MarkFound = false");
			_MarkFound = false;
			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
		}

		//--- _CamFindLine_Horzizontal_start ----------------------------------------------
		private void _CamFindLine_Horzizontal_start()
		{
			_MarkFound=false;
			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			Console.WriteLine("{0}: _CamFindLine_Horzizontal_start, MarkFound={1}", RxGlobals.Timer.Ticks(), _MarkFound);			
			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
			RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
		}

		//--- _CamFindFirstAngle_start -----------------------
		private void _CamFindFirstAngle_start()
		{
		//	Console.WriteLine("{0}: Action[{1}]: _CamFindFirstAngle_start, ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			RxGlobals.SetupAssist.ScanReference();
			RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
		}

		//--- _CamConfirmFocus_start ----------------------------------------------
		private void _CamConfirmFocus_start()
		{
			if (_Confirmed) ActionDone();
		}

		//--- _CamMeasureAngle_start ----------------------------------------------
		private void _CamMeasureAngle_start()
		{
		//	RxGlobals.Events.AddItem(new LogItem("_CamMeasureAngle_start: stopPos={0}", RxGlobals.SetupAssist.ScanStopPos));

			Console.WriteLine("{0}: _CamMeasureAngle_start Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			_Confirmed = true;
			if (_Actions[_ActionIdx-1].Function==ECamFunction.CamConfirmFocus || _Actions[_ActionIdx-1].Function==ECamFunction.CamFindFirstAngle)
			{
				_Action.ScanMoveDone = _Action.WebMoveDone = true;
				_CamMeasureAngle_Step = 3;
				_StartCamFunction();
			}
			/*
			else
			{
				_CamMeasureAngle_Step = 0;
				if (_Action.HeadNo==0) RxGlobals.SetupAssist.ScanReference();
				else _ScanMoveDone();
				RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
			}
			*/
			Console.WriteLine("_CamMeasureAngle_start: _CamMeasureAngle_Step={0}", _CamMeasureAngle_Step);
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

			Console.WriteLine("{0}: Action[{1}]: Move web from {2} to {3} dist={4}", RxGlobals.Timer.Ticks(), _ActionIdx, RxGlobals.SetupAssist.WebPos, _Action.WebPos, _Action.WebPos-RxGlobals.SetupAssist.WebPos);

			RxGlobals.SetupAssist.WebMove(_Action.WebPos-RxGlobals.SetupAssist.WebPos);
		}

		//--- _CamMeasureAngle_done -----------------------------
		private void _CamMeasureAngle_done()
		{
		//	RxGlobals.Events.AddItem(new LogItem("Camera: Send Angle Correction {0} to head{1}", _Action.Correction, _Action.HeadNo));
			_Action.State = ECamFunctionState.waitRob;
			_NextRobotCmd(_Action.StepperNo);

			if (_ActionIdx+1<_Actions.Count && _Actions[_ActionIdx+1].Function==ECamFunction.CamMeasureAngle && _Actions[_ActionIdx+1].HeadNo>0)
			{
				_Actions[_ActionIdx+1].ScanPos = _Actions[_ActionIdx].ScanPos+_AngleDist;

				if (_StitchIdx+_Action.HeadNo<_Actions.Count())
				{					
					_Actions[_StitchIdx+_Action.HeadNo].ScanPos = RxGlobals.SetupAssist.ScanPos+_AngleDist-0.2;//+_CallbackData.DPosX/1000;
					Console.WriteLine("_CamMeasureAngle_done: Set Stitch Action[{0}].ScanPos={1}, DPosX={2}", _StitchIdx+_Action.HeadNo, _Actions[_StitchIdx+_Action.HeadNo].ScanPos, _CallbackData.DPosX);
				}
				if (_DistIdx+_Action.HeadNo<_Actions.Count())
				{					
					_Actions[_DistIdx+_Action.HeadNo].ScanPos = RxGlobals.SetupAssist.ScanPos+_AngleDist+0.2;//+_CallbackData.DPosX/1000;
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
			/*
		//	RxGlobals.Events.AddItem(new LogItem("Camera: Send Stitch Correction {0} to head{1}", _Action.Correction, _Action.HeadNo));
			for(int i=0; i+1<_HeadsPerColor; i++)
			{
				SA_Action action=_Actions[_DistIdx+i];
				if (action.Correction!=null && Math.Abs((double)action.Correction)>0.010) 
				{
					RxGlobals.PrintSystem.HeadDist[action.PrintbarNo*RxGlobals.PrintSystem.HeadsPerColor+ action.HeadNo+1] -= (double)action.Correction;
				}
				action.State = ECamFunctionState.done;
			}
			RxGlobals.PrintSystem.SendMsg(TcpIp.CMD_SET_PRINTER_CFG);
			*/
			ActionDone();
		}

		//--- _ScanMoveDone -------------------------------
		private void _ScanMoveDone()
		{
			Console.WriteLine("{0}: Action[{1}]: _ScanMoveDone", RxGlobals.Timer.Ticks(), _ActionIdx);
			if (_Action==null) return;
			_Action.ScanMoveDone=true;

			if (_Action.Function==ECamFunction.CamFindFirstAngle)
			{
				Console.WriteLine("{0}: CamFindFirstAngle: _ScanMoveDone motorPos={1}, stopPos={2}, DPosX={3}", RxGlobals.Timer.Ticks(), RxGlobals.SetupAssist.ScanPos, RxGlobals.SetupAssist.ScanStopPos, _CallbackData.DPosX);
				if (RxGlobals.SetupAssist.ScanPos>5)
				{
					if (_MarkFound) 
					{
						ActionDone();
					}
					else
					{
						Console.WriteLine("{0}: Mark NOT Found", RxGlobals.Timer.Ticks());
						_Action.State=ECamFunctionState.error;
						Abort();
					}
				}
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
				switch(_CamMeasureAngle_Step)
				{
					case 0: _CamMeasureAngle_Step=1;
							_CamFunctions.MeasureAngle(true);
							_Action.ScanMoveDone=false;
							RxGlobals.SetupAssist.ScanMoveTo(RxGlobals.SetupAssist.ScanPos+1, 5);
							break;

					case 2: Thread.Sleep(100);
							_CamMeasureAngle_Step = 3;
							break;
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
			Console.WriteLine("{0}: Action[{1}]: _WebMoveDone, WebPos={2:0.000}", RxGlobals.Timer.Ticks(), _ActionIdx, RxGlobals.SetupAssist.WebPos);
			if (_Action!=null)
			{
				_Action.WebMoveDone=true;
				switch (_Action.Function)
				{
					case ECamFunction.CamNoFunction:	ActionDone();
														break;

					case ECamFunction.CamFindLines_Vertical:	
														if (_MarkFound) 
														{															
															_StopTime = RxTimer.GetTicks()-_StopTime;
															if (_ActionIdx+1<_Actions.Count)
															{
															//	_Actions[_ActionIdx+1].WebMoveDist = -(20.0+_StopTime*(SA_Base.WebSpeed/60.0));
																string msg=string.Format("CamFindLines_Vertical: Mark Found. WebStopTime={0} ms, move Web {1}mm", _StopTime, _Actions[_ActionIdx+1].WebMoveDist);
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

					case ECamFunction.CamFindLine_Horzizontal:
														if (_MarkFound)
														{ 
															RxGlobals.SetupAssist.WebPos_Reset();
															ActionDone();
														}
														else _StartCamFunction();
														break;

					case ECamFunction.CamFindFirstAngle:
														if (_Action.ScanMoveDone)
															_StartCamFunction();
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
					_Action.State = (_Action.Function<ECamFunction.CamFindLines_Vertical) ? ECamFunctionState.printing : ECamFunctionState.runningCam;

				switch(_Action.Function)
				{
					case ECamFunction.CamNoFunction:
						break;

					case ECamFunction.CamFindLines_Vertical:				
						_CamFunctions.FindLines_Vertical();
						RxGlobals.SetupAssist.WebMove(1000.0);
						break;

					case ECamFunction.CamFindLine_Horzizontal:
						_MarkFound=false;
						_CamFunctions.FindLine_Horizontal();
						break;

					case ECamFunction.CamFindFirstAngle:
						Console.WriteLine("Action[{0}]: Start MeasureAngle, cnt={1}", _ActionIdx, _Actions[_ActionIdx].MeasureCnt);
						result=_CamFunctions.MeasureAngle(true);
						if (result!=RxCam.ENCamResult.OK) RxGlobals.Events.AddItem(new LogItem("MeasureAngle Error {0}", result.ToString()));
						RxGlobals.SetupAssist.ScanMoveTo(100, 5);
						break;

					case ECamFunction.CamConfirmFocus:
						_CamFunctions.MeasureAngle(false);
						break;

					case ECamFunction.CamMeasureAngle:
						Console.WriteLine("Action[{0}]: Start MeasureAngle, cnt={1}", _ActionIdx, _Actions[_ActionIdx].MeasureCnt);
						result=_CamFunctions.MeasureAngle(false);
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
