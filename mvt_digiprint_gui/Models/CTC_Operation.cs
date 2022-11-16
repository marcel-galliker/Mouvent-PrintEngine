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

		private int FLUID_VALVE_SHUTOFF	= 1;
		private int FLUID_VALVE_FLUSH	= 6;
		private int FLUID_VALVE_BLEED	= 7;

		//--- _SetFluidValve -------------------------------------
		private void _SetFluidValve(int valve, bool value)
		{
			TcpIp.SHeadTestCmd msg = new TcpIp.SHeadTestCmd();
			msg.no    = 0; // always fluid 0
			msg.valve = valve;
			msg.value = Convert.ToInt32(value);
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
		private int HEAD_VALVE_OFF	= 0;
		private int HEAD_VALVE_FLUSH= 1;
		private int HEAD_VALVE_INK	= 2;
		private int HEAD_VALVE_BOTH	= 3;
		
		private void _SetHeadValve(CTC_Test test, int valve)
		{
			TcpIp.SHeadTestCmd msg = new TcpIp.SHeadTestCmd();
			for(int head=0; head<CTC_Test.HEADS; head++)
			{
				msg.no = head;
				if (test.State[head]!=CTC_Test.EN_State.failed) msg.valve=valve;
				else msg.valve = HEAD_VALVE_OFF;
				RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_VALVE_TEST, ref msg);
			}
		}

		//--- _SetMeniscusCheck -----------------------------------------
		private void _SetMeniscusCheck(bool value)
		{
			TcpIp.SHeadTestCmd msg = new TcpIp.SHeadTestCmd();
			msg.valve = Convert.ToInt32(value);
			RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_SET_MENISCUS_CHK, ref msg);
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
							test.SetHeadState(headNo, CTC_Test.EN_State.failed);
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
				RxBindable.Invoke(() => _Tests.Add(test1));
				CTC_Test test2 = new CTC_Test() { Step = "test 2" };
				RxBindable.Invoke(() => _Tests.Add(test2));
				CTC_Test test3 = new CTC_Test() { Step = "test 3" };
				RxBindable.Invoke(() => _Tests.Add(test3));
				CTC_Test test4 = new CTC_Test() { Step = "test 4" };
				RxBindable.Invoke(() => _Tests.Add(test4));
				CTC_Test test5 = new CTC_Test() { Step = "test 5" };
				RxBindable.Invoke(() => _Tests.Add(test5));
				CTC_Test test6 = new CTC_Test() { Step = "test 6" };
				RxBindable.Invoke(() => _Tests.Add(test6));
				
				CTC_Settings settings = new CTC_Settings();
				CTC_Param pressurePar = settings.GetParam("Leak Test", test1.Step, "Pressure",		 500, 0);
				CTC_Param timePar     = settings.GetParam("Leak Test", test1.Step, "Check Time",	10000, 0);
				CTC_Param startPar3   = settings.GetParam("Leak Test", test1.Step, "Check Pressure",   400, 0);
				CTC_Param coolerPresPar  = settings.GetParam("Leak Test", "Cooler Pressure", "Cooler_Pressure", 400, 600);
				CTC_Param test4TimePar   = settings.GetParam("Leak Test", test4.Step, "Check Time",	10000, 0);
				CTC_Param test4PresPar   = settings.GetParam("Leak Test", test4.Step, "Check Pressure",   400, 0);
				CTC_Param test5TimePar   = settings.GetParam("Leak Test", test5.Step, "Check Time",	10000, 0);
				CTC_Param test5PresPar   = settings.GetParam("Leak Test", test5.Step, "Check Pressure",   400, 0);
				CTC_Param test6TimePar   = settings.GetParam("Leak Test", test6.Step, "Check Time",	10000, 0);
				CTC_Param test6PresPar   = settings.GetParam("Leak Test", test6.Step, "Check Pressure",   400, 0);

				//--- Test 1: pressure over flush -----------------------------------------------

				test1.Start();
				_SetFluidValve(FLUID_VALVE_SHUTOFF, true);
				_SetFluidValve(FLUID_VALVE_FLUSH, false);
				_SetHeadValve(test1, HEAD_VALVE_FLUSH);
				_CtrlAirPressure(test1, pressurePar.Min);
				_CheckHeadPIN(test1, timePar.Min, pressurePar.Min);
				_SendStop();

				//--- test cooler pressure 
				test2.Start();
				_SetHeadValve(test2, HEAD_VALVE_BOTH);
				Thread.Sleep(5000);
				_SetHeadValve(test2, HEAD_VALVE_OFF);
				_SetFluidValve(FLUID_VALVE_FLUSH, true);
				Thread.Sleep(1000);
				for(int head=0; head<CTC_Test.HEADS; head++)
				{
					if (test2.State[head]==CTC_Test.EN_State.running)
					{
						HeadStat stat = RxGlobals.HeadStat.List[head];
						UInt32   err  = RxGlobals.HeadStat.GetClusterErr(head/4);
						test2.SetResult(head, stat.Cooler_Pressure>=coolerPresPar.Min && stat.Cooler_Pressure<=coolerPresPar.Max);
						if (test2.State[head]==CTC_Test.EN_State.failed)
							Console.WriteLine("CoolerPressure[{0}]={1} min={2} max={3} {4}", head, stat.Cooler_Pressure, coolerPresPar.Min, coolerPresPar.Max, test2.State[head].ToString());
					}
				}

				//--- test 3 ------------------------------------
				test3.Start();
				_SetFluidValve(FLUID_VALVE_BLEED, true);
				Thread.Sleep(200);
				_SetFluidValve(FLUID_VALVE_BLEED, false);
				_CheckHeadPIN(test3, timePar.Min, pressurePar.Min);

				//--- test 4 ---------------------------
				test4.Start();
				_SetHeadValve(test4, HEAD_VALVE_FLUSH);
				_CheckHeadPIN(test4, test4TimePar.Min, test4PresPar.Min);
				_SetHeadValve(test4, HEAD_VALVE_OFF);

				//--- test 5 ----------------------------
				test5.Start();
				_SetFluidValve(FLUID_VALVE_FLUSH, false);
				_CheckHeadPIN(test5, test5TimePar.Min, test5PresPar.Min);

				//--- test 6----------------------------
				test6.Start();
				_SetHeadValve(test4, HEAD_VALVE_BOTH);
				_SetFluidValve(FLUID_VALVE_FLUSH, false);
				_CheckHeadPIN(test6, test6TimePar.Min, test6PresPar.Min);

				//--- END ---------------------------------------
				_SetFluidValve(FLUID_VALVE_SHUTOFF, true);
				Thread.Sleep(1000);
				_SendStop();
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
								
				CTC_Test test2 = new CTC_Test() { Step = "Test Valve"};
				CTC_Param checkTime2	= settings.GetParam("Valve Test", test2.Step, "Check Time",				10000, 0);
				CTC_Param checkPump2	= settings.GetParam("Valve Test", test2.Step, "Check Pump",				80, 0);
				
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
				ok = _CheckHeadPrinting(test1, checkTime.Min, checkIn, checkOut, checkPump);

				//===== TEST ==================================================================================
				test2.Start();
				RxBindable.Invoke(() => _Tests.Add(test2));
				_SetFluidValve(FLUID_VALVE_SHUTOFF, true);

				ok = _CheckHeadPrinting(test2, checkTime2.Min, null, null, checkPump2);

				_SetFluidValve(FLUID_VALVE_SHUTOFF, false);
			
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
			_SetFluidValve(FLUID_VALVE_SHUTOFF, false);
			_SetFluidValve(FLUID_VALVE_FLUSH, false);
			_SetFluidValve(FLUID_VALVE_BLEED, false);
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
