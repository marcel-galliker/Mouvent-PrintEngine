using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
	public class CTC_Operation : RxBindable
	{

		private const bool _Simulation = false;
		private Thread Process;

		//--- Constructor -----------------------------------------------------------------
		public CTC_Operation()
		{
			Reset();
		}

		//--- Clear ----------------------------
		public void Reset()
		{
			if (Process!=null) Process.Abort();
			
			//--- switch ink system off ----------------------------
			{
				TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
				msg.no = -1;
				msg.ctrlMode = EFluidCtrlMode.ctrl_off;
				RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
			}

			_Tests.Clear();
			CTC_Test.Overall.ResetState();
			_Tests.Add(CTC_Test.Overall);
		}

		//--- ElectronicsTest ---------------------------------------------------------
		public void ElectronicsTest(Action onDone)
		{
			Process = new Thread(()=>
			{
				RxBindable.Invoke(()=>_Tests.Add(new CTC_Test(){Name="Electronics" }));

				//--------------------------------------------------
				CTC_Test connected = new CTC_Test() { Step = "Connected" };
				CTC_Test temp = new CTC_Test() { Step = "Cooler Temperature" };
				CTC_Test pres = new CTC_Test() { Step = "Cooler Pressure" };
				CTC_Test volt = new CTC_Test() { Step = "Power -36V" };
				
				CTC_Settings settings = new CTC_Settings();
				CTC_Param tempPar = settings.GetParam("Electronics", temp.Step, "Cooler_Temp", 25000, 35000);
				CTC_Param presPar = settings.GetParam("Electronics", pres.Step, "Cooler_Pressure", 400, 600);

				RxBindable.Invoke(() =>
				{
					_Tests.Add(connected);
					_Tests.Add(temp);
					_Tests.Add(pres);
					_Tests.Add(volt);
				});

				if (_Simulation)
				{
					Thread.Sleep(1000);
					RxBindable.Invoke(() => 
					{
						connected.SimuFailed(new List<int>() { 1, 9 });
						pres.SimuFailed(new List<int>() { 4 });
						temp.SimuFailed(new List<int>() { 7 });
						volt.SimuFailed(new List<int>() { 10 });
					});
				}
				else 
				{
					for(int head=0; head<CTC_Test.HEADS; head++)
					{
						HeadStat stat = RxGlobals.HeadStat.List[head];
						UInt32   err  = RxGlobals.HeadStat.GetClusterErr(head/4);
						connected.SetResult(head, stat.Connected);
						Console.WriteLine("Head[{0}]: CoolerTemp={1}, CoolerPres={2}", head, stat.Cooler_Temp, stat.Cooler_Pressure);
						temp.SetResult(head, stat.Cooler_Temp>=tempPar.Min		&& stat.Cooler_Temp<=tempPar.Max);
						pres.SetResult(head, stat.Cooler_Pressure>=presPar.Min && stat.Cooler_Pressure<=presPar.Max);
						volt.SetResult(head, (err & 0x00000800)==0);
					}
				}

				if (onDone != null) RxBindable.Invoke(()=>onDone());
			});
			Process.Start();
		}

		//--- _SetShutoffValve -------------------------------------
		private void _SetShutoffValve(bool value)
		{
			TcpIp.SHeadTestCmd msg = new TcpIp.SHeadTestCmd();
			msg.no = 0;
			msg.valve = Convert.ToInt32(value);
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_SET_VALVE, ref msg);
		}

		//--- _CtrlAirPressure ---------------------------------------
		private bool _CtrlAirPressure(CTC_Test test, int pressure)
		{
			TcpIp.SFluidTestCmd msg = new TcpIp.SFluidTestCmd();
			msg.airValve = 1;
			msg.airPressure=(Int32)(pressure*11/10);
			for(int head=0; head<CTC_Test.HEADS; head++)
			{
				if (CTC_Test.Overall.State[head]!=CTC_Test.EN_State.failed)
				{
					test.State[head]=CTC_Test.EN_State.running;
					msg.no = head/RxGlobals.PrintSystem.HeadsPerColor;
					RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_TEST, ref msg);
				}
			}

			bool ok=false;
			while (!ok)
			{
				Thread.Sleep(200);
				ok=true;
				for(int head=0; head<CTC_Test.HEADS; head++)
				{
					if (test.State[head]==CTC_Test.EN_State.running)
					{
						msg.no=head/RxGlobals.PrintSystem.HeadsPerColor;
						Console.WriteLine("AirPressure[{0}]={1}", head, RxGlobals.InkSupply.List[msg.no].AirPressure);
						if (RxGlobals.InkSupply.List[msg.no].AirPressure < pressure) ok=false;
					}
				}
			};

			msg.airPressure=0;
			for(int head=0; head<CTC_Test.HEADS; head++)
			{
				if (CTC_Test.Overall.State[head]!=CTC_Test.EN_State.failed)
				{
					test.State[head]=CTC_Test.EN_State.running;
					msg.no = head/RxGlobals.PrintSystem.HeadsPerColor;
					RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_TEST, ref msg);
				}
			}

			return ok;
		}

		//--- _SetHeadValve --------------------------------------
		private int VALVE_OFF	= 0;
		private int VALVE_FLUSH	= 1;
		private int VALVE_INK	= 2;
		private void _SetHeadValve(CTC_Test test, int valve)
		{
			TcpIp.SHeadTestCmd msg = new TcpIp.SHeadTestCmd();
			for(int head=0; head<CTC_Test.HEADS; head++)
			{
				msg.no = head;
				if (test.State[head]!=CTC_Test.EN_State.failed) msg.valve=valve;
				else msg.valve = 0;
				RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_VALVE_TEST, ref msg);
			}
		}

		//--- _SetMeniscusCheck -----------------------------------------
		private void _SetMeniscusCheck(bool value)
		{
			TcpIp.SHeadTestCmd msg = new TcpIp.SHeadTestCmd();
			msg.valve = Convert.ToInt32(value);
		}

		//--- _CheckHeadPIN ----------------------------------------------------
		private bool _CheckHeadPIN(CTC_Test test, int time, int pressure)
		{
			while(time>0)
			{
				bool ok=false;
				Thread.Sleep(200);
				time-= 200;
				for( int headNo=0; headNo<CTC_Test.HEADS; headNo++)
				{
					if (test.State[headNo]==CTC_Test.EN_State.running)
					{
						Console.WriteLine("Conditioner[{0}].PIN={1}", headNo, RxGlobals.HeadStat.List[headNo].PresIn);
						if (RxGlobals.HeadStat.List[headNo].PresIn >= pressure) 
						{
							ok=true;
						}
						else
						{
							test.State[headNo]=CTC_Test.EN_State.failed;
						}
					}
				}
				if (!ok) break;
			}
			test.Done(CTC_Test.EN_State.ok);
			return true;
		}

		//--- _inRange ----------------------------------------------
		private bool _inRange(int value, CTC_Param par)
		{
			bool result;
			if (par==null) return true;
			if (par.Max>par.Min) result=(value>=par.Min && value<=par.Max);
			else                 result=value>par.Min; 
		//	Console.WriteLine("val={0}, Min={1}, Max={2}, result={3}", value, par.Min, par.Max, result);
			return result;
		}

		//--- _CheckHeadPrinting ----------------------------------------------------
		private bool _CheckHeadPrinting(CTC_Test test, int time, CTC_Param checkIn, CTC_Param checkOut, CTC_Param checkPump)
		{
			while(time>0)
			{
				bool ok=true;
				Thread.Sleep(200);
				time-= 200;
				for( int headNo=0; headNo<CTC_Test.HEADS; headNo++)
				{
					if (test.State[headNo]==CTC_Test.EN_State.running)
					{
						Console.WriteLine("Conditioner[{0}].PIN={1}, POUT={2}, Pump={3}", 
							headNo, RxGlobals.HeadStat.List[headNo].PresIn, RxGlobals.HeadStat.List[headNo].PresOut, RxGlobals.HeadStat.List[headNo].PumpSpeed);

						if (_inRange(-RxGlobals.HeadStat.List[headNo].PresIn, checkIn) 
						&&  _inRange(-RxGlobals.HeadStat.List[headNo].PresOut, checkOut) 
						&&  _inRange((int)RxGlobals.HeadStat.List[headNo].PumpSpeed, checkPump))
						{
							test.State[headNo] = CTC_Test.EN_State.ok;
						}
						else ok=false;
					}
				}
				if (ok) break;
			}
			test.Done(CTC_Test.EN_State.failed);
			return true;
		}

		//--- LeakTest ---------------------------------------------------------
		public void LeakTest(Action onDone)
		{
			Process = new Thread(() =>
			{
				RxBindable.Invoke(() => _Tests.Add(new CTC_Test() { Name = "Leak Test" }));

				//--------------------------------------------------
				CTC_Test test1 = new CTC_Test() { Step = "Ink Tank Pressure" };
				
				CTC_Settings settings = new CTC_Settings();
				CTC_Param startPar1 = settings.GetParam("Leak Test", test1.Step, "Pressure",    500, 0);
				CTC_Param startPar2 = settings.GetParam("Leak Test", test1.Step, "CheckTime", 10000, 0);
				CTC_Param startPar3 = settings.GetParam("Leak Test", test1.Step, "Pressure2",   400, 0);

				//--------------------------------------------------

				RxBindable.Invoke(() => _Tests.Add(test1));
				_SetHeadValve(test1, VALVE_OFF);

				_SetShutoffValve(false);

				bool ok = _CtrlAirPressure(test1, startPar1.Min);

				//--- check air pressure stays in range ---------------------
				Console.WriteLine("AirPressure Pressure ok for {0} sek", startPar2.Min);
				TcpIp.SFluidTestCmd msg = new TcpIp.SFluidTestCmd();
				msg.airValve	  = 1;
				msg.airPressure   = 0; 
				int time=startPar2.Min;
				do
				{
					Thread.Sleep(200);
					time-= 200;
					Console.WriteLine("AirPressure{0}[{1}]={2}", startPar2.Min, 0, RxGlobals.InkSupply.List[0].AirPressure);
					if (RxGlobals.InkSupply.List[0].AirPressure < startPar3.Min) 
					{
						test1.State[0]=CTC_Test.EN_State.failed;
						ok=false;
					}
				} while (ok && time>0);
				test1.Done(CTC_Test.EN_State.ok);

				_SetShutoffValve(true);
				if (ok) 
				{
					CTC_Test test2 = new CTC_Test() { Step = "Flush Pressure" };
					RxBindable.Invoke(() => _Tests.Add(test2));

					_SetHeadValve(test2, VALVE_FLUSH);

					ok = _CtrlAirPressure(test2, startPar1.Min);

					//--- check conditioner IN-Pressure stays in range ---------------------
					Console.WriteLine("conditioner IN-Pressure ok for {0} sek", startPar2.Min);
					ok=_CheckHeadPIN(test2, startPar2.Min, startPar3.Min);
				}

				if (ok)
				{
					CTC_Test test3 = new CTC_Test() { Step = "Ink Pressure" };
					RxBindable.Invoke(() => _Tests.Add(test3));

					_SetHeadValve(test3, VALVE_INK);
					ok = _CtrlAirPressure(test3, startPar1.Min);

					//--- check conditioner IN-Pressure stays in range ---------------------
					Console.WriteLine("conditioner IN-Pressure ok for {0} sek", startPar2.Min);
					ok=_CheckHeadPIN(test3, startPar2.Min, startPar3.Min);
				}
				_SetShutoffValve(false);

				_SendStop();

				//--- END ---------------------------------------
				if (onDone != null) RxBindable.Invoke(() => onDone());
			});
			Process.Start();
		}

		//--- ValveTest ---------------------------------------------------------
		public void ValveTest(Action onDone)
		{
			Process = new Thread(() =>
			{
				RxBindable.Invoke(() => _Tests.Add(new CTC_Test() { Name = "Valve Test" }));

				CTC_Test connected = new CTC_Test() { Step = "Connected" };
				for(int head=0; head<CTC_Test.HEADS; head++)
				{
					HeadStat stat = RxGlobals.HeadStat.List[head];
					connected.SetResult(head, stat.Connected);
				}
				RxBindable.Invoke(() => _Tests.Add(connected));

				bool[] isUsed = new bool[RxGlobals.PrintSystem.ColorCnt];
				bool ok;
				CTC_Test test1 = new CTC_Test() { Step = "Print" };
				
				CTC_Settings settings = new CTC_Settings();
				CTC_Param pressure		= settings.GetParam("Valve Test", test1.Step, "Pressure Inlet Cond",    0, 0);
				CTC_Param checkTime		= settings.GetParam("Valve Test", test1.Step, "Check Time",				10000, 0);
				CTC_Param checkIn		= settings.GetParam("Valve Test", test1.Step, "Check Inlet",			30, 50);
				CTC_Param checkOut		= settings.GetParam("Valve Test", test1.Step, "Check Outlet",			90, 110);
				CTC_Param checkPump		= settings.GetParam("Valve Test", test1.Step, "Check Pump",				40, 45);
								
				CTC_Test test2 = new CTC_Test() { Step = "Test" };
				CTC_Param checkTime2	= settings.GetParam("Valve Test", test2.Step, "Check Time",				10000, 0);
				CTC_Param checkPump2	= settings.GetParam("Valve Test", test2.Step, "Check Pump",				40, 45);
				
				CTC_Test test3 = new CTC_Test() { Step = "Print" };

				test1.Start();
				RxBindable.Invoke(() => _Tests.Add(test1));

				//--- switch off meniscus check --------------------------------
				_SetMeniscusCheck(false);
				for( int headNo=0; headNo<CTC_Test.HEADS; headNo++)
				{
					if (test1.State[headNo]==CTC_Test.EN_State.running) isUsed[headNo/RxGlobals.PrintSystem.ColorCnt] = true;
				}

				//--- start ink supplies ------------------------------------
				for (int isNo=0; isNo<RxGlobals.PrintSystem.ColorCnt; isNo++)
				{
					if (isUsed[isNo])
					{
						TcpIp.SValue msg = new TcpIp.SValue(){no=isNo, value=(Int32)pressure.Min};
						RxGlobals.InkSupply.List[isNo].CylinderPresSet = pressure.Min;
						RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_PRESSURE, ref msg);

						TcpIp.SFluidCtrlCmd cmd = new TcpIp.SFluidCtrlCmd();
						cmd.no = isNo;
						cmd.ctrlMode = EFluidCtrlMode.ctrl_print;
				        RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref cmd);
					}
				}

				//--- check conditioners ---------------
			//	ok = _CheckHeadPrinting(test1, checkTime.Min, checkIn, checkOut, checkPump);


				//===== TEST ==================================================================================
				test2.Start();
				RxBindable.Invoke(() => _Tests.Add(test2));
				_SetShutoffValve(true);

			//	ok = _CheckHeadPrinting(test2, checkTime2.Min, null, null, checkPump2);

				_SetShutoffValve(false);
			
				//=== START again ======================================================

				test3.Start();
				RxBindable.Invoke(() => _Tests.Add(test3));
				_SetMeniscusCheck(true);
				ok = _CheckHeadPrinting(test3, checkTime.Min, checkIn, checkOut, checkPump);

				_SendStop();
				//--- END ---------------------------------------
				if (onDone != null) RxBindable.Invoke(() => onDone());
			});
			Process.Start();
		}

		//--- LongRun ---------------------------------------------------------
		public void LongRun(Action onDone)
		{
			Process = new Thread(() =>
			{
				CTC_Test result;

				RxBindable.Invoke(() => _Tests.Add(new CTC_Test() { Name = "Long Run" }));

				//--------------------------------------------------
				result = new CTC_Test() { Step = "test 1" };
				RxBindable.Invoke(() => _Tests.Add(result));

				if (_Simulation)
				{
					for(int head=0; head<CTC_Test.HEADS; head++)
					{
						if (CTC_Test.Overall.State[head]!=CTC_Test.EN_State.failed)
						{
							RxBindable.Invoke(() => result.State[head]=CTC_Test.EN_State.running);
							Thread.Sleep(1000);
							RxBindable.Invoke(() => result.State[head] = CTC_Test.EN_State.ok);
						}
					}	
				}

				//--- END ---------------------------------------
				if (onDone != null) RxBindable.Invoke(() => onDone());
			});
			Process.Start();
		}

		//--- Empty ---------------------------------------------------------
		public void Empty(Action onDone)
		{
			Process = new Thread(() =>
			{
				CTC_Test result;

				RxBindable.Invoke(() => _Tests.Add(new CTC_Test() { Name = "Empty" }));

				//--------------------------------------------------
				result = new CTC_Test() { Step = "test 1" };
				result.Start();
				RxBindable.Invoke(() => _Tests.Add(result));

				if (_Simulation)
				{
					Thread.Sleep(1000);
					RxBindable.Invoke(() => result.SimuFailed(new List<int>() { 1, 9 }));
				}


				//--- END ---------------------------------------
				if (onDone != null) RxBindable.Invoke(() => onDone());
			});
			Process.Start();
		}


		//--- _SendStop ------------------------------------
		private void _SendStop()
		{
			//--- switch ink system off ----------------------------
			{
				TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
				msg.no = -1;
				msg.ctrlMode = EFluidCtrlMode.ctrl_off;
				RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
			}
		}

		//--- Stop ---------------------------------------------------------
		public void Stop(Action onDone)
		{
			_SendStop();

			Process = new Thread(() =>
			{
				CTC_Test result;

				//--------------------------------------------------
				result = new CTC_Test() { Name = "Off" };
				result.Start();
				RxBindable.Invoke(() => _Tests.Add(result));

				if (_Simulation)
				{
					Thread.Sleep(1000);
					RxBindable.Invoke(() => result.SimuFailed(new List<int>() { 1, 9 }));
				}


				//--- END ---------------------------------------
				if (onDone != null) RxBindable.Invoke(() => onDone());
			});
			Process.Start();
		}

		//--- Property Tests ---------------------------------------
		private ObservableCollection<CTC_Test> _Tests = new ObservableCollection<CTC_Test>();
		public ObservableCollection<CTC_Test> Tests
		{
			get { return _Tests; }
			set { SetProperty(ref _Tests, value); }
		}
	}
}
