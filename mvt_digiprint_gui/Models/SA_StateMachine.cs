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

		public bool				_SimuCamera  = false;
		public bool				_SimuMachine = false;

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

		private const int		 _MaxPrintbars=8;

		private DateTime		 _TimePrimted;
		private bool			 _SA_Running = false;
		private ENAssistMode	 _AssistMode=ENAssistMode.undef;
		private RxCamFunctions	 _CamFunctions;
		private List<SA_Action>  _Actions;
		private int				 _ActionIdx;
		private SA_Action		 _Action;
		private int[]			 _AngleIdxes = new int[_MaxPrintbars];
		private int[]			 _StitchIdxes=new int[_MaxPrintbars];
		private int[]			 _DistIdxes=new int[_MaxPrintbars];
		private ECamFunction[]	 _AdjustFunction=new ECamFunction[_MaxPrintbars];
		private int				 _HeadNo;
		private int				 _MeasurementPass;
		private readonly double  _FindPos		= 30.0;
		private readonly double	 _AngleDist		= px2mm(128);
		private readonly double	 _StitchWebDist = px2mm(2*140);
		private readonly double	 _DistWebDist   = Math.Round((24.0*25.4)/1200, 3);	// 
		private readonly double	 _AlignmaneBmp_Height = Math.Round((6400*25.4)/1200, 3);
		private bool			 _SkipFct=false;
		private int				 _DistStepCnt   = 0;
		private int				 _DistMeasureCnt= 0;
		private bool			 _Confirmed		= false;
		private int				 _CamMeasureAngle_Step;
		private RxCam.CallBackDataStruct	_CallbackData;
		private int				 _StopTime;
		private bool[]			 _RobotRunning= new bool[4];
		private bool[]			 _RobotUsed=new bool[4];
		private bool			 _Adjusted;

		private readonly int	_CamMeasureRegStitch_Cnt=5;
		private readonly int[]	 _DensitiyLevels = { 20, 30, 40, 50};
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

			foreach (var state in RxGlobals.StepperStatus)
			{
				state.PropertyChanged += SA_StateMachine_PropertyChanged; 			
			}				
			_CamFunctions = new RxCamFunctions(RxGlobals.Camera);
			if (_SimuCamera) _CamFunctions.SimuCallback	+= CallBackfromCam;
		}

		//--- GetActions ------------------------
		public List<SA_Action> GetActions()
		{
			return _Actions;
		}

		//--- TimePrinted ----------------------------------
		public DateTime TimePrinted()
		{
			return _TimePrimted;
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
					if (e.PropertyName.Equals("AdjustDoneCnt") && _Actions!=null)
					{
						foreach(SA_Action action in _Actions)
						{	
							if (action.StepperNo==no && action.State==ECamFunctionState.runningRob)
							{
								_RobotRunning[no]=false;
								Console.WriteLine("ROBOT[{0}]: Screwing done, ok={1}", no, stepper.ScrewedOk);
								if (stepper.ScrewedOk) action.State = ECamFunctionState.done;
								else				   action.State = ECamFunctionState.error;
								break;
							}
						}

						_NextRobotCmd(no);
					}
					if (e.PropertyName.Equals("ScrewerReady"))
					{
						_NextRobotCmd(no);
					}
					return;
				}
			}
		}

		//--- _NextRobotCmd ---------------------------------------
		private void _NextRobotCmd(int stepperNo)
		{
			if (_RobotRunning[stepperNo]) return;

			if (!RxGlobals.StepperStatus[stepperNo].ScrewerReady) return;
			if (_Actions!=null)
			{
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
							/*
							action.State	= ECamFunctionState.done;
							RxGlobals.Events.AddItem(new LogItem(string.Format("ROB Command, Head={0}, axis={1}, steps={2} NOT STARTING (Development)", msg.headNo, msg.steps, msg.steps)));
							break;
							*/

							// _Adjusted = true;
							_RobotRunning[stepperNo] = true;
							_RobotUsed[stepperNo] = true;

							action.State	= ECamFunctionState.runningRob;
							Console.WriteLine("Stepper[{0}]: CMD_HEAD_ADJUST Printbar={1}, Head={2}, axis={3}, steps={4}", stepperNo, msg.printbarNo, msg.headNo, msg.axis, msg.steps);
							RxGlobals.Events.AddItem(new LogItem(string.Format("ROB Command, Printbar={0}, Head={1}, axis={2}, steps={3}", msg.printbarNo, msg.headNo, msg.axis, msg.steps)));
							RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_ADJUST, ref msg);
						}
						return;
					}
				}
			}
			if (_RobotUsed[stepperNo] && !_RobotRunning[stepperNo])
			{
				TcpIp.SValue msg = new TcpIp.SValue();
				msg.no=stepperNo;
				RxGlobals.RxInterface.SendMsg(TcpIp.CMD_ROBI_MOVE_TO_GARAGE, ref msg);
				_RobotUsed[stepperNo] = false;	
			}
			if (_Adjusted)
			{
				RxGlobals.Events.AddItem(new LogItem("ROB DONE"));
				/*
				if (_Action==null)
				{
					StartAlign();
					RxGlobals.Events.AddItem(new LogItem("Print again"));
				}
				*/
			}
			if (_Actions==null) Abort();
		}

		//--- Property SimuMachine ---------------------------------------
		public bool SimuMachine
		{
			get 
			{
				if (RxGlobals.PrintSystem.HostName!=null)
				{
					_SimuMachine = RxGlobals.PrintSystem.HostName.Equals("TEST-0001")
							    || RxGlobals.PrintSystem.HostName.Equals("MAG-Notebook-17");

					RxGlobals.SetupAssist.Simu = _SimuMachine;

					if (!_SimuCamera && _SimuMachine)
					{
						_SimuCamera = _SimuMachine;
						_CamFunctions.SimuCallback += CallBackfromCam;
					}
				}
				return _SimuMachine; 
			}
		}

