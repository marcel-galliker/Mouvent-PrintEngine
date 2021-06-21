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
using System.Windows.Media;

namespace RX_DigiPrint.Models
{
	public class SA_StateMachine : RxBindable
	{
		private const bool		_Debug=true;

		public const bool		_SimuCamera  = false;
		public bool				_SimuMachine = false;

		private const int		_Adjustment_Tolerance = 3; // in 1/6 screw turns

		private enum ENAssistMode
		{
			undef = 0,
			align = 1,
			density = 2,
			register = 3,
		};

		private static double px2mm(int px)
		{
			return (px*25.4/1200.0);
		}

		private ENAssistMode	 _AssistMode=ENAssistMode.undef;
		private RxCamFunctions	 _CamFunctions;
		private List<SA_Action>  _Actions;
		private int				 _ActionIdx;
		private SA_Action		 _Action;
		private int[]			 _StitchIdxes=new int[8];
		private int				 _StitchIdx;
		private int[]			 _DistIdxes=new int[8];
		private int				 _DistIdx;
		private int				 _HeadNo;
		private readonly double  _FindPos		= 20.0;
		private readonly double	 _AngleDist		= px2mm(128);
		private readonly double	 _StitchWebDist = px2mm(2*140);
		private readonly double	 _DistWebDist   = Math.Round((24.0*25.4)/1200, 3);	// 
		private int				 _DistStepCnt   = 0;
		private int				 _DistMeasureCnt= 0;
		private bool			 _Confirmed		= false;
		private int				 _CamMeasureAngle_Step;
		private RxCam.CallBackDataStruct	_CallbackData;
		private int				 _StopTime;
		private bool[]			 _RobotRunning= new bool[4];
		private bool			 _Adjusted;

		private readonly int[]	 _DensitiyLevels = { 20, 30, 40, 50};
//		private readonly int[]	 _DensitiyDist   = { 64, 128, 64, 128, 64, 128, 64, 128, 64, 128, 64, 128, 64, 128, 64, 128, 64, 128, 64,  48+32, 48, 48, 48+32};
		private readonly int[]	 _DensitiyDist   = { 64, 128, 64, 128, 64, 128, 64, 128, 64, 128, 64, 128, 64, 128, 64, 128, 64, 128, 64,  104, 48, 104};
		private int				 _Jet;
		private List<ColorConversion.SpectroResultStruct> _DensityResult;
		private List<List<ColorConversion.SpectroResultStruct>> _DensityResults = new List<List<ColorConversion.SpectroResultStruct>>();

		private int				_HeadsPerColor=2;

		public SA_StateMachine()
		{
			RxGlobals.Camera.CamCallBack += new RxCam.CameraCallBack(CallBackfromCam);
			RxGlobals.SetupAssist.OnWebMoveDone  = _WebMoveDone;
			RxGlobals.SetupAssist.OnScanMoveDone = _ScanMoveDone;
			RxGlobals.SetupAssist.Simu			 = _SimuMachine;
			#if DEBUG
	//		_SimuMachine |= (RxGlobals.PrinterProperties.Host_Name==null);
			#endif

			foreach (var state in RxGlobals.StepperStatus)
			{
				state.PropertyChanged += SA_StateMachine_PropertyChanged; 			
			}				
			_CamFunctions = new RxCamFunctions(RxGlobals.Camera);
			if (_SimuCamera) _CamFunctions.SimuCallback	+= CallBackfromCam;
		}

		//--- SA_StateMachine_PropertyChanged --------------------------------------
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
						if (Math.Abs(msg.steps)<=_Adjustment_Tolerance)
						{
							action.State	= ECamFunctionState.done;
						}
						else
						{
							/*
							action.State	= ECamFunctionState.done;
							RxGlobals.Events.AddItem(new LogItem(string.Format("ROB Command, Head={0}, axis={1}, steps={2} NOT STARTING (Development)", msg.headNo, msg.steps, msg.steps)));
							break;
							*/

							//_Adjusted = true;
							_RobotRunning[stepperNo] = true;

							action.State	= ECamFunctionState.runningRob;
							RxGlobals.Events.AddItem(new LogItem(string.Format("ROB Command, Printbar={0}, Head={1}, axis={2}, steps={3}", msg.printbarNo, msg.headNo, msg.axis, msg.steps)));
							RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_ADJUST, ref msg);
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

			/*
			{
				RxCam.ENCamCallBackInfo CallBackInfo = new ENCamCallBackInfo(); 
				RxCam.CallBackDataStruct CallBackData = new CallBackDataStruct();
				_Action=new SA_Action()
				{
					PrintbarNo	= 0,
					StepperNo   = 0,
					HeadNo		= 0,
					WebMoveDist = 12.0,
					WebPos		= 12.0,
					Function	= ECamFunction.CamMeasureAngle,
					Name		= String.Format(" {0}-{1}",  "TEST", 1),
				};
				_CamMeasureAngle_Step=3;
				CallBackfromCam(CallBackInfo, CallBackData);
			}
			*/

			for (n=0; n<_RobotRunning.Length; n++) _RobotRunning[n]=false;

			_AssistMode = ENAssistMode.align;
			_Adjusted   = false;

			_Actions = new List<SA_Action>();
			_Actions.Add(new SA_Action(){Name="Print Image"});
			//--- measurmentfunctions -----------------------------
			for (color=0; color<RxGlobals.PrintSystem.ColorCnt; color++)
			{
				InkType ink = RxGlobals.InkSupply.List[color*RxGlobals.PrintSystem.InkCylindersPerColor].InkType;
				if (ink!=null)
				{
					Brush colorBrush;
					try
					{
						colorBrush = new SolidColorBrush(ink.Color);
					}
					catch(Exception)
					{
						colorBrush=Brushes.Transparent;
					}
					_Actions.Add(new SA_Action()
					{
						PrintbarNo	= color,
						ColorBrush  = colorBrush,
						Function = ECamFunction.CamFindLines_Vertical,
						Name="Find 3 Vert Lines",
						ScanPos	= _FindPos,
					});

					_Actions.Add(new SA_Action()
					{
						PrintbarNo	= color,
						ColorBrush  = colorBrush,
						Function = ECamFunction.CamFindLine_Horzizontal,
						Name="Find Horiz Line",
						WebMoveDist = 0,
						ScanPos	    = _FindPos,
					});

					_Actions.Add(new SA_Action()
					{
						PrintbarNo	= color,
						ColorBrush  = colorBrush,
						Function = ECamFunction.CamFindFirstAngle,
						Name="Find First Angle",
					//	WebMoveDist = 12.0,
					//	WebPos		= 12.0,
						WebMoveDist = 0,
						WebPos		= 0,
						ScanPos	    = _FindPos,
					});

					if (!_Confirmed && color==0)
					{
						_Actions.Add(new SA_Action()
						{
							Function = ECamFunction.CamConfirmFocus,
							Name="Confirm",
							WebMoveDist = 0,
							WebPos = 0
						});
					}

				//	_HeadsPerColor = RxGlobals.PrintSystem.HeadsPerColor;
					string colorName = new ColorCode_Str().Convert(ink.ColorCode, null, color*RxGlobals.PrintSystem.InkCylindersPerColor, null).ToString();
					for (n=0; n<_HeadsPerColor; n++)
					{
						SA_Action action=new SA_Action()
						{
							PrintbarNo	= color,
							ColorBrush  = colorBrush,
							StepperNo   = color/2,
							HeadNo		= n,
							WebMoveDist = n==0? 12.0 : 0,
							WebPos		= 12.0,
							Function	= ECamFunction.CamMeasureAngle,
							Name		= String.Format(" {0}-{1}",  colorName, n+1),
						};						
						_Actions.Add(action);
					}
					_StitchIdxes[color] = _Actions.Count();
					for (n=0; n<_HeadsPerColor-1; n++)
					{
						SA_Action action=new SA_Action()
						{
							PrintbarNo	= color,
							ColorBrush  = colorBrush,
							StepperNo   = color/2,
							HeadNo		= n,
							Function	= ECamFunction.CamMeasureStitch,
							Name		= String.Format(" {0}-{1}..{2}",  colorName, n+1, n+2),
							WebMoveDist	= n==0? 10.0 : 0,
							WebPos		= 22.0,
						};						
						_Actions.Add(action);
					}

					_DistIdxes[color] = _Actions.Count();
					for (n=0; n<_HeadsPerColor-1; n++)
					{
						SA_Action action=new SA_Action()
						{
							PrintbarNo	= color,
							ColorBrush  = colorBrush,
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
			_Adjusted	= false;

			_Actions = new List<SA_Action>();
			if (_SimuMachine==false)
			{
				_Actions.Add(new SA_Action(){Name="Print Image"});

				_Actions.Add(new SA_Action()
				{
					Function = ECamFunction.CamFindLines_Vertical,
					Name="Find 3 Vert Lines",
					ScanPos	= _FindPos,
				});

				_Actions.Add(new SA_Action()
				{
					Function = ECamFunction.CamFindLine_Horzizontal,
					Name="Find Horiz Line",
					WebMoveDist = 0,
					ScanPos	    = _FindPos,
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
						WebMoveDist = 0,
						WebPos = 0
					});
				}
			}

			if (!RxGlobals.I1Pro3.IsWhiteCalibrated)
			{
				_Actions.Add(new SA_Action()
				{
					Function = ECamFunction.I1Calibrate,
					Name="White Calibration",
				//	WebMoveDist = 12.0,
					WebMoveDist = 0,
					WebPos = 12.0
				});
			}

			_Jet=128+64;
			for (n=0; n<2*_DensitiyDist.Length; n++)
			{
				Console.WriteLine("DensityDist[{0:F00}] dist={1:F000}, jet={2:F0000}, jet={3:F000} ", n+3, _DensitiyDist[n%_DensitiyDist.Length], _Jet, _Jet%2048);
				_Jet+=_DensitiyDist[n%_DensitiyDist.Length];
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
							WebMoveDist = (d==0)? px2mm(280) : px2mm(400),
							PrintbarNo	= color,
							StepperNo   = color/2,
							HeadNo		= d,
							Function	= ECamFunction.I1Measure,
							Name		= String.Format(" {0} {1}%",  colorName, _DensitiyLevels[d]),
						};						
						_Actions.Add(action);
					}
				}
			}
			_DistStepCnt = 0;
			_ActionIdx	 = 0;
			_HeadNo		 = 0;
			_StartAction();
			return _Actions;
		}
		//--- _InitActions -----------------------------
		public List<SA_Action> StartRegister()
		{
			int color, n;

			for (n=0; n<_RobotRunning.Length; n++) _RobotRunning[n]=false;

			_AssistMode = ENAssistMode.register;
			_Adjusted	= false;

			_Actions = new List<SA_Action>();
			_Actions.Add(new SA_Action(){Name="Print Image"});
			
			if (_SimuMachine==false)
			{
				_Actions.Add(new SA_Action()
				{
					Function = ECamFunction.CamFindLines_Vertical,
					Name="Find 3 Vert Lines",
					ScanPos	= _FindPos,
				});

				_Actions.Add(new SA_Action()
				{
					Function = ECamFunction.CamFindLine_Horzizontal,
					Name="Find Horiz Line",
					WebMoveDist = 0,
					ScanPos	    = _FindPos,
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
						WebMoveDist = 0,
						WebPos = 0
					});
				}
			}
			_DistStepCnt = 0;
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
						_Actions[_DistIdx+i].ScanPos = _Actions[_DistIdx+i-1].ScanPos+px2mm(2000);
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