//		private Random _Random = new Random();

		//--- _InitActions -----------------------------
		public List<SA_Action> StartAlign()
		{
			int color, n;
			bool confirm_added=false;
			bool first=true;

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

			if (_SimuMachine) RxGlobals.Events.AddItem(new LogItem("SA_StateMachine: SimuMachine"));

			for (n=0; n<_RobotRunning.Length; n++) 
			{
				_RobotRunning[n]=false;
				_RobotUsed[n]  = false;
			}

			_AssistMode = ENAssistMode.align;
			_Adjusted   = false;

			_Actions = new List<SA_Action>();
			_Actions.Add(new SA_Action(){Name="Print Image"});

			_SkipFct = false;

			//--- measurmentfunctions -----------------------------
		//	for (color=0; color<RxGlobals.PrintSystem.ColorCnt; color++)
			for (int i=RxGlobals.PrintSystem.ColorCnt-1; i>=0; i--)	// reverse order!
			{
				color = RxGlobals.PrintSystem.Color_Order[i];
				InkType ink = RxGlobals.InkSupply.List[color*RxGlobals.PrintSystem.InkCylindersPerColor].InkType;
				_AdjustFunction[color]= ECamFunction.CamPrint;
				if (ink!=null)
				{
					if (first)
					{
						_Actions.Add(new SA_Action()
						{
							PrintbarNo	= color,
							ColorBrush  = ink.ColorBrush,
							StrokeBrush = ink.StrokeBrush,
							guiCtrl = ink.guiCtrl,
							WebMoveDist = first? 200 : 20,
							WebMoveDone = true,
							Function = ECamFunction.CamFindLines_Vertical,
							Name="Find 3 Vert Lines",
							ScanPos	= _FindPos,
						});
					}

					_Actions.Add(new SA_Action()
					{
						PrintbarNo	= color,
						ColorBrush  = ink.ColorBrush,
						StrokeBrush = ink.StrokeBrush,
						guiCtrl = ink.guiCtrl,
						Function = ECamFunction.CamFindLine_Horzizontal,
						Name="Find Horiz Line",
						WebMoveDist = 0,
						ScanPos	    = _FindPos,
					});

					_Actions.Add(new SA_Action()
					{
						PrintbarNo	= color,
						ColorBrush  = ink.ColorBrush,
						StrokeBrush = ink.StrokeBrush,
						guiCtrl = ink.guiCtrl,
						Function = ECamFunction.CamFindFirstAngle,
						Name="Find First Angle",
						WebMoveDist = 0,
						WebPos		= 0,
						ScanPos	    = _FindPos,
					});

					if (!_Confirmed && !confirm_added)
					{
						confirm_added = true;
						_Actions.Add(new SA_Action()
						{
							Function = ECamFunction.CamConfirmFocus,
							Name="Confirm",
							WebMoveDist = 0,
							WebPos = 0
						});
					}

					_HeadsPerColor = RxGlobals.PrintSystem.HeadsPerColor;
					string colorName = new ColorCode_Str().Convert(ink.ColorCode, null, color*RxGlobals.PrintSystem.InkCylindersPerColor, null).ToString();
					_AngleIdxes[color] = _Actions.Count();
					if (RxGlobals.SA_AlignSettings.MeasureAngle[i])
					{
						for (n=0; n<_HeadsPerColor; n++)
						{
							SA_Action action=new SA_Action()
							{
								PrintbarNo	= color,
								ColorBrush  = ink.ColorBrush,
								guiCtrl = ink.guiCtrl,
								StrokeBrush = ink.StrokeBrush,
								StepperNo   = color/2,
								HeadNo		= n,
								WebMoveDist = n==0? 12.0 : 0,
								WebPos		= 12.0,
								Function	= ECamFunction.CamMeasureAngle,
								Name		= String.Format("{0}-{1}", colorName, n+1),
							};
							_Actions.Add(action);
							if (_SimuMachine) action.ReadCsv("D:\\Temp\\alignment.csv");
						}
						_Actions.Add(new SA_Action()
						{
							PrintbarNo = color,
							ColorBrush = ink.ColorBrush,
							StrokeBrush = ink.StrokeBrush,
							guiCtrl = ink.guiCtrl,
							StepperNo = color / 2,
							WebMoveDist = 0,
							WebPos = 0,
							Function = ECamFunction.CamMoveScan,
							Name = "Repeat",
						});

						_Actions.Add(new SA_Action()
						{
							PrintbarNo = color,
							Function = ECamFunction.CamDummy,
							Name = "Dummy",
						});

					}

					_StitchIdxes[color] = _Actions.Count();
					if (RxGlobals.SA_AlignSettings.MeasureStitch[i])
					{
						for (n=0; n<_HeadsPerColor-1; n++)
						{
							SA_Action action=new SA_Action()
							{
								PrintbarNo	= color,
								ColorBrush  = ink.ColorBrush,
								StrokeBrush = ink.StrokeBrush,
								guiCtrl		= ink.guiCtrl,
								StepperNo   = color/2,
								HeadNo		= n,
								Function	= ECamFunction.CamMeasureStitch,
								Name		= String.Format("{0}-{1}..{2}",  colorName, n+1, n+2),
								WebMoveDist	= n==0? 10.0 : 0,
								WebPos		= 22.0,
							};						
							_Actions.Add(action);
						}
					}

					_DistIdxes[color] = _Actions.Count();
					if (false && first)
					{ 
						for (n=0; n<_HeadsPerColor-1; n++)
						{
							_Actions.Add(new SA_Action()
							{
								PrintbarNo	= color,
								ColorBrush  = ink.ColorBrush,
								StrokeBrush = ink.StrokeBrush,
								guiCtrl = ink.guiCtrl,
								StepperNo   = color/2,
								HeadNo		= n,
								Function	= ECamFunction.CamMeasureDist,
								Name		= String.Format(" {0}-{1}..{2}",  colorName, n+1, n+2),
								WebMoveDist	= n==0? 7.5 : 0,
								WebPos		= 82.840,
							});				
						}
					}

					_Actions.Add(new SA_Action()
					{
						PrintbarNo = color,
						ColorBrush = ink.ColorBrush,
						StrokeBrush = ink.StrokeBrush,
						guiCtrl = ink.guiCtrl,
						StepperNo = color / 2,
						WebMoveDist = 0,
						WebPos = 130,
						ScanPos = _FindPos,
						Function = ECamFunction.CamMoveWeb,
						Name = "Move Web",
					});

					_Actions.Add(new SA_Action()
					{
						PrintbarNo = color,
						Function = ECamFunction.CamDummy,
						Name = "Dummy",
					});

					first = false;
				}
			}

			_ActionIdx	 = 0;
			_HeadNo		 = 0;
			_MeasurementPass = 0;
			if (!_SimuMachine) _StartAction();
			else _TimePrimted = DateTime.Now;
			return _Actions;
		}

		//--- _InitActions -----------------------------
		public List<SA_Action> StartDensity()
		{
			int color, n;
			bool confirm_added=false;

			for (n=0; n<_RobotRunning.Length; n++) _RobotRunning[n]=false;

			_AssistMode = ENAssistMode.density;
			_Adjusted	= false;

			_Actions = new List<SA_Action>();
		//	if (_SimuMachine==false)
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

				if (!_Confirmed && !confirm_added)
				{
					confirm_added = true;
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
					WebMoveDist = 0,
					WebPos = 0
				});
			}

			_Jet=128+64;
			for (n=0; n<2*_DensitiyDist.Length; n++)
			{
				Console.WriteLine("DensityDist[{0:F00}] dist={1:F000}, jet={2:F0000}, jet={3:F000} ", n+3, _DensitiyDist[n%_DensitiyDist.Length], _Jet, _Jet%2048);
				_Jet+=_DensitiyDist[n%_DensitiyDist.Length];
			}

			//--- measurmentfunctions -----------------------------
			for (int i=RxGlobals.PrintSystem.ColorCnt-1; i>=0; i--)	// reverse order!
			{
				color = RxGlobals.PrintSystem.Color_Order[i];
				InkType ink = RxGlobals.InkSupply.List[color*RxGlobals.PrintSystem.InkCylindersPerColor].InkType;
				if (ink!=null)
				{
					_HeadsPerColor = RxGlobals.PrintSystem.HeadsPerColor;
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
			_MeasurementPass=0;
			_StartAction();
			return _Actions;
		}
		//--- _InitActions -----------------------------
		public List<SA_Action> StartRegister()
		{
			int n, color;
			bool confirm_added=false;

			for (n=0; n<_RobotRunning.Length; n++) _RobotRunning[n]=false;

			_AssistMode = ENAssistMode.register;
			_Adjusted	= false;

			_Actions = new List<SA_Action>();
			_Actions.Add(new SA_Action(){Name="Print Image"});
			
			if (_SimuMachine==false)
			{
				for (int i = RxGlobals.PrintSystem.ColorCnt - 2; i >= 0; i--)   // reverse order!
				{
					color = RxGlobals.PrintSystem.Color_Order[i];
					InkType ink = RxGlobals.InkSupply.List[color * RxGlobals.PrintSystem.InkCylindersPerColor].InkType;
					if (ink != null)
					{
						_HeadsPerColor = RxGlobals.PrintSystem.HeadsPerColor;
						string colorName = new ColorCode_Str().Convert(ink.ColorCode, null, color * RxGlobals.PrintSystem.InkCylindersPerColor, null).ToString();
						Brush colorBrush;
						try
						{
							colorBrush = new SolidColorBrush(ink.Color);
						}
						catch (Exception)
						{
							colorBrush = Brushes.Transparent;
						}

						_Actions.Add(new SA_Action()
						{
							ColorBrush = colorBrush,
							Function = ECamFunction.CamFindLines_Vertical,
							Name ="Find 3 Vert Lines",
							ScanPos	= _FindPos,
						});

						_Actions.Add(new SA_Action()
						{
							ColorBrush = colorBrush,
							Function = ECamFunction.CamFindLine_Horzizontal,
							Name="Find Horiz Line",
							WebMoveDist = 0,
							ScanPos	    = _FindPos,
						});

						_Actions.Add(new SA_Action()
						{
							ColorBrush = colorBrush,
							Function = ECamFunction.CamFindFirstAngle,
							Name="Find First Angle",
							WebMoveDist = 12.0,
							WebPos		= 12.0,
							ScanPos	    = 0,
						});

						if (!_Confirmed && !confirm_added)
						{
							confirm_added=true;
							_Actions.Add(new SA_Action()
							{
								ColorBrush = colorBrush,
								Function = ECamFunction.CamConfirmFocus,
								Name="Confirm",
								WebMoveDist = 0,
								WebPos = 0
							});
						}
						_StitchIdxes[color] = _Actions.Count();
						for (n=0; n<_CamMeasureRegStitch_Cnt; n++)
						{
							SA_Action action = new SA_Action()
							{
								WebMoveDist = (n==0)? px2mm(320):0,
								ColorBrush = colorBrush,
								PrintbarNo = color,
								StepperNo = color / 2,
								HeadNo = 0,
								Function = ECamFunction.CamMeasureRegStitch,
								Name = colorName,
							};
							_Actions.Add(action);
						}
					}
				}
			}
			_DistStepCnt = 0;
			_ActionIdx	 = 0;
			_HeadNo		 = 0;
			_MeasurementPass=0;
			_StartAction();
			return _Actions;
		}

		//--- ConfirmFocus -------------------------------------
		public void ConfirmFocus()
		{
			if (_Action.Function==ECamFunction.CamConfirmFocus)
			{
				_CamFunctions.Off();
				_Action.ConfirmVisibile = false;
				
				if (false) // start at distance measurement
				{
					int distIdx = _DistIdxes[_Action.PrintbarNo];
					_Actions[distIdx].ScanPos = RxGlobals.SetupAssist.ScanPos+40;
					for (int i=1; i+1<_HeadsPerColor; i++)
						_Actions[distIdx + i].ScanPos = _Actions[distIdx + i-1].ScanPos+px2mm(2000);
					_ActionIdx = distIdx - 1;
				}
				ActionDone();
			}
		}

		//--- CallBackfromCam ----------------------------------------
		private int _CallbackNo=0;
		private static Mutex callbackMutex = new Mutex();
		private double _firstAnglePos;

		private void CallBackfromCam(RxCam.ENCamCallBackInfo CallBackInfo, RxCam.CallBackDataStruct CallBackData)
        {
			bool handled=false;
			bool ok;
			int stitchIdx;
			SA_Action action;

			Console.WriteLine("CALLBACK 1: info={0} Action[{1}].function={2}", CallBackInfo.ToString(), _ActionIdx, _Action?.Function.ToString());

			ok=callbackMutex.WaitOne(100);
			if (!ok)
			{
				Console.WriteLine("Callback Timeout");
				return;
			}

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
							else 
							{
								if (RxGlobals.SetupAssist.WebPos<3000)
									RxGlobals.SetupAssist.WebMove(1);
								else 
								{
									Console.WriteLine("{0}: Mark NOT Found", RxGlobals.Timer.Ticks());
									Abort();
								}
							}
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
							_firstAnglePos = 0;
							_HeadNo = 0;
							handled =true;
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
									_Action.Measured(_MeasurementPass, float.NaN);
									next=true;
								}
							}
							else if (_CamMeasureAngle_Step==3)
							{
								_CamFunctions.Off();
								Console.WriteLine("{0} Callback: _CamMeasureAngle_Step=3, Cnt={1}, value={2}", RxGlobals.Timer.Ticks(), _Action.MeasureCnt, CallBackData.Value_1);
								if (CallBackInfo==RxCam.ENCamCallBackInfo.AngleCorr)
								{
									Console.WriteLine("Angle: corr[{0}]={1}, DPosX={2}", _Action.MeasureCnt, CallBackData.Value_1, CallBackData.DPosX);
									_Action.Measured(_MeasurementPass, CallBackData.Value_1);
								}
								next=true;
							}

							if (next)
							{
								if (_Action.ScanPos == 0) _Action.ScanPos = RxGlobals.SetupAssist.ScanPos;
								if (_firstAnglePos == 0) _firstAnglePos= _Action.ScanPos;
								_CamFunctions.Off();
								Console.WriteLine("Callback: _CamMeasureAngle_Step={0}, Cnt={1}, move from {2} to {3}", _CamMeasureAngle_Step, _Action.MeasureCnt, RxGlobals.SetupAssist.ScanPos, RxGlobals.SetupAssist.ScanPos+2*_AngleDist-0.5);
								_CamMeasureAngle_Step = 0;
								if (_Action.MeasureCnt >= 8)
								{
									Console.WriteLine("CALLBACK idx={0}, _CamMeasureAngle_done", _ActionIdx);
									_CamMeasureAngle_done();
									_HeadNo++;									
								}
								Console.WriteLine("_HeadNo={0}, _HeadsPerColor={1}", _HeadNo, _HeadsPerColor);
								if (_HeadNo<_HeadsPerColor) 
								{
									_Action.ScanMoveDone = false;										
									if (CallBackInfo==RxCam.ENCamCallBackInfo.AngleCorr)
									{
										Console.WriteLine("SAN MOVE ok, pos={0}", RxGlobals.SetupAssist.ScanPos + 2 * _AngleDist - 0.5);
										RxGlobals.SetupAssist.ScanMoveTo(RxGlobals.SetupAssist.ScanPos+2*_AngleDist-0.5, 500);
									}
									else
									{
										Console.WriteLine("SAN MOVE err, pos={0}", RxGlobals.SetupAssist.ScanPos + 2 * _AngleDist - 1.0);
										RxGlobals.SetupAssist.ScanMoveTo(RxGlobals.SetupAssist.ScanPos + 2 * _AngleDist - 1.0, 500); // mark not found
									}
								}
								else _HeadNo = 0;
							}
							handled=true;
							break;

					//--- measure stitch ----------------------------------------------------------------------------
					case ECamFunction.CamMeasureStitch: // CallBackfromCam
							stitchIdx = _StitchIdxes[_Action.PrintbarNo];
							action=_Actions[stitchIdx + _HeadNo];
							if (CallBackInfo!=RxCam.ENCamCallBackInfo.StitchCorr)
								break;

							CallbackInfo=string.Format("# {0}\n", ++_CallbackNo) 
										+ "Measure Stitch\n"
										+ string.Format("  Correction: X={0:0.00}\n", CallBackData.Value_1)
										+ string.Format("  Center position: X={0:0.00}, y={1:0.00}\n", CallBackData.DPosX, CallBackData.DPosY);
							
							if (CallBackInfo==RxCam.ENCamCallBackInfo.StitchCorr)
							{
								Console.WriteLine("CALLBACK: Stitch[{0}]: corr[{1}]={2}, Pos={3}, DPosX={4}", _HeadNo, action.MeasureCnt, CallBackData.Value_1, RxGlobals.SetupAssist.ScanPos, CallBackData.DPosX);
								action.Measured(_MeasurementPass, CallBackData.Value_1);
								if (!CallBackData.Value_1.Equals(float.NaN))
								{
									Console.WriteLine("Action[{0}]: ScanPos from {1} to {2}", stitchIdx + _HeadNo, action.ScanPos, action.ScanPos + CallBackData.DPosX/1000);
									action.ScanPos += CallBackData.DPosX/1000;
								}
							}
							_HeadNo++;

							if (_HeadNo+1<_HeadsPerColor)
							{
								_ActionIdx= stitchIdx + _HeadNo;
								_Action=_Actions[_ActionIdx];
								RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
							}
							else
							{
								_HeadNo=0;
								if (++_MeasurementPass<RxGlobals.SA_AlignSettings.Passes)
								{
									_ActionIdx = stitchIdx + _HeadNo;
									_Action = _Actions[_ActionIdx];
									RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
								}
								else if (action.MeasureCnt < 5) //	if (action.MeasureCnt < 19)
								{
									_MeasurementPass = 0;
									_ActionIdx = stitchIdx + _HeadNo;
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
							int distIdx = _DistIdxes[_Action.PrintbarNo];
							action=_Actions[distIdx + _HeadNo];
							double offset=-((_DistStepCnt/4)-2) * px2mm(18);
							CallbackInfo=string.Format("# {0}\n", ++_CallbackNo) 
										+ "Measure Dist\n"
										+ string.Format("  Correction: X={0:0.00} offset={1:0.00}\n", CallBackData.Value_1, offset)
										+ string.Format("  Center position: X={0:0.00}, y={1:0.00}\n", CallBackData.DPosX, CallBackData.DPosY);
							Console.WriteLine("Step {0}, WebPos={1:0.000}: {2}", _DistStepCnt, RxGlobals.SetupAssist.WebPos, CallbackInfo);

							_CamFunctions.Off();
							if (CallBackInfo!=RxCam.ENCamCallBackInfo.RegisterCorr)
								break;
							action.Measured(_MeasurementPass, CallBackData.Value_1);

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
										_CamMeasureDist_done();
										break;
									}
									// new line 
									_HeadNo = 0;
									_Action=_Actions[distIdx + _HeadNo];
									_DistStepCnt++;
									action.WebMoveDone = false;
									RxGlobals.SetupAssist.WebMove(_DistWebDist);
									RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
									break;
								}
								if (_HeadNo+1<_HeadsPerColor && _Actions[distIdx + _HeadNo].State!=ECamFunctionState.done)
								{
									_Action=_Actions[distIdx + _HeadNo];
									RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
									break;
								}							
							}
							handled=true;
							break;

					//--- measure CamMeasureRegStitch ----------------------------------------------------------------------------
					case ECamFunction.CamMeasureRegStitch: // CallBackfromCam
						stitchIdx = _StitchIdxes[_Action.PrintbarNo];
						action = _Actions[stitchIdx + _HeadNo];
						if (CallBackInfo != RxCam.ENCamCallBackInfo.StitchCorr)
							break;

						CallbackInfo = string.Format("# {0}\n", ++_CallbackNo)
									+ "Measure Register Stitch\n"
									+ string.Format("  Correction: X={0:0.00}\n", CallBackData.Value_1)
									+ string.Format("  Center position: X={0:0.00}, y={1:0.00}\n", CallBackData.DPosX, CallBackData.DPosY);

						if (CallBackInfo == RxCam.ENCamCallBackInfo.StitchCorr)
						{
							Console.WriteLine("CALLBACK: Stitch[{0}]: corr[{1}]={2}, Pos={3}, DPosX={4}", _HeadNo, action.MeasureCnt, CallBackData.Value_1, RxGlobals.SetupAssist.ScanPos, CallBackData.DPosX);
							action.Measured(_MeasurementPass, CallBackData.Value_1);
							/*
							if (!CallBackData.Value_1.Equals(float.NaN))
							{
								Console.WriteLine("Action[{0}]: ScanPos from {1} to {2}", stitchIdx + _HeadNo, action.ScanPos, action.ScanPos + CallBackData.DPosX / 1000);
						//		action.ScanPos += CallBackData.DPosX / 1000;
							}
							*/
						}
						_CamFunctions.Off();
						if (_Action.MeasureCnt < 2*_HeadsPerColor)
						{
							int dist = ((_Action.MeasureCnt & 1)==0)? 2048 - 256 : 256;
							Console.WriteLine("Action[{0}]: CamMeasureRegStitch dist[{1}]={2}", _ActionIdx, _Action.MeasureCnt, dist);
							RxGlobals.SetupAssist.ScanMoveTo(RxGlobals.SetupAssist.ScanPos+ px2mm(dist));
							_Action.WebMoveDist=0;
							_Action.WebMoveDone=true;
						}
						else
						{
							/*
							_HeadNo = 0;
							//	if (action.MeasureCnt < 19)
							if (action.MeasureCnt < 5)
							{
								_ActionIdx = stitchIdx + _HeadNo;
								_Action = _Actions[_ActionIdx];
								Console.WriteLine("CamMeasureStitch next actinIdx={0}", _ActionIdx);
								_Action.WebMoveDone = false;
								_Action.ScanMoveDone = false;
								RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
								RxGlobals.SetupAssist.WebMove(_StitchWebDist);
							}
							else
							{
								_CamMeasureStitch_done();
							}
							*/
							_CamMeasureStitch_done();
						}
						handled = true;
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
				_Action.Measured(_MeasurementPass, result.CieLab.L);
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
		public bool Abort()
		{
			if (!_SA_Running) return false;
			_SA_Running = false;
			Console.WriteLine("Abort");
			if (_Actions!=null)
			{
				foreach(SA_Action action in _Actions)
				{
					if (action.State==ECamFunctionState.printing || action.State==ECamFunctionState.runningCam) 
					{
						action.State = ECamFunctionState.aborted;
					}
				}
				_ActionIdx=_Actions.Count();
			}
			else  _ActionIdx=1;
			_Action = null;
			if (RxGlobals.PrinterStatus.PrintState!=EPrintState.ps_off)
			{
				RxGlobals.RxInterface.SendCommand(TcpIp.CMD_STOP_PRINTING);
				Thread.Sleep(500);
				RxGlobals.RxInterface.SendCommand(TcpIp.CMD_STOP_PRINTING);
			}
			
			RxGlobals.SetupAssist.ScanReference();
			for(int n=0; n<_RobotUsed.Length; n++)
			{
				if (_RobotUsed[n]) RxGlobals.RxInterface.SendCommand(TcpIp.CMD_ROBI_MOVE_TO_GARAGE);
				_RobotUsed[n] = false;
			}

			if (!RxGlobals.PrintSystem.IsRobotConnected)
			{
				new SA_Report().PrintReport(RxGlobals.SA_StateMachine.GetActions(), RxGlobals.SA_StateMachine.TimePrinted(), false);
			}

			return false;
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
		//	if (_Debug) CanContinue = false;
			_Action			=_Actions[_ActionIdx];
			if (_Action.State==ECamFunctionState.undef || _Action.State == ECamFunctionState.done) // done in case of repeat
				_Action.State = (_Action.Function<ECamFunction.CamFindLines_Vertical)? ECamFunctionState.printing : ECamFunctionState.runningCam;			
			
			Console.WriteLine("_StartAction[{0}] >>{1}", _ActionIdx, _Action.Name);
			_SA_Running = true;
			switch(_Action.Function)
			{
				case ECamFunction.CamPrint:					_TestPrint_start();					break;
				case ECamFunction.CamFindLines_Vertical:	_CamFindLines_Vertical_start();		break;
				case ECamFunction.CamFindLine_Horzizontal:	_CamFindLine_Horzizontal_start();	break;
				case ECamFunction.CamFindFirstAngle:		_CamFindFirstAngle_start();			break;
				case ECamFunction.CamConfirmFocus:			_CamConfirmFocus_start();			break;
				case ECamFunction.CamMeasureAngle:			_CamMeasureAngle_start();			break;
				case ECamFunction.CamMoveScan:				_CamMoveScan_start();				break;
				case ECamFunction.CamMoveWeb:				_CamMoveWeb_start();				break;
				case ECamFunction.CamMeasureStitch:			_CamMeasureStitch_start();			break;
				case ECamFunction.CamMeasureDist:			_CamMeasureDist_start();			break;
				case ECamFunction.CamMeasureRegStitch:	    _CamMeasureRegStitch_start();		break;

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
				if (_Action.State!=ECamFunctionState.waitRob  && _Action.State!=ECamFunctionState.runningRob 
				&&  _Action.State!=ECamFunctionState.manualCw && _Action.State!=ECamFunctionState.manualCcw
				&& _Action.State != ECamFunctionState.error) 
					_Action.State = ECamFunctionState.done;
				if (_Debug && _Action!=_Actions.Last()) CanContinue = true;

				if (_ActionIdx==0) RxGlobals.SetupAssist.WebMoveInit();
				Console.WriteLine("ActionDone[{0}] >>{1}<<", _ActionIdx, _Action.Name);
				ECamFunction function=_Action.Function;
				int stepperNo = _Action.StepperNo;
				int printBar  = _Action.PrintbarNo;
				_Action = null;
				while (++_ActionIdx<_Actions.Count())
				{
					if (_Actions[_ActionIdx].Function==function && function!=ECamFunction.CamMeasureDist)
					{
						_StartAction();
						return;
					}

					/*
					if (_AdjustFunction[printBar]==ECamFunction.CamPrint) || _Actions[_ActionIdx].PrintbarNo!=printBar)
					{
						_StartAction();
						return;
					}
					*/

					if (_Actions[_ActionIdx].Function==ECamFunction.CamMoveWeb)
					{
						_SkipFct = false;
						_StartAction();
						return;
					}

					if (!_SkipFct)
					{
						_StartAction();
						return;
					}

					/*
					else if (_AdjustFunction[printBar]==ECamFunction.CamMeasureAngle)
					{
						Console.WriteLine("SKIP Image, WebPos={0}", RxGlobals.SetupAssist.WebPos);
						_SkipFct = _AlignmaneBmp_Height-15.0;			
					}
					else if (_AdjustFunction[printBar] == ECamFunction.CamMeasureStitch)
					{
						Console.WriteLine("SKIP Image, WebPos={0}", RxGlobals.SetupAssist.WebPos);
						_SkipFct = _AlignmaneBmp_Height - 50.0;
					}
					*/
					/*
					else if (_Adjusted)
					{
						RxGlobals.SetupAssist.ScanReference();
						Start();
						return;
					}
					*/
					/*
					if (!_Adjusted)
						_StartAction();
					else _NextRobotCmd(stepperNo);
					*/
					// return;
				}
				Console.WriteLine("All Actions done");
				bool robrunning=false;
				foreach(bool running in _RobotRunning)
					robrunning|=running;
				if (!robrunning)
					Abort();
			}
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
			Console.WriteLine("_CamFindLines_Vertical_start");
			_MarkFound = false;
			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
		}

		//--- _CamFindLine_Horzizontal_start ----------------------------------------------
		private void _CamFindLine_Horzizontal_start()
		{
			_MarkFound=false;
			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			Console.WriteLine("{0}: _CamFindLine_Horzizontal_start, MarkFound={1}", RxGlobals.Timer.Ticks(), _MarkFound);
			if (_SimuMachine) ActionDone();
			else
			{
				_Action.ScanMoveDone = false;
				RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
				/*
				_Action.ScanMoveDone = false;
				if (_SkipFct==0) _Action.WebMoveDone  = true;
				else 
				{
					RxGlobals.SetupAssist.WebMove(_SkipFct);
					_Action.WebMoveDone  = false;
					_SkipFct = 0;
				}
				*/
			}
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
				if (_Action.WebMoveDist==0) _Action.WebMoveDone=true; 
				else RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);

				if (true) //--- skip first measurement, probably out of scanner range
				{
					_Action.Measured(_MeasurementPass, float.NaN);
					if (_Action.ScanPos == 0) _Action.ScanPos = RxGlobals.SetupAssist.ScanPos;// + _AngleDist;
					_Action.WebMoveDist = 0;	// move only the first time
				}
				if (_SimuMachine)
				{
					_Action.ScanMoveDone=true;
					_StartCamFunction();
					return;
				}
				RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);

				_CamMeasureAngle_Step = 2;
			}
			else
			{
				_CamMeasureAngle_Step = 0;
			}
			Console.WriteLine("_CamMeasureAngle_start: _CamMeasureAngle_Step={0}", _CamMeasureAngle_Step);
		}

		//--- _CamMoveScan_start ----------------------------------------------
		private void _CamMoveScan_start()
		{
			_Action.WebMoveDone = true;
			_Action.ScanPos = _firstAnglePos;
			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
		}

		//--- _CamMoveWeb_start ----------------------------------------------
		private void _CamMoveWeb_start()
		{
			_Action.WebMoveDist = _Action.WebPos - RxGlobals.SetupAssist.WebPos;
			Console.WriteLine("CamMoveWeb WebPos ={0}, to {1} dist {2} done={3}", RxGlobals.SetupAssist.WebPos, _Action.WebPos, _Action.WebMoveDist, _Action.WebMoveDone);
			RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
			RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
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
			bool ok=true;
			// distances = _HeadsPerColor-1!
			for (int i=0; i<_HeadsPerColor-1; i++)
			{
				SA_Action action=_Actions[_DistIdxes[_Action.PrintbarNo]+i];
				if (action.State!=ECamFunctionState.done) action.State = ECamFunctionState.error;
				if (action.Correction!=null && Math.Abs((double)action.Correction)>0.010) 
				{
					corr += (double)action.Correction;
					RxGlobals.PrintSystem.HeadDist[action.PrintbarNo*RxGlobals.PrintSystem.HeadsPerColor+ action.HeadNo+1] += corr;
				}
				else ok=false;
			}
			if (corr!=0 && ok) RxGlobals.PrintSystem.SendMsg(TcpIp.CMD_SET_PRINTER_CFG);
			_ActionIdx = _DistIdxes[_Action.PrintbarNo]+_HeadsPerColor-2;
			_Action=_Actions[_ActionIdx];
			ActionDone();
		}

		//--- _CamMeasureRegStitch_start ----------------------------------------------
		private void _CamMeasureRegStitch_start()
		{
			//	RxGlobals.Events.AddItem(new LogItem("_CamMeasureRegStitch_start"));

			_DistStepCnt = 0;
			_DistMeasureCnt = 0;
			Console.WriteLine("{0}: Action[{1}]: ScanPos={2}, WebMoveDist={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.ScanPos, _Action.WebMoveDist);
			RxGlobals.SetupAssist.ScanMoveTo(RxGlobals.SetupAssist.ScanPos-px2mm(120));

			Console.WriteLine("{0}: Action[{1}]: Move web from {2} to {3} dist={4}", RxGlobals.Timer.Ticks(), _ActionIdx, RxGlobals.SetupAssist.WebPos, _Action.WebPos, _Action.WebPos - RxGlobals.SetupAssist.WebPos);

			RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
		}

		//--- _CamMeasureRegStitch_done -----------------------------
		private void _CamMeasureRegStitch_done()
		{
			//	RxGlobals.Events.AddItem(new LogItem("Camera: Send Stitch Correction {0} to head{1}", _Action.Correction, _Action.HeadNo));
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
			if (_Action.Correction == null)
			{
				if (_Action.State == ECamFunctionState.runningCam) _Action.State = ECamFunctionState.error;
			}
			else
			{
				if (Math.Abs((double)_Action.Correction) <= RxGlobals.SA_AlignSettings.ToleranceAngle + 0.05) _Action.State = ECamFunctionState.done;
				else
				{
					_AdjustFunction[_Action.PrintbarNo] = _Action.Function;
					if (RxGlobals.PrintSystem.IsRobotConnected)
					{ 
						_Action.State = ECamFunctionState.waitRob;
						_NextRobotCmd(_Action.StepperNo);
					}
					else _Action.State = (_Action.Correction < 0) ? ECamFunctionState.manualCw : ECamFunctionState.manualCcw;
				}
			}

			try
			{
				if (_ActionIdx+1<_Actions.Count && _Actions[_ActionIdx+1].Function==ECamFunction.CamMeasureAngle && _Actions[_ActionIdx+1].HeadNo>0)
				{
					// _Actions[_ActionIdx+1].ScanPos = _Actions[_ActionIdx].ScanPos+2*_AngleDist;
					int stitchIdx = _StitchIdxes[_Action.PrintbarNo];
					if (stitchIdx!=0 && stitchIdx + _Action.HeadNo < _Actions.Count())
					{
						_Actions[stitchIdx + _Action.HeadNo].ScanPos = RxGlobals.SetupAssist.ScanPos + _AngleDist - 0.2;//+_CallbackData.DPosX/1000;
						Console.WriteLine("_CamMeasureAngle_done: Set Stitch Action[{0}].ScanPos={1}, DPosX={2}", stitchIdx + _Action.HeadNo, _Actions[stitchIdx + _Action.HeadNo].ScanPos, _CallbackData.DPosX);
					}
					int distIdx = _DistIdxes[_Action.PrintbarNo];
					if (distIdx!=0 && distIdx + _Action.HeadNo<_Actions.Count() && _Actions[distIdx + _Action.HeadNo].Function==ECamFunction.CamMeasureDist)
					{
						_Actions[distIdx + _Action.HeadNo].ScanPos = RxGlobals.SetupAssist.ScanPos+_AngleDist+0.2;//+_CallbackData.DPosX/1000;
						Console.WriteLine("_CamMeasureAngle_done: Set Dist Action[{0}].ScanPos={1}, DPosX={2}", distIdx + _Action.HeadNo, _Actions[distIdx + _Action.HeadNo].ScanPos, _CallbackData.DPosX);
					}
				}
			}
			catch(Exception ex)
			{
				Console.WriteLine("Exception: {0}", ex.Message);
			}
			ActionDone();
		}

		//--- _CamMeasureStitch_done -----------------------------
		private void _CamMeasureStitch_done()
		{
		//	RxGlobals.Events.AddItem(new LogItem("Camera: Send Stitch Correction {0} to head{1}", _Action.Correction, _Action.HeadNo));
			_CamFunctions.Off();
			for (int i = 0; i + 1 < _HeadsPerColor; i++)
			{
				SA_Action action = _Actions[_StitchIdxes[_Action.PrintbarNo] + i];
				if (action.Correction==null)
				{
					if (action.State == ECamFunctionState.runningCam) action.State = ECamFunctionState.error;
				}
				else
				{
					if (Math.Abs((double)action.Correction) <= RxGlobals.SA_AlignSettings.ToleranceStitch + 0.05) 
						action.State = ECamFunctionState.done;
					else
					{
						_AdjustFunction[action.PrintbarNo] = action.Function;
						if (RxGlobals.PrintSystem.IsRobotConnected)
						{
							action.State = ECamFunctionState.waitRob;
							_NextRobotCmd(action.StepperNo);
						}
						else
						{
							action.State = (action.Correction < 0) ? ECamFunctionState.manualCw : ECamFunctionState.manualCcw;
						}
					}
				}
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
								case ECamFunction.CamConfirmFocus:  action.ConfirmVisibile = true; idx=1000; break;
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

					/*
			case ECamFunction.CamConfirmFocus: // _ScanMoveDone
					Console.WriteLine("Action[{0}].CamConfirmFocus, ScanPos={1}", _ActionIdx, RxGlobals.SetupAssist.ScanPos);
					if (RxGlobals.SetupAssist.ScanPos+2<_Action.ScanPos)
					{
						_Action.ScanMoveDone=false;
						RxGlobals.SetupAssist.ScanMoveTo(_Action.ScanPos);
					}
					else 
					_Action.ConfirmVisibile = true;
					break;
					*/

			case ECamFunction.CamMeasureAngle: // _ScanMoveDone
					break;

			case ECamFunction.CamMoveScan: // _ScanMoveDone
					if (++_MeasurementPass>= RxGlobals.SA_AlignSettings.Passes) 
					{
						if (_AdjustFunction[_Action.PrintbarNo] != ECamFunction.CamPrint)
							_SkipFct = true;
						_MeasurementPass = 0;
					}
					else 
					{
						_ActionIdx = _AngleIdxes[_Action.PrintbarNo] - 1;
						_Action = _Actions[_ActionIdx];
						_HeadNo = 0;
					}
					ActionDone();
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

			case ECamFunction.CamMeasureRegStitch: // _ScanMoveDone
					break;

			}

			if (_Action!=null)
			{
				Console.WriteLine("{0}: Action[{1}]: _ScanMoveDone: WebMoveDist={2}, WebMoveDone={3}", RxGlobals.Timer.Ticks(), _ActionIdx, _Action.WebMoveDist, _Action.WebMoveDone);
				if (_Action.WebMoveDist == 0 || _Action.WebMoveDone) _StartCamFunction();
			}
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
					case ECamFunction.CamPrint:	ActionDone();
														_TimePrimted = DateTime.Now;
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
					case ECamFunction.CamMoveWeb:
						Console.WriteLine("CamMoveWeb Web move DONE");
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
					_ActionIdx=_StitchIdxes[_Action.PrintbarNo]+_HeadNo;
					_Action=_Actions[_ActionIdx];
				}
				if (_Action.Function==ECamFunction.CamMeasureDist)
				{
					_ActionIdx= _DistIdxes[_Action.PrintbarNo] + _HeadNo;
					_Action=_Actions[_ActionIdx];
				}
				if (_Action.State==ECamFunctionState.undef)
					_Action.State = (_Action.Function<ECamFunction.CamFindLines_Vertical) ? ECamFunctionState.printing : ECamFunctionState.runningCam;

				switch(_Action.Function)
				{
					case ECamFunction.CamPrint:
						break;

					case ECamFunction.CamFindLines_Vertical:				
						_CamFunctions.FindLines_Vertical();
						RxGlobals.SetupAssist.WebMove(_Action.WebMoveDist);
						break;

					case ECamFunction.CamFindLine_Horzizontal:
						_MarkFound=false;
						_CamFunctions.FindLine_Horizontal();
						break;

					case ECamFunction.CamFindFirstAngle:
						Console.WriteLine("Action[{0}]: Start MeasureAngle, cnt={1}", _ActionIdx, _Actions[_ActionIdx].MeasureCnt);

						if (_SimuMachine) _MarkFound = true;
						else			  _MarkFound = false;
						result=_CamFunctions.MeasureAngle(true);
						if (result!=RxCam.ENCamResult.OK) RxGlobals.Events.AddItem(new LogItem("MeasureAngle Error {0}", result.ToString()));
						Console.WriteLine("{0}: Action[{1}]: Start MeasureAngle, ScanMoveTo(0, 100)", RxGlobals.Timer.Ticks(), _ActionIdx);
						_Action.ScanMoveDone = false;
						RxGlobals.SetupAssist.ScanMoveTo(0, 100);
						_MeasurementPass = 0;
						break;

					case ECamFunction.CamConfirmFocus:
						_Action.ConfirmVisibile = true;
						_CamFunctions.MeasureAngle(false);
						break;

					case ECamFunction.CamMeasureAngle:
						Console.WriteLine("Action[{0}].CamMeasureAngle, HeadNo={1}, ScanPos={2}, WebPos={3}, _CamMeasureAngle_Step={4}", _ActionIdx, _Action.HeadNo, RxGlobals.SetupAssist.ScanPos, RxGlobals.SetupAssist.WebPos, _CamMeasureAngle_Step);
						if (RxGlobals.SetupAssist.WebMoving) return;

						switch (_CamMeasureAngle_Step)
						{
							case 0:
								_CamMeasureAngle_Step = 1;
								_CamFunctions.MeasureAngle(true);
								_Action.ScanMoveDone = false;
								RxGlobals.SetupAssist.ScanMoveTo(RxGlobals.SetupAssist.ScanPos + 1.5, 100);
								break;

							case 1: //--- mark not found !
								RxCam.ENCamCallBackInfo CallBackInfo = new ENCamCallBackInfo();
								RxCam.CallBackDataStruct CallBackData = new CallBackDataStruct();
								CallBackfromCam(CallBackInfo, CallBackData);
								break;

							case 2:
								Console.WriteLine("Action[{0}]: Start MeasureAngle, cnt={1}, _CamMeasureAngle_Step={2}", _ActionIdx, _Actions[_ActionIdx].MeasureCnt, _CamMeasureAngle_Step);
								result = _CamFunctions.MeasureAngle(false);
								if (result != RxCam.ENCamResult.OK) RxGlobals.Events.AddItem(new LogItem("MeasureAngle Error {0}", result.ToString()){ LogType=ELogType.eErrCont});
								_CamMeasureAngle_Step = 3;
								break;
						}
						break;

					case ECamFunction.CamMeasureStitch:
						Console.WriteLine("Action[{0}]: Start MeasureStitch, cnt={1}, ScanPos={2}", _ActionIdx, _Actions[_ActionIdx].MeasureCnt, RxGlobals.SetupAssist.ScanPos);
						result=_CamFunctions.MeasureStitch();
						if (result!=RxCam.ENCamResult.OK) RxGlobals.Events.AddItem(new LogItem("MeasureStitch Error {0}", result.ToString()) { LogType = ELogType.eErrCont });
						break;

					case ECamFunction.CamMeasureDist:
						Console.WriteLine("Action[{0}]: Start MeasureDist, cnt={1}, ScanPos={2}", _ActionIdx, _Actions[_ActionIdx].MeasureCnt, RxGlobals.SetupAssist.ScanPos);
					//	if (_Action.State == ECamFunctionState.runningCam)
						{
							result=_CamFunctions.MeasureDist();
							if (result!=RxCam.ENCamResult.OK) RxGlobals.Events.AddItem(new LogItem("MeasureDist Error {0}", result.ToString()) { LogType = ELogType.eErrCont });
						}
						break;

					case ECamFunction.CamMeasureRegStitch:
						Console.WriteLine("Action[{0}]: Start CamMeasureRegStitch, cnt={1}, ScanPos={2}", _ActionIdx, _Actions[_ActionIdx].MeasureCnt, RxGlobals.SetupAssist.ScanPos);
						result = _CamFunctions.MeasureStitch();
						if (result != RxCam.ENCamResult.OK) RxGlobals.Events.AddItem(new LogItem("MeasureRegStitch Error {0}", result.ToString()));
						break;

					case ECamFunction.CamMoveWeb:
						Console.WriteLine("CamMoveWeb DONE");
						ActionDone();
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