			Console.WriteLine("CALLBACK 1: info={0} Action[{1}].function={2}", CallBackInfo.ToString(), _ActionIdx, _Action?.Function.ToString());


			callbackMutex.WaitOne();

			if (_Action!=null)
			{
				Console.WriteLine("CALLBACK: info={0} Action[{1}].function={2}, pos={3}, cnt={4}, NumMeasures={5}, value={6}, DPosX={7}, DPosY={8}", CallBackInfo.ToString(), _ActionIdx, _Action.Function.ToString(), RxGlobals.SetupAssist.ScanPos,  _Action.MeasureCnt, CallBackData.NumMeasures, CallBackData.Value_1, CallBackData.DPosX, CallBackData.DPosY);
				if (CallBackData.DPosX.Equals(float.NaN)) CallBackData.DPosX=0;
				if (CallBackData.DPosY.Equals(float.NaN)) CallBackData.DPosY=0;
				switch(_Action.Function)
				{
					case ECamFunction.CamFindLines_Vertical: // CallBackfromCam
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

					case ECamFunction.CamFindLine_Horzizontal: // CallBackfromCam
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
					case ECamFunction.CamFindFirstAngle:  // CallBackfromCam
							_CallbackData = CallBackData;
							CallbackInfo=string.Format("# {0}\n", ++_CallbackNo) 
										+ "Measure Angle\n"
										+ string.Format("  Correction: X={0:0.00}\n", CallBackData.Value_1)
										+ string.Format("  Center position: X={0:0.00}, y={1:0.00}\n", CallBackData.DPosX, CallBackData.DPosY);

							if (CallBackInfo==RxCam.ENCamCallBackInfo.AngleCorr && !Equals(CallBackData.Value_1, float.NaN))
							{
								Console.WriteLine("Mark Found: ScanStop");
								RxGlobals.SetupAssist.ScanStop();
								_MarkFound=true;
							}
							handled=true;
							break;

					case ECamFunction.CamConfirmFocus: // CallBackfromCam
							_CallbackData = CallBackData;
							CallbackInfo=string.Format("# {0}\n", ++_CallbackNo) 
										+ "Measure Angle\n"
										+ string.Format("  Correction: X={0:0.00}\n", CallBackData.Value_1)
										+ string.Format("  Center position: X={0:0.00}, y={1:0.00}\n", CallBackData.DPosX, CallBackData.DPosY);
						break;

					//--- Measure angle ----------------------------------------------------
					case ECamFunction.CamMeasureAngle: // CallBackfromCam
							_CallbackData = CallBackData;
							CallbackInfo=string.Format("# {0}\n", ++_CallbackNo) 
										+ "Measure Angle\n"
										+ string.Format("  Correction: {0:0.00} cnt={1}\n", CallBackData.Value_1, CallBackData.NumMeasures)
										+ string.Format("  Center position: X={0:0.00}, y={1:0.00}\n", CallBackData.DPosX, CallBackData.DPosY);

							bool next=false;
							if (_CamMeasureAngle_Step==1)
							{
								_CamFunctions.Off();
								if (CallBackInfo==RxCam.ENCamCallBackInfo.AngleCorr && !Equals(CallBackData.Value_1, float.NaN))
								{
									Console.WriteLine("{0} Callback: _CamMeasureAngle_Step=1, Cnt={1}", RxGlobals.Timer.Ticks(), _Action.MeasureCnt);
									_CamMeasureAngle_Step++;
									RxGlobals.SetupAssist.ScanStop();
									_MarkFound=true;
								}
								else
								{
									_MarkFound = false;
									_Action.Measured(float.NaN);
									next=true;
								}
							}
							else if (_CamMeasureAngle_Step==3)
							{
								if (CallBackInfo==RxCam.ENCamCallBackInfo.AngleCorr)
								{
									Console.WriteLine("Angle: corr[{0}]={1}, DPosX={2}", _Action.MeasureCnt, CallBackData.Value_1, CallBackData.DPosX);
									_Action.Measured(CallBackData.Value_1);
								}
								_Action.ScanPos = RxGlobals.SetupAssist.ScanPos - CallBackData.DPosX/1000;
								next=true;
							}

							if (next)
							{ 
								_CamFunctions.Off();
								_CamMeasureAngle_Step = 0;
								Console.WriteLine("Callback: _CamMeasureAngle_Step=3, Cnt={0}, move from {1} to {2}", _Action.MeasureCnt, RxGlobals.SetupAssist.ScanPos, RxGlobals.SetupAssist.ScanPos+2*_AngleDist-0.5);
								if (_Action.MeasureCnt >= 8)
								{
									Console.WriteLine("CALLBACK idx={0}, _CamMeasureAngle_done", _ActionIdx);
									_CamMeasureAngle_done();
									_HeadNo++;
								}
								if (_HeadNo<_HeadsPerColor) 
								{
									_Action.ScanMoveDone=false;
									if (CallBackInfo==RxCam.ENCamCallBackInfo.AngleCorr)
										RxGlobals.SetupAssist.ScanMoveTo(RxGlobals.SetupAssist.ScanPos+2*_AngleDist-0.5, 500);
									else
										RxGlobals.SetupAssist.ScanMoveTo(RxGlobals.SetupAssist.ScanPos+2*_AngleDist-1.0, 500); // mark not found
								}
								else _HeadNo=0;
							}
							handled=true;
							break;

					//--- measure stitch ----------------------------------------------------------------------------
					case ECamFunction.CamMeasureStitch: // CallBackfromCam
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
								RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
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
									RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
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
					case ECamFunction.CamMeasureDist: // CallBackfromCam
							action=_Actions[_DistIdx+_HeadNo];
							double offset=-((_DistStepCnt/4)-2) * px2mm(18);
							CallbackInfo=string.Format("# {0}\n", ++_CallbackNo) 
										+ "Measure Dist\n"
										+ string.Format("  Correction: X={0:0.00} offset={1:0.00}\n", CallBackData.Value_1, offset)
										+ string.Format("  Center position: X={0:0.00}, y={1:0.00}\n", CallBackData.DPosX, CallBackData.DPosY);
							Console.WriteLine("Step {0}, WebPos={1:0.000}: {2}", _DistStepCnt, RxGlobals.SetupAssist.WebPos, CallbackInfo);

							_CamFunctions.Off();
							if (CallBackInfo!=RxCam.ENCamCallBackInfo.RegisterCorr)
								break;
							action.Measured(CallBackData.Value_1);

							if (!float.IsNaN(CallBackData.Value_1))
							{
								action.Correction=offset+CallBackData.Value_1/1000;
								Console.WriteLine("Measure Dist: corr={0:0.00}", action.Correction);
								action.State = ECamFunctionState.done;
								_DistMeasureCnt++;
								if (_DistMeasureCnt+1==_HeadsPerColor)
								{
									_CamMeasureDist_done();
									break;
								}
							}

							//--- find next head ----------------
							while(true)
							{
								_HeadNo++;
								if (_HeadNo+1==_HeadsPerColor) // end of line
								{								
									if (_DistStepCnt==25) // end of measurment
									{										
										Abort();
										break;
									}
									// new line 
									_HeadNo = 0;
									_Action=_Actions[_DistIdx+_HeadNo];
									_DistStepCnt++;
									action.WebMoveDone = false;
									RxGlobals.SetupAssist.WebMove(_DistWebDist);
									RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
									break;
								}
								if (_HeadNo+1<_HeadsPerColor && _Actions[_DistIdx+_HeadNo].State!=ECamFunctionState.done)
								{
									_Action=_Actions[_DistIdx+_HeadNo];
									RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
									break;
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

		//--- _I1Calibrated --------------------------------
		private void _I1Calibrated(bool ok)
		{
			if (RxGlobals.I1Pro3.IsWhiteCalibrated)	
				ActionDone();
			else Abort();
		}

		//--- _I1Measured --------------------------------
		private void _I1Measured(ColorConversion.SpectroResultStruct result)
		{
			if (_Action!=null)
			{
				_DensityResult.Add(result);
				_Action.Measured(result.CieLab.L);
				if (_Action.MeasureCnt==_HeadsPerColor*_DensitiyDist.Length-2)
				{
					_I1Measure_done();
				}
				else
				{
					if (_SimuMachine) _StartCamFunction();
					else
					{
						_Action.ScanMoveDone=false;
						Console.WriteLine("_I1Measured: cnt={0}, jet={1}, idx={2}, dist={3}", _Action.MeasureCnt, _Jet, (_Action.MeasureCnt-1) % _DensitiyDist.Length, _DensitiyDist[_Action.MeasureCnt % _DensitiyDist.Length]);
						_Jet += _DensitiyDist[(_Action.MeasureCnt-1) % _DensitiyDist.Length];
						RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos+px2mm(_Jet), 500);
					}
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

				case ECamFunction.I1Calibrate:			_I1Calibrate_start();			break;
				case ECamFunction.I1Measure:			_I1Measure_start();				break;
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

				ECamFunction function=_Action.Function;
				_Action = null;
				_ActionIdx++;
				if (_ActionIdx<_Actions.Count())
				{
					if (_Actions[_ActionIdx].Function==function)
					{
						_StartAction();
						return;
					}
					else if (_Adjusted)
					{
						RxGlobals.SetupAssist.ScanReference();
						Start();
						return;
					}
					_StartAction();
					return;
				}
				RxGlobals.SetupAssist.ScanReference();
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
			switch(_AssistMode)
			{
			case ENAssistMode.align:	item.TestImage	= ETestImage.SA_Alignment; break;
			case ENAssistMode.density:	item.TestImage	= ETestImage.SA_Density; break;
			case ENAssistMode.register:	item.TestImage	= ETestImage.SA_Register; break;
			}
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

			_StitchIdx   = _StitchIdxes[_Action.PrintbarNo];
			_DistIdx	 = _DistIdxes[_Action.PrintbarNo];

			_MarkFound = false;
			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
		}

		//--- _CamFindLine_Horzizontal_start ----------------------------------------------
		private void _CamFindLine_Horzizontal_start()
		{
			_MarkFound=false;
			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			Console.WriteLine("{0}: _CamFindLine_Horzizontal_start, MarkFound={1}", RxGlobals.Timer.Ticks(), _MarkFound);			
		//	RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
		//	RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
			_Action.ScanMoveDone = true;
			_Action.WebMoveDone  = true;
			_StartCamFunction();			
		}

		//--- _CamFindFirstAngle_start -----------------------
		private void _CamFindFirstAngle_start()
		{
		//	Console.WriteLine("{0}: Action[{1}]: _CamFindFirstAngle_start, ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
		//	RxGlobals.SetupAssist.ScanReference();
		//	RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
			_Action.ScanMoveDone = true;
			_Action.WebMoveDone  = true;
			_StartCamFunction();
		}

		//--- _CamConfirmFocus_start ----------------------------------------------
		private void _CamConfirmFocus_start()
		{
			if (_Confirmed) ActionDone();
			else if (_AssistMode == ENAssistMode.density)
			{
				_Action.ScanMoveDone=true;
				_Action.WebMoveDone =true;
			}
		}

		//--- _CamMeasureAngle_start ----------------------------------------------
		private void _CamMeasureAngle_start()
		{
		//	RxGlobals.Events.AddItem(new LogItem("_CamMeasureAngle_start: stopPos={0}", RxGlobals.SetupAssist.ScanStopPos));

			Console.WriteLine("{0}: _CamMeasureAngle_start Action[{1}]: ScannerPos={2}, ScanPos={3}, WebMoveDist={4}, ", RxGlobals.Timer.Ticks(), _ActionIdx, RxGlobals.SetupAssist.ScanPos, _Action.ScanPos, _Action.WebMoveDist);
			_Confirmed = true;
			if (_Actions[_ActionIdx-1].Function==ECamFunction.CamConfirmFocus || _Actions[_ActionIdx-1].Function==ECamFunction.CamFindFirstAngle)
			{
				if (true) //--- skip first measurement, probably out of scanner range
				{
					_Action.Measured(float.NaN);
					_Action.ScanPos = RxGlobals.SetupAssist.ScanPos;// + _AngleDist;
				}

				RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
				RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);

				_CamMeasureAngle_Step = 3;
			}
			else
			{
				_CamMeasureAngle_Step = 0;
			}
			Console.WriteLine("_CamMeasureAngle_start: _CamMeasureAngle_Step={0}", _CamMeasureAngle_Step);
		}

		//--- _CamMeasureStitch_start ----------------------------------------------
		private void _CamMeasureStitch_start()
		{
		//	RxGlobals.Events.AddItem(new LogItem("_CamMeasureStitch_start"));

			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
			RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
		}

		//--- _CamMeasureDist_start ----------------------------------------------
		private void _CamMeasureDist_start()
		{
		//	RxGlobals.Events.AddItem(new LogItem("_CamMeasureDist_start"));

			_DistStepCnt = 0;
			_DistMeasureCnt = 0;
			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);

			Console.WriteLine("{0}: Action[{1}]: Move web from {2} to {3} dist={4}", RxGlobals.Timer.Ticks(), _ActionIdx, RxGlobals.SetupAssist.WebPos, _Action.WebPos, _Action.WebPos-RxGlobals.SetupAssist.WebPos);

			RxGlobals.SetupAssist.WebMove(_Action.WebPos-RxGlobals.SetupAssist.WebPos);
		}

		//--- _CamMeasureDist_done -----------------------------
		private void _CamMeasureDist_done()
		{
		//	RxGlobals.Events.AddItem(new LogItem("Camera: Send Stitch Correction {0} to head{1}", _Action.Correction, _Action.HeadNo));
			double corr=0;
			for(int i=0; i+1<_HeadsPerColor; i++)
			{
				SA_Action action=_Actions[_DistIdx+i];
				if (action.Correction!=null && Math.Abs((double)action.Correction)>0.010) 
				{
					corr += (double)action.Correction;
					RxGlobals.PrintSystem.HeadDist[action.PrintbarNo*RxGlobals.PrintSystem.HeadsPerColor+ action.HeadNo+1] -= corr;
				}
			}
			RxGlobals.PrintSystem.SendMsg(TcpIp.CMD_SET_PRINTER_CFG);
			ActionDone();
		}


		//--- _I1Calibrate_start -------------------
		private void _I1Calibrate_start()
		{
			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			_Action.ScanMoveDone = true;
			_Action.WebMoveDone  = true;
			_StartCamFunction();
		}

		//--- _I1Measure_start -------------------
		private void _I1Measure_start()
		{
			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			_DensityResult = new List<ColorConversion.SpectroResultStruct>();
			_Jet=128+64;
			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos+px2mm(_Jet));
			RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
		}

		//--- _I1Measure_done ----------------------------------------------------
		private void _I1Measure_done()
		{
			_DensityResults.Add(_DensityResult);
			if (_DensityResults.Count==_DensitiyLevels.Length)
			{
				DensityCalc calc = new DensityCalc();

				InkType ink = RxGlobals.InkSupply.List[_Action.PrintbarNo*RxGlobals.PrintSystem.InkCylindersPerColor].InkType;
				bool isYellow = (ink.ColorCode==3);
				List<List<int>> currentDensityList = new List<List<int>>();
				List<List<int>> newDensityList;

				//--- prepare ---------------------------------
				for (int head=0; head<_HeadsPerColor; head++)
				{
					List<int> values = new List<int>();
					HeadStat stat = RxGlobals.HeadStat.List[_Action.PrintbarNo*RxGlobals.PrintSystem.HeadsPerColor + head];
					foreach(short val in stat.DensityValue) values.Add((int)val);
					currentDensityList.Add(values);
				}

				//--- calculate ----------------------
				ColorConversion.CieLabStruct[] avg = calc.CalcAverageCie(_DensityResults, isYellow);
				List<List<double>>	deList		   = calc.CalcWeightedDeList(_DensityResults, avg, isYellow);
				List<List<double>>	deList12	   = calc.CalcAverageTo12Points(deList, _HeadsPerColor);
				newDensityList = calc.CalcNewDensities(_HeadsPerColor, currentDensityList, deList12);

				//--- save ----------------------------------------------------------
				for (int head=0; head<RxGlobals.PrintSystem.HeadsPerColor; head++)
				{
					HeadStat stat = RxGlobals.HeadStat.List[_Action.PrintbarNo*RxGlobals.PrintSystem.HeadsPerColor + head];

					TcpIp.SDensityMsg msg = new TcpIp.SDensityMsg();

					RxGlobals.DisabledJets.Save(ref msg);
					RxGlobals.Density.Save(ref msg);
					int cnt = newDensityList[0].Count;
					msg.head = _Action.PrintbarNo*RxGlobals.PrintSystem.HeadsPerColor + head;
					msg.data.voltage		= stat.Voltage;
					msg.data.disabledJets	= stat.DisabledJets;
					msg.data.densityValue	= new Int16[cnt];

					for (int i = 0; i < cnt; i++)
					{
						stat.DensityValue[i]     = (short)newDensityList[head][i];
						msg.data.densityValue[i] = (short)newDensityList[head][i];
					}

					RxGlobals.RxInterface.SendMsg(TcpIp.CMD_SET_DENSITY, ref msg);
				}
			}
			ActionDone();
		}

		//--- _CamMeasureAngle_done -----------------------------
		private void _CamMeasureAngle_done()
		{
		//	RxGlobals.Events.AddItem(new LogItem("Camera: Send Angle Correction {0} to head{1}", _Action.Correction, _Action.HeadNo));
			_Action.State = ECamFunctionState.waitRob;
			_NextRobotCmd(_Action.StepperNo);

			if (_ActionIdx+1<_Actions.Count && _Actions[_ActionIdx+1].Function==ECamFunction.CamMeasureAngle && _Actions[_ActionIdx+1].HeadNo>0)
			{
				_Actions[_ActionIdx+1].ScanPos = _Actions[_ActionIdx].ScanPos+2*_AngleDist;

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

		//--- _ScanMoveDone -------------------------------
		private void _ScanMoveDone()
		{
			Console.WriteLine("{0}: Action[{1}]: _ScanMoveDone", RxGlobals.Timer.Ticks(), _ActionIdx);
			if (_Action==null) return;
			_Action.ScanMoveDone=true;

			switch(_Action.Function)
			{
			case ECamFunction.CamFindLines_Vertical: // _ScanMoveDone
					RxGlobals.SetupAssist.Down(); 
					break;

			case ECamFunction.CamFindFirstAngle: // _ScanMoveDone
					Console.WriteLine("{0}: CamFindFirstAngle: _ScanMoveDone motorPos={1}, stopPos={2}, DPosX={3}", RxGlobals.Timer.Ticks(), RxGlobals.SetupAssist.ScanPos, RxGlobals.SetupAssist.ScanStopPos, _CallbackData.DPosX);
					if (_MarkFound) 
					{
						for (int idx=_ActionIdx; idx<_Actions.Count; idx++)
						{
							SA_Action action=_Actions[idx];
							switch(action.Function)
							{
								case ECamFunction.CamMeasureAngle:  RxGlobals.SetupAssist.ScanMoveTo(RxGlobals.SetupAssist.ScanPos + _CallbackData.DPosX/1000); idx=1000; break;
								case ECamFunction.I1Measure:  action.ScanPos = RxGlobals.SetupAssist.ScanPos + _CallbackData.DPosX/1000 - px2mm(286); break;
								default: break;
							}
						}
						ActionDone();
					}
					else
					{
						Console.WriteLine("{0}: Mark NOT Found", RxGlobals.Timer.Ticks());
						_Action.State=ECamFunctionState.error;
						Abort();
						return;
					}
					break;

			case ECamFunction.CamConfirmFocus: // _ScanMoveDone
					Console.WriteLine("Action[{0}].CamConfirmFocus, ScanPos={1}", _ActionIdx, RxGlobals.SetupAssist.ScanPos);
					/*
					if (RxGlobals.SetupAssist.ScanPos+2<_Action.ScanPos)
					{
						_Action.ScanMoveDone=false;
						RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
					}
					else 
					*/
					_Action.ConfirmVisibile = true;
					break;

			case ECamFunction.CamMeasureAngle: // _ScanMoveDone
					Console.WriteLine("Action[{0}].CamMeasureAngle, HeadNo={1}, ScanPos={2}, _CamMeasureAngle_Step={3}", _ActionIdx, _Action.HeadNo, RxGlobals.SetupAssist.ScanPos, _CamMeasureAngle_Step);
					switch(_CamMeasureAngle_Step)
					{
						case 0: _CamMeasureAngle_Step=1;
								_CamFunctions.MeasureAngle(true);
								_Action.ScanMoveDone=false;
								RxGlobals.SetupAssist.ScanMoveTo(RxGlobals.SetupAssist.ScanPos+1.0, 100);
								break;

								
						case 1:	//--- mark not found !
								RxCam.ENCamCallBackInfo CallBackInfo = new ENCamCallBackInfo(); 
								RxCam.CallBackDataStruct CallBackData = new CallBackDataStruct();
								CallBackfromCam(CallBackInfo, CallBackData);
								break;

						case 2: _CamMeasureAngle_Step = 3;
								break;
					}
					break;

			case ECamFunction.CamMeasureStitch: // _ScanMoveDone
					/*
					if (_Action.HeadNo==0 && !_SimuMachine)
					{
						Console.WriteLine("Action[{0}].CamMeasureStitch, HeadNo={1}, ScanPos={2}", _ActionIdx, _Action.HeadNo, RxGlobals.SetupAssist.ScanPos);
						if (RxGlobals.SetupAssist.ScanPos+2<_Action.ScanPos)
						{
							_Action.ScanMoveDone=false;
							RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
						}
					}
					*/
					break;

			case ECamFunction.CamMeasureDist: // _ScanMoveDone
					if (_Action.HeadNo==0 && !_SimuMachine)
					{
						Console.WriteLine("Action[{0}].CamMeasureDist, HeadNo={1}, ScanPos={2}", _ActionIdx, _Action.HeadNo, RxGlobals.SetupAssist.ScanPos);
						if (RxGlobals.SetupAssist.ScanPos+2<_Action.ScanPos)
						{
							_Action.ScanMoveDone=false;
							RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
						}
					}
					break;
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

					/*
					case ECamFunction.CamFindFirstAngle:
														if (_Action.ScanMoveDone)
															_StartCamFunction();
														break;
					*/

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
						_MarkFound=false;
						result=_CamFunctions.MeasureAngle(true);
						if (result!=RxCam.ENCamResult.OK) RxGlobals.Events.AddItem(new LogItem("MeasureAngle Error {0}", result.ToString()));
						Console.WriteLine("{0}: Action[{1}]: Start MeasureAngle, ScanMoveTo(0, 100)", RxGlobals.Timer.Ticks(), _ActionIdx);
						_Action.ScanMoveDone=false;
						RxGlobals.SetupAssist.ScanMoveTo(0, 100);
						break;

					case ECamFunction.CamConfirmFocus:
						_Action.ConfirmVisibile = true;
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

					case ECamFunction.I1Calibrate:
						RxGlobals.I1Pro3.WhiteCalibrate(_I1Calibrated);						
						break;

					case ECamFunction.I1Measure:
						RxGlobals.I1Pro3.MeasurePoint(_I1Measured);
						break;

					default: 
						RxGlobals.Events.AddItem(new LogItem("Unknown Camera function")); 
						break;
				}
			}
		}
	}
}
