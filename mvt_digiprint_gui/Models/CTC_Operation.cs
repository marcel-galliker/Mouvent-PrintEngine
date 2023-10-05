using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.ObjectModel;
using System.Threading;

namespace RX_DigiPrint.Models
{
	public class CTC_Operation : RxBindable
	{
		private Thread Process;
		private bool _LongRun_Running=false;
		
		private CTC_FlowSensor _FlowSensor = new CTC_FlowSensor();

		public Action<int> DisplayTimer = (time)=> { };
		public Action<string> DisplayFlow = (time)=> { };

		//--- Constructor -----------------------------------------------------------------
		public CTC_Operation()
		{
			Reset();
		}

		//--- Clear ----------------------------
		public void Reset()
		{
			if (Process != null)
			{
				Stop(null);
				Thread.Sleep(1000);
			}

			_SendStop();
			_Tests.Clear();
			CTC_Test.Overall.ResetState();
			_Tests.Add(CTC_Test.Overall);
		}

		//--- ElectronicsTest ---------------------------------------------------------
		public void ElectronicsTest(Action<bool> onDone)
		{
			Stop(null);

			Process = new Thread(()=>
			{
				RxBindable.Invoke(()=>_Tests.Add(new CTC_Test(){Name="Electronics" }));

				//--------------------------------------------------
				CTC_Test connected = new CTC_Test() { Step = "Connected" };
				CTC_Test temp = new CTC_Test() { Step = "Cooler Temperature" };
				CTC_Test pres = new CTC_Test() { Step = "Cooler Pressure" };
				CTC_Test volt = new CTC_Test() { Step = "Power -36V" };
				CTC_Test heater = new CTC_Test() { Step = "Heater" };

				CTC_Settings settings = new CTC_Settings();
				CTC_Param tempPar	 = settings.GetParam("Electronics", temp.Step,	"Cooler_Temp",	   25000, 35000);
				CTC_Param presPar	 = settings.GetParam("Electronics", pres.Step,	"Cooler_Pressure", 	400,    600);
				CTC_Param heaterTime = settings.GetParam("Electronics", heater.Step,"Heating Time",		2000,     0);
				uint[] _Temp = new uint[CTC_Test.HEADS];

				RxBindable.Invoke(() =>
				{
					_Tests.Add(connected);
					_Tests.Add(temp);
					_Tests.Add(pres);
					_Tests.Add(volt);
					_Tests.Add(heater);
				});

				volt.Start();
				// enable -36 V
				for (int head = 0; head < CTC_Test.HEADS && head < RxGlobals.HeadStat.List.Count; head++)
				{
					_Temp[head] = RxGlobals.HeadStat.List[head].TempHeater;
					if ((head % TcpIp.HEAD_CNT) == 0)
					{
						TcpIp.SValue msg = new TcpIp.SValue();
						msg.no = head/ TcpIp.HEAD_CNT;
						msg.value = 10;
						RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_ENCODER_FREQ, ref msg);
					}
					if (CTC_Test.Overall.State[head] != CTC_Test.EN_State.failed)
					{
						heater.SetHeadState(head, CTC_Test.EN_State.running);
						TcpIp.SValue msg = new TcpIp.SValue();
						msg.no = head;
						msg.value = heaterTime.Min;
						RxGlobals.RxInterface.SendMsg(TcpIp.CMD_TEST_HEATER, ref msg);
					}
				}

				CTC_Test.Wait(2000, DisplayTimer);
			//	for (int time=2000; time>0;)
				{
					/*
					DisplayTimer(time);
					Thread.Sleep(500);
					time-=500;
					*/
					for (int head=0; head<CTC_Test.HEADS && head< RxGlobals.HeadStat.List.Count; head++)
					{
						if (volt.State[head]==CTC_Test.EN_State.running)
						{
							HeadStat stat = RxGlobals.HeadStat.List[head];
							UInt32   err  = RxGlobals.HeadStat.GetClusterErr(head/4);
							connected.SetResult(head, stat.Connected);
							Console.WriteLine("Head[{0}]: CoolerTemp={1}, CoolerPres={2}, fp_voltage={3}", head, stat.Cooler_Temp, stat.Cooler_Pressure, stat.FP_Voltage);
							temp.SetResult(head, stat.Cooler_Temp>=tempPar.Min		&& stat.Cooler_Temp<=tempPar.Max);
							pres.SetResult(head, stat.Cooler_Pressure>=presPar.Min && stat.Cooler_Pressure<=presPar.Max);
							if ((err & (0x00000800 // 36V
										   | 0x00010000 // 2.5V
										   | 0x02000000 // 3.3V
										   | 0x04000000))==0
								&& stat.FP_Voltage<-30000)
								volt.SetHeadState(head, CTC_Test.EN_State.ok);
							else 
								volt.SetHeadState(head, CTC_Test.EN_State.failed);
						}
					}
				}
				volt.Done(CTC_Test.EN_State.failed);
				CTC_Test.Wait(10000, DisplayTimer);
				
				for (int head = 0; head < CTC_Test.HEADS && head < RxGlobals.HeadStat.List.Count; head++)
				{
					// disable -36 V
					if ((head % TcpIp.HEAD_CNT) == 0)
					{
						TcpIp.SValue msg = new TcpIp.SValue();
						msg.no = head / TcpIp.HEAD_CNT;
						msg.value = 0;
						RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_ENCODER_FREQ, ref msg);
					}
					/*
					// check temp ---------------------------
					Console.WriteLine("Head[{0}]: TempOrg={1}, Temp={2}, delta={3}", head, _Temp[head], RxGlobals.HeadStat.List[head].TempHeater, RxGlobals.HeadStat.List[head].TempHeater - _Temp[head]);
					if (RxGlobals.HeadStat.List[head].TempHeater > 40000 
					||  RxGlobals.HeadStat.List[head].TempHeater-_Temp[head] > 500)
						heater.SetHeadState(head, CTC_Test.EN_State.ok);
					else if (CTC_Test.Overall.State[head] != CTC_Test.EN_State.failed)
						heater.SetHeadState(head, CTC_Test.EN_State.failed);
					else heater.SetHeadState(head, CTC_Test.EN_State.undef);
					*/
				}

				_SendStop();
				CTC_Test.Wait(5000, DisplayTimer);
				for (int head = 0; head < CTC_Test.HEADS && head < RxGlobals.HeadStat.List.Count; head++)
				{
					// check temp ---------------------------
					Console.WriteLine("Head[{0}]: TempOrg={1}, Temp={2}, delta={3}", head, _Temp[head], RxGlobals.HeadStat.List[head].TempHeater, RxGlobals.HeadStat.List[head].TempHeater - _Temp[head]);
					if (RxGlobals.HeadStat.List[head].TempHeater > 40000)
//					||  RxGlobals.HeadStat.List[head].TempHeater-_Temp[head] > 500)
						heater.SetHeadState(head, CTC_Test.EN_State.ok);
					else if (CTC_Test.Overall.State[head] != CTC_Test.EN_State.failed)
						heater.SetHeadState(head, CTC_Test.EN_State.failed);
					else heater.SetHeadState(head, CTC_Test.EN_State.undef);
				}

				if (onDone != null) RxBindable.Invoke(()=>onDone(false));
			});
			Process.Start();
		}

		private int FLUID_VALVE_SHUTOFF	= 0;
		private int FLUID_CYLINDER_0	= 1;
	//	private int FLUID_CYLINDER_1	= 2;
	//	private int FLUID_CYLINDER_2	= 3;
	//	private int FLUID_CYLINDER_3    = 4;
		private int FLUID_VALVE_FLUSH	= 5;
		private int FLUID_VALVE_RELEASE	= 6;

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
		private bool _CtrlAirPressure(CTC_Test test, int head0, int cnt, int pressure, int timeout)
		{
			//--- start fluid -----------------------
			TcpIp.SFluidTestCmd msg = new TcpIp.SFluidTestCmd();
			bool ok;
			int time=0;
			msg.airValve = 1;
			msg.airPressure=(Int32)(pressure*11/10);
			if (msg.airPressure>1200) msg.airPressure=1200;
			ok=false;
			for(int head= head0; head< head0+cnt; head++)
			{
				if (test.State[head]==CTC_Test.EN_State.running)
				{
					msg.no = head/RxGlobals.PrintSystem.HeadsPerColor;
					Console.WriteLine("CMD_FLUID_TEST head={0}, no={1}, pres={2}, valve={3}", head, msg.no, msg.airPressure, msg.airValve);
					RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_TEST, ref msg);
					ok=true;
				}
			}

			if (!ok) return false;

			time=timeout;
			ok=false;
			while (time>0)
			{
				DisplayTimer(time);
				Thread.Sleep(200);
				time -= 200;
				Console.WriteLine("time={0}, AirPressure={1}", time, RxGlobals.InkSupply.List[0].AirPressure);
				if (RxGlobals.InkSupply.List[0].AirPressure > pressure-20) 
				{
					ok=true;
					break;
				}
			};
			DisplayTimer(0);

			//--- toggle valves ----------------------------------
			if (ok)
			{
				_SetHeadValve(test, 0, CTC_Test.HEADS, HEAD_VALVE_INK);
				CTC_Test.Wait(5000, DisplayTimer);
				_SetHeadValve(test, 0, CTC_Test.HEADS, HEAD_VALVE_FLUSH);
				CTC_Test.Wait(5000, DisplayTimer);
				_SetHeadValve(test, 0, CTC_Test.HEADS, HEAD_VALVE_INK);
				CTC_Test.Wait(5000, DisplayTimer);
				_SetHeadValve(test, 0, CTC_Test.HEADS, HEAD_VALVE_FLUSH);
				CTC_Test.Wait(5000, DisplayTimer);
			}
			else RxGlobals.Events.AddItem(new LogItem("Leak Test {0}: Timeout, fluid did not reach defined pressure", test.Name){LogType = ELogType.eErrCont});

			msg.airPressure=0;
			for(int head=head0; head< head0+cnt; head++)
			{
				if (CTC_Test.Overall.State[head]!=CTC_Test.EN_State.failed)
				{
					if (ok) test.SetHeadState(head, CTC_Test.EN_State.running);
					else test.SetHeadState(head, CTC_Test.EN_State.failed);
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
		
		private void _SetHeadValve(CTC_Test test, int head0, int cnt, int valve)
		{
			TcpIp.SHeadTestCmd msg = new TcpIp.SHeadTestCmd();
			for(int head= head0; head< head0+cnt; head++)
			{
				msg.no = head;
				if (test.State[head]!=CTC_Test.EN_State.failed) msg.valve=valve;
				else msg.valve = 0;
				Console.WriteLine("{0}: Head[{1}].Valve={2}", RxGlobals.Timer.Ticks(), head, msg.valve);
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
		private bool _CheckHeadPIN(CTC_Test test, int head0, int cnt, int time, int presMin, int presMax)
		{
			Console.WriteLine("{0}: _CheckHeadPIN < {1}", RxGlobals.Timer.Ticks(), presMin, presMax);
			if (presMax==0) presMax=1000000;
			int delay=5;
			while(time>0)
			{
				DisplayTimer(time);
				Thread.Sleep(200);
				time-= 200;
				if (delay>0)
				{
					delay--;
				}
				else
				{
					bool ok = false;
					for ( int head=head0; head<head0+cnt && head<RxGlobals.HeadStat.List.Count; head++)
					{
						if (test.State[head]==CTC_Test.EN_State.running)
						{
							if (head==0) Console.WriteLine("{0}: Conditioner[{1}].PIN={2}", RxGlobals.Timer.Ticks(), head, RxGlobals.HeadStat.List[head].PresIn);
							if (RxGlobals.HeadStat.List[head].PresIn/10 >= presMin && RxGlobals.HeadStat.List[head].PresIn/10 <= presMax) 
							{
								ok=true;
							}
							else
							{
								if (RxGlobals.HeadStat.List[head].PresIn!=0) RxGlobals.Events.AddItem(new LogItem("Leak Test {0}: Head[{1}].PresIn={2} out of ({3}..{4})", test.Name, head+1, RxGlobals.HeadStat.List[head].PresIn/10, presMin, presMax){LogType = ELogType.eErrCont});
								test.SetHeadState(head, CTC_Test.EN_State.failed);
							}
						}
					}
					if (!ok) time=0;
				}
			}
			DisplayTimer(0);
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

		//--- _allOk -----------------------------
		private bool _allOk(CTC_Test connected)
		{
			bool ok=true;
			for(int i=0; i<connected.State.Count; i++)
			{
				if (connected.State[i]==CTC_Test.EN_State.ok)
					ok &= (CTC_Test.Overall.State[i]==CTC_Test.EN_State.ok); 
			}
			return ok;
		}

		//--- _CheckHeadPrinting ----------------------------------------------------
		private bool _CheckHeadPrinting(CTC_Test test, int time, CTC_Param checkIn, CTC_Param checkCylinder, CTC_Param checkOut, CTC_Param checkPump)
		{
			while(time>0)
			{
				DisplayTimer(time);
				bool ok=true;
				Thread.Sleep(200);
				time-= 200;
				for( int cluster=0; cluster<(CTC_Test.HEADS+3)/4; cluster++)
				{
					//--- calculate pumpspeed per cluster ----------
					int pumpSpeed = 0;
					int cnt = 0;
					for (int i=0; i<4; i++)
					{
						int headNo = 4*cluster+i;
						if (test.State[headNo]==CTC_Test.EN_State.running)
						{
							pumpSpeed += (int)RxGlobals.HeadStat.List[headNo].PumpSpeed;
							cnt++;
						}
					}
					if (cnt>0) pumpSpeed /= cnt;
				
					for (int i=0; i<4; i++)
					{
						int headNo = 4*cluster+i;
						if (test.State[headNo]==CTC_Test.EN_State.running)
						{
							bool r1 = _inRange(RxGlobals.HeadStat.List[headNo].InkSupply.CylinderPres, checkCylinder);
							bool r2 = _inRange(RxGlobals.HeadStat.List[headNo].PresIn, checkIn);
							bool r3 = _inRange(RxGlobals.HeadStat.List[headNo].PresOut, checkOut);
							bool r4 = _inRange(pumpSpeed, checkPump);

							Console.WriteLine("Conditioner[{0}].Cylinder={1}, PIN={2} POUT={3}, Pump={4}, {5} {6} {7} {8}", 
								headNo,	// 0 
								RxGlobals.HeadStat.List[headNo].InkSupply.CylinderPres,  // 1
								-RxGlobals.HeadStat.List[headNo].PresIn, // 2
								RxGlobals.HeadStat.List[headNo].PresOut, // 3
								RxGlobals.HeadStat.List[headNo].PumpSpeed, // 4
								r1, // 5 
								r2, // 6 
								r3, // 7
								r4, // 8 
								RxGlobals.HeadStat.List[headNo].PresIn2 // 9
								);
							if (r1&&r2&&r3&&r4)
							{
								RxBindable.Invoke(()=>test.State[headNo] = CTC_Test.EN_State.ok);
							}
							else ok=false;
						}
					}
				}
				if (ok) break;
			}

			//--- display error reason -----------------
			for( int cluster=0; cluster<(CTC_Test.HEADS+3)/4; cluster++)
			{
				//--- calculate pumpspeed per cluster ----------
				int pumpSpeed = 0;
				int cnt = 0;
				for (int i=0; i<4; i++)
				{
					int headNo = 4*cluster+i;
					if (test.State[headNo]==CTC_Test.EN_State.running)
					{
						pumpSpeed += (int)RxGlobals.HeadStat.List[headNo].PumpSpeed;
						cnt++;
					}
				}
				if (cnt>0) pumpSpeed /= cnt;
				for (int i=0; i<4; i++)
				{
					int headNo = 4*cluster+i;
					if (test.State[headNo]==CTC_Test.EN_State.running)
					{
						if (!_inRange(RxGlobals.HeadStat.List[headNo].InkSupply.CylinderPres, checkCylinder))	
							RxGlobals.Events.AddItem(new LogItem("Head[{0}]: Cylinder pressure out of range {1}({2}..{3})", headNo+1, RxGlobals.HeadStat.List[headNo].InkSupply.CylinderPres, checkCylinder.Min, checkCylinder.Max){LogType = ELogType.eErrCont});
						if (!_inRange(RxGlobals.HeadStat.List[headNo].PresIn, checkIn))							
							RxGlobals.Events.AddItem(new LogItem("Head[{0}]: Inlet pressure out of range {1}({2}..{3})", headNo+1, RxGlobals.HeadStat.List[headNo].PresIn, checkIn.Min, checkIn. Max){LogType = ELogType.eErrCont});
						if (!_inRange(RxGlobals.HeadStat.List[headNo].PresOut, checkOut))						
							RxGlobals.Events.AddItem(new LogItem("Head[{0}]: Output pressure out of range {1}({2}..{3})", headNo+1, RxGlobals.HeadStat.List[headNo].PresOut, checkOut.Min, checkOut.Max){LogType = ELogType.eErrCont});
						if (!_inRange(pumpSpeed, checkPump))													
							RxGlobals.Events.AddItem(new LogItem("Head[{0}]: Pump speed out of range {1}({2}..{3})", headNo+1, pumpSpeed, checkPump.Min, checkPump.Max){LogType = ELogType.eErrCont});
					}
				}
			}
			test.Done(CTC_Test.EN_State.failed);
			return true;
		}

		//--- LeakTest ---------------------------------------------------------
		public void LeakTest(Action<bool> onDone)
		{
			Stop(null);

			Process = new Thread(() =>
			{
				RxBindable.Invoke(() => _Tests.Add(new CTC_Test() { Name = "Leak Test" }));

				//--------------------------------------------------
				CTC_Test test1 = new CTC_Test() { Step = "Air Pressure" };
				RxBindable.Invoke(() => _Tests.Add(test1));
				CTC_Test test2 = new CTC_Test() { Step = "Ink to Water" };
				RxBindable.Invoke(() => _Tests.Add(test2));
				CTC_Test test3 = new CTC_Test() { Step = "Conditioner Return" };
				RxBindable.Invoke(() => _Tests.Add(test3));
				CTC_Test test4 = new CTC_Test() { Step = "Flushline" };
				RxBindable.Invoke(() => _Tests.Add(test4));
				CTC_Test test5 = new CTC_Test() { Step = "Flush Valves" };
				RxBindable.Invoke(() => _Tests.Add(test5));
				CTC_Test test6 = new CTC_Test() { Step = "Ink Valves / Feed" };
				RxBindable.Invoke(() => _Tests.Add(test6));
				
				CTC_Settings settings = new CTC_Settings();
				CTC_Param pressurePar = settings.GetParam("Leak Test", test1.Step, "Pressure",		 500, 0);
				CTC_Param pressureTimeout = settings.GetParam("Leak Test", test1.Step, "Pressure Timeout",		 20000, 0);
				CTC_Param timePar     = settings.GetParam("Leak Test", test1.Step, "Check Time",	10000, 0);
				CTC_Param test1checkPresPar   = settings.GetParam("Leak Test", test1.Step, "Check Pressure",   400, 0);
				CTC_Param coolerPresPar  = settings.GetParam("Leak Test", "Cooler Pressure", "Cooler_Pressure", 400, 600);
				CTC_Param test3TimePar   = settings.GetParam("Leak Test", test3.Step, "Check Time",	10000, 0);
				CTC_Param test3PresPar   = settings.GetParam("Leak Test", test3.Step, "Check Pressure",   400, 0);
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
				_SetHeadValve(test1, 0, CTC_Test.HEADS, HEAD_VALVE_FLUSH);
				_CtrlAirPressure(test1, 0, CTC_Test.HEADS, pressurePar.Min, pressureTimeout.Min);
				_CheckHeadPIN(test1, 0, CTC_Test.HEADS, timePar.Min, test1checkPresPar.Min, test1checkPresPar.Max);
				_SendStop();
				_SetFluidValve(FLUID_VALVE_SHUTOFF, true);

				//--- test 2 ---------------------------------------------------
				test2.Start();
				_SetHeadValve(test2, 0, CTC_Test.HEADS, HEAD_VALVE_BOTH);
				CTC_Test.Wait(5000, DisplayTimer);
				_SetHeadValve(test2, 0, CTC_Test.HEADS, HEAD_VALVE_OFF);

				RxGlobals.Events.AddItem(new LogItem("_SetFluidValve(FLUID_VALVE_FLUSH, true)")); 

				CTC_Test.Wait(8000, DisplayTimer);
				_SetFluidValve(FLUID_VALVE_FLUSH, true);
				CTC_Test.Wait(1000, DisplayTimer);
				for(int head=0; head<CTC_Test.HEADS; head++)
				{
					if (test2.State[head]==CTC_Test.EN_State.running)
					{
						HeadStat stat = RxGlobals.HeadStat.List[head];
						UInt32   err  = RxGlobals.HeadStat.GetClusterErr(head/4);
						test2.SetResult(head, stat.Cooler_Pressure>=coolerPresPar.Min && stat.Cooler_Pressure<=coolerPresPar.Max);
						if (test2.State[head]==CTC_Test.EN_State.failed)
						{
							RxGlobals.Events.AddItem(new LogItem("Leak Test: Head[{0}].coolerPressure={1} ({2}..{3})", head+1, stat.Cooler_Pressure, coolerPresPar.Min, coolerPresPar.Max){LogType = ELogType.eErrCont}); 
							Console.WriteLine("Leak Test: Head[{0}].coolerPressure={1} ({2}..{3})", head, stat.Cooler_Pressure, coolerPresPar.Min, coolerPresPar.Max);
						}
					}
				}

				//--- test 3 ------------------------------------
				test3.Start();
				_SetFluidValve(FLUID_VALVE_RELEASE, true);
				_CheckHeadPIN(test3, 0, CTC_Test.HEADS, test3TimePar.Min, test3PresPar.Min, test3PresPar.Max);

				//--- test 4 ---------------------------
				test4.Start();
				_SetHeadValve(test4, 0, CTC_Test.HEADS, HEAD_VALVE_FLUSH);
				_CheckHeadPIN(test4, 0, CTC_Test.HEADS, test4TimePar.Min, test4PresPar.Min, test4PresPar.Max);
				_SetHeadValve(test4, 0, CTC_Test.HEADS, HEAD_VALVE_OFF);

				//--- test 5 ----------------------------
				test5.Start();
				_SetFluidValve(FLUID_VALVE_FLUSH, false);
				_CheckHeadPIN(test5, 0, CTC_Test.HEADS, test5TimePar.Min, test5PresPar.Min, test5PresPar.Max);

				//--- test 6----------------------------
				test6.Start();
				_SetHeadValve(test4, 0, CTC_Test.HEADS, HEAD_VALVE_BOTH);
				_SetFluidValve(FLUID_VALVE_FLUSH, false);
				_CheckHeadPIN(test6, 0, CTC_Test.HEADS, test6TimePar.Min, test6PresPar.Min, test6PresPar.Max);

				//--- END ---------------------------------------
				_SetFluidValve(FLUID_VALVE_SHUTOFF, false);
				for (int i=0; i<RxGlobals.PrintSystem.ColorCnt; i++)
				{
					_SetFluidValve(FLUID_CYLINDER_0+i, true);
				}
				CTC_Test.Wait(10000, DisplayTimer);
				_SendStop();
				if (onDone != null) RxBindable.Invoke(() => onDone(false));
			});
			Process.Start();
		}

		//--- _do_fluid_ctrlMode -------------------------
		private bool _do_fluid_ctrlMode(EFluidCtrlMode CtrlMode, bool[] isUsed, int timeout)
		{
			for (int isNo=0; isNo<RxGlobals.PrintSystem.ColorCnt; isNo++)
			{
				if (isUsed[isNo])
				{
					TcpIp.SFluidCtrlCmd cmd = new TcpIp.SFluidCtrlCmd();
					cmd.no = isNo;
					cmd.ctrlMode = CtrlMode;
				    RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref cmd);
				}
			}

			for (int time=0; time<timeout; time+=1000)
			{
				bool ok=true;
				for (int isNo=0; isNo<RxGlobals.PrintSystem.ColorCnt; isNo++)
				{
					if (isUsed[isNo])
					{
						Console.WriteLine("Fluid[{0}].ControlMode={1}", isNo, RxGlobals.InkSupply.List[isNo].CtrlMode);
						ok &= (RxGlobals.InkSupply.List[isNo].CtrlMode==EFluidCtrlMode.ctrl_purge_hard);
					}
				}
				if (ok) return true;
				CTC_Test.Wait(1000, DisplayTimer);					
			}
			return false;
		}

		//--- ValveTest ---------------------------------------------------------
		public void ValveTest(Action<bool> onDone)
		{
			Stop(null);

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

				CTC_Settings settings = new CTC_Settings();

				CTC_Test prep			= new CTC_Test(){Step = "Prepare"};
				CTC_Param prepPressure	= settings.GetParam("Valve Test", prep.Step, "Pressure",				  -45, 0);
				CTC_Param prepTime		= settings.GetParam("Valve Test", prep.Step, "Time",					10000, 0);

				CTC_Test test1 = new CTC_Test() { Step = "Print" };
				CTC_Param pressure		= settings.GetParam("Valve Test", test1.Step, "Pressure Inlet Cond",    -45, 0);
				CTC_Param checkTime		= settings.GetParam("Valve Test", test1.Step, "Check Time",				10000, 0);
				CTC_Param checkIn		= settings.GetParam("Valve Test", test1.Step, "Check Inlet",			30, 50);
				CTC_Param checkOut		= settings.GetParam("Valve Test", test1.Step, "Check Outlet",			90, 110);
				CTC_Param checkPump		= settings.GetParam("Valve Test", test1.Step, "Check Pump",				40, 45);
								
				CTC_Test test2 = new CTC_Test() { Step = "Test Valve"};
				CTC_Param delayTime2	= settings.GetParam("Valve Test", test2.Step, "Delay Time",				10000, 0);
				CTC_Param checkTime2	= settings.GetParam("Valve Test", test2.Step, "Check Time",				10000, 0);
				CTC_Param checkPump2	= settings.GetParam("Valve Test", test2.Step, "Check Pump",				80, 0);
				
				CTC_Test test3 = new CTC_Test() { Step = "Print 2" };
				CTC_Param checkTime3	= settings.GetParam("Valve Test", test3.Step, "Check Time",				10000, 0);
				CTC_Param checkIn3		= settings.GetParam("Valve Test", test3.Step, "Check Inlet",			30, 50);
			//	CTC_Param checkIn3b		= settings.GetParam("Valve Test", test3.Step, "Check Inlet2",			30, 50);
				CTC_Param checkOut3		= settings.GetParam("Valve Test", test3.Step, "Check Outlet",			90, 110);
				CTC_Param checkPump3	= settings.GetParam("Valve Test", test3.Step, "Check Pump",				40, 45);
				CTC_Param checkCylinder3= settings.GetParam("Valve Test", test3.Step, "Check Cylinder",			0, 1000);

				//--- switch off meniscus check --------------------------------
				_SetMeniscusCheck(false);
				//=== start prepare ink supplies =========================================
				RxBindable.Invoke(() => _Tests.Add(prep));
				prep.Start();
				for( int headNo=0; headNo<CTC_Test.HEADS; headNo++)
				{
					if (prep.State[headNo]==CTC_Test.EN_State.running) isUsed[headNo/RxGlobals.PrintSystem.HeadsPerColor] = true;
				}
				for (int isNo=0; isNo<RxGlobals.PrintSystem.ColorCnt; isNo++)
				{
				//	_SetFluidValve(FLUID_CYLINDER_0+isNo, true);
					if (isUsed[isNo])
					{
						TcpIp.SValue msg = new TcpIp.SValue(){no=isNo, value=(Int32)prepPressure.Min};
						RxGlobals.InkSupply.List[isNo].CylinderPresSet = prepPressure.Min;
						RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_PRESSURE, ref msg);

						TcpIp.SFluidCtrlCmd cmd = new TcpIp.SFluidCtrlCmd();
						cmd.no = isNo;
						cmd.ctrlMode = EFluidCtrlMode.ctrl_print;
				        RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref cmd);
					}
				}
				CTC_Test.Wait(prepTime.Min, DisplayTimer);
				// _do_fluid_ctrlMode(EFluidCtrlMode.ctrl_off, isUsed, 1000);
				for (int isNo=0; isNo<RxGlobals.PrintSystem.ColorCnt; isNo++)
				{
				//	_SetFluidValve(FLUID_CYLINDER_0+isNo, true);
					if (isUsed[isNo])
					{
						TcpIp.SFluidCtrlCmd cmd = new TcpIp.SFluidCtrlCmd();
						cmd.no = isNo;
						cmd.ctrlMode = EFluidCtrlMode.ctrl_off;
				        RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref cmd);
					}
				}

				//--- wait until pressure < 100 ---------------
				int cnt=0;
				do
				{
					Thread.Sleep(1000);
					cnt++;
					if (cnt>60)
					{
						prep.Done(CTC_Test.EN_State.failed);
						RxGlobals.Events.AddItem(new LogItem("Fluid pressure release timeout"){LogType = ELogType.eErrAbort});
						goto ValveTest_END;
					}
					ok=true;
					for (int isNo=0; isNo<RxGlobals.PrintSystem.ColorCnt; isNo++)
					{
						Console.WriteLine("Cylinder[{0}].pressure={1}", isNo, RxGlobals.InkSupply.List[isNo].CylinderPres);
						if (isUsed[isNo] && RxGlobals.InkSupply.List[isNo].CylinderPres>100) ok=false;
					}
				}
				while (!ok);
				prep.Done(CTC_Test.EN_State.ok);

				//=== TEST 1 =============================================
				RxBindable.Invoke(() => _Tests.Add(test1));
				test1.Start();

				//--- start ink supplies ------------------------------------
				for (int isNo=0; isNo<RxGlobals.PrintSystem.ColorCnt; isNo++)
				{
				//	_SetFluidValve(FLUID_CYLINDER_0+isNo, true);
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
				CTC_Test.Wait(checkTime.Min, DisplayTimer);
				ok = _CheckHeadPrinting(test1, 1000, checkIn, null, checkOut, checkPump);

				//===== test2: test Valve ==================================================================================
				test2.Start();
				RxBindable.Invoke(() => _Tests.Add(test2));
				_SetFluidValve(FLUID_VALVE_SHUTOFF, true);

				CTC_Test.Wait(delayTime2.Min, DisplayTimer);

				ok = _CheckHeadPrinting(test2, checkTime2.Min, null, null, null, checkPump2);

				_SetFluidValve(FLUID_VALVE_SHUTOFF, false);
				
				for (int isNo=0; isNo<RxGlobals.PrintSystem.ColorCnt; isNo++)
				{
					_SetFluidValve(FLUID_CYLINDER_0+isNo, false);
				}

				//=== test3: Print 2 ======================================================

				test3.Start();
				RxBindable.Invoke(() => _Tests.Add(test3));
				CTC_Test.Wait(checkTime3.Min, DisplayTimer);
				ok = _CheckHeadPrinting(test3, 1000, checkIn3, checkCylinder3, checkOut3, checkPump3);
			
			ValveTest_END:
				_SetMeniscusCheck(true);
				_SendStop();
				//--- END ---------------------------------------
				if (onDone != null) RxBindable.Invoke(() => onDone(_allOk(connected)));
			});
			Process.Start();
		}

		//--- FlowTest ---------------------------------------------------------
		public void FlowTest(Action<bool> onDone)
		{
			Stop(null);

			Process = new Thread(() =>
			{
				RxBindable.Invoke(() => _Tests.Add(new CTC_Test() { Name = "Flow Test" }));

				//--- connected -----------------------------------------------------
				CTC_Test connected = new CTC_Test() { Step = "Connected" };
				for(int head=0; head<CTC_Test.HEADS; head++)
				{
					HeadStat stat = RxGlobals.HeadStat.List[head];
					connected.SetResult(head, stat.Connected);
				}
				RxBindable.Invoke(() => _Tests.Add(connected));

				//--- -----------------------------------------------------------------------------
				CTC_Test flowTest    = new CTC_Test() { Step = "Flow" };
				RxBindable.Invoke(() => _Tests.Add(flowTest));

				CTC_Settings settings   = new CTC_Settings();
				CTC_Param timePar       = settings.GetParam("Flow Test", flowTest.Step, "Time",					 2000,   0);
				CTC_Param delayPar      = settings.GetParam("Flow Test", flowTest.Step, "Test Delay",			50000,   0);
				CTC_Param flowPar	    = settings.GetParam("Flow Test", flowTest.Step, "Flow",					   30,  50);
				CTC_Param pressure      = settings.GetParam("Flow Test", flowTest.Step, "Pressure Inlet Cond",    -45,   0);
				CTC_Param checkIn		= settings.GetParam("Flow Test", flowTest.Step, "Check Inlet",			   30,  55);
				CTC_Param checkIn2		= settings.GetParam("Flow Test", flowTest.Step, "Check Inlet2",			  120, 220);
				CTC_Param checkOut		= settings.GetParam("Flow Test", flowTest.Step, "Check Outlet",			   80, 120);
				CTC_Param checkPump		= settings.GetParam("Flow Test", flowTest.Step, "Check Pump",			   35,  55);

				//-------------------- start fluid -------------
				{
					TcpIp.SValue msg = new TcpIp.SValue(){no=-1, value=1};
					RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_HEADS_PER_COLOR, ref msg);
				}
				for (int isNo=0; isNo<RxGlobals.PrintSystem.ColorCnt; isNo++)
				{
					TcpIp.SValue msg = new TcpIp.SValue(){no=isNo, value=(Int32)pressure.Min};
					RxGlobals.InkSupply.List[isNo].CylinderPresSet = pressure.Min;
					RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_PRESSURE, ref msg);

					TcpIp.SFluidCtrlCmd cmd = new TcpIp.SFluidCtrlCmd();
					cmd.no = isNo;
					cmd.ctrlMode = EFluidCtrlMode.ctrl_print;
					RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref cmd);
				}
		
				for (int head = 0; head < CTC_Test.HEADS; head++)
				{
					int isNo=head/RxGlobals.PrintSystem.HeadsPerColor;
					for (int i=0; i<RxGlobals.PrintSystem.ColorCnt; i++)
					{
						_SetFluidValve(FLUID_CYLINDER_0+i, i==isNo);
					}
					if (CTC_Test.Overall.State[head] != CTC_Test.EN_State.failed)
					{
						RxBindable.Invoke(() => flowTest.State[head] = CTC_Test.EN_State.running);
						
						//--- start head -----------------------------------
						for(int h=0; h<CTC_Test.HEADS; h++)
						{
							TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
							msg.no = h;
							if (h/RxGlobals.PrintSystem.HeadsPerColor == isNo)
							{
								if (h==head) msg.ctrlMode = EFluidCtrlMode.ctrl_test;
								else         msg.ctrlMode = EFluidCtrlMode.ctrl_off;
							}
							else msg.ctrlMode = EFluidCtrlMode.ctrl_print;

							RxGlobals.RxInterface.SendMsg(TcpIp.CMD_CTC_HEAD_CTRL_MODE, ref msg);
						}

						//--- check flow --------------------------
						if (!_FlowSensor.StartMeasurement())
							RxBindable.Invoke(() =>DisplayFlow("NO SENSOR"));
						int flow;
						int interval=1000;
						int t0=Environment.TickCount;
						int delay;
						for (int time = 0; time < timePar.Min && flowTest.State[head] == CTC_Test.EN_State.running; )
						{
							DisplayTimer(timePar.Min-time);
							time += interval;
							delay = Environment.TickCount-t0;
							delay = time-delay;
							if (delay>0) Thread.Sleep(delay);

							if (!_FlowSensor.MeasureFlow(out flow))
							{
								RxBindable.Invoke(() =>flowTest.SetHeadState(head, CTC_Test.EN_State.failed));
								break;
							}

							if (time >= delayPar.Min)
							{
								Console.WriteLine("{0}: flow={1}ml/min tick={2}, PIN={3}, PIN2={4} POUT={5}, Pump={6}", 
									time, flow, Environment.TickCount-t0,
									RxGlobals.HeadStat.List[head].PresIn, RxGlobals.HeadStat.List[head].PresIn2, -RxGlobals.HeadStat.List[head].PresOut, RxGlobals.HeadStat.List[head].PumpSpeed);
								if (RxGlobals.HeadStat.List[head].PresIn<checkIn.Min || RxGlobals.HeadStat.List[head].PresIn>checkIn.Max)
								{
									RxGlobals.Events.AddItem(new LogItem("Flow test: Head[{0}]: PresIn={1} out ou range {2}..{3}", head+1,  RxGlobals.HeadStat.List[head].PresIn, checkIn.Min, checkIn.Max){LogType = ELogType.eErrCont}); 
									RxBindable.Invoke(() =>flowTest.SetHeadState(head, CTC_Test.EN_State.failed));
								}
								if (RxGlobals.HeadStat.List[head].PresIn2<checkIn2.Min || RxGlobals.HeadStat.List[head].PresIn2>checkIn2.Max)
								{
									RxGlobals.Events.AddItem(new LogItem("Flow test: Head[{0}]: PresIn2={1} out ou range {2}..{3}", head+1, RxGlobals.HeadStat.List[head].PresIn2, checkIn2.Min, checkIn2.Max){LogType = ELogType.eErrCont}); 
									RxBindable.Invoke(() =>flowTest.SetHeadState(head, CTC_Test.EN_State.failed));
								}
								if (RxGlobals.HeadStat.List[head].PresOut<checkOut.Min || RxGlobals.HeadStat.List[head].PresOut>checkOut.Max)
								{
									RxGlobals.Events.AddItem(new LogItem("Flow test: Head[{0}]: PresOut={1} out ou range {2}..{3}", head+1,  RxGlobals.HeadStat.List[head].PresOut, checkOut.Min, checkOut.Max){LogType = ELogType.eErrCont}); 
									RxBindable.Invoke(() =>flowTest.SetHeadState(head, CTC_Test.EN_State.failed));
								}
								if (RxGlobals.HeadStat.List[head].PumpSpeed<checkPump.Min || RxGlobals.HeadStat.List[head].PumpSpeed>checkPump.Max)
								{
									RxGlobals.Events.AddItem(new LogItem("Flow test: Head[{0}]: PumpSpeed={1} out ou range {2}..{3}", head+1, RxGlobals.HeadStat.List[head].PumpSpeed, checkPump.Min, checkPump.Max){LogType = ELogType.eErrCont}); 
									RxBindable.Invoke(() =>flowTest.SetHeadState(head, CTC_Test.EN_State.failed));
								}
							}
							string str;
							if (time >= delayPar.Min) str=string.Format("Flow: CHECK\n{0} ml/min", flow);
							else                      str=string.Format("Flow: prep\n{0} ml/min", flow);
							RxBindable.Invoke(() =>DisplayFlow(str));

							//--- check ------------------------------------------
							if (time >= delayPar.Min && (flow<flowPar.Min || flow>flowPar.Max))
							{
								Console.WriteLine("Flow ERROR");
								RxGlobals.Events.AddItem(new LogItem("Long run test: Head[{0}] Flow={1} ml/min at Time={2} ms", head+1, flow, time){LogType = ELogType.eErrCont}); 
								RxBindable.Invoke(() =>flowTest.SetHeadState(head, CTC_Test.EN_State.failed));
							}
						}

						if (flowTest.State[head] == CTC_Test.EN_State.running) RxBindable.Invoke(() =>flowTest.SetHeadState(head, CTC_Test.EN_State.ok));
						_FlowSensor.StopMeasurement();
						DisplayTimer(0);
					}
				}
				_SendStop();
				//--- END ---------------------------------------
				if (onDone != null) RxBindable.Invoke(() => onDone(_allOk(connected)));
			});
			Process.Start();
		}

		//--- LongRun ---------------------------------------------------------
		public void LongRun(Action<bool> onDone)
		{
			Stop(null);

			Process = new Thread(() =>
			{
				RxBindable.Invoke(() => _Tests.Add(new CTC_Test() { Name = "Long Run" }));

				//--- connected -----------------------------------------------------
				CTC_Test connected = new CTC_Test() { Step = "Connected" };
				for(int head=0; head<CTC_Test.HEADS; head++)
				{
					HeadStat stat = RxGlobals.HeadStat.List[head];
					connected.SetResult(head, stat.Connected);
				}
				RxBindable.Invoke(() => _Tests.Add(connected));

				//--------------------------------------------------
				CTC_Test start = new CTC_Test() { Step = "Start" };
				CTC_Test longRunTest = new CTC_Test() { Step = "Long Run" };
				RxBindable.Invoke(() => _Tests.Add(start));
				RxBindable.Invoke(() => _Tests.Add(longRunTest));

				CTC_Settings settings = new CTC_Settings();
	
				//=== ALL HEADS ========================================================================================
				CTC_Param startTime     = settings.GetParam("LongRun", start.Step, "Time", 120000, 0);
				CTC_Param pressure      = settings.GetParam("LongRun", longRunTest.Step, "Pressure Inlet Cond", -45, 0);
				CTC_Param recoveryPar	= settings.GetParam("LongRun", longRunTest.Step, "Recovery Time",		5000, 0);
				CTC_Param filterPar		= settings.GetParam("LongRun", longRunTest.Step, "Filter Time",			5000, 0);
				CTC_Param checkIn		= settings.GetParam("LongRun", longRunTest.Step, "Check Inlet",			30, 50);
				CTC_Param checkOut		= settings.GetParam("LongRun", longRunTest.Step, "Check Outlet",		90, 110);
				CTC_Param checkPump		= settings.GetParam("LongRun", longRunTest.Step, "Check Pump",			40, 45);

				RxGlobals.Events.AddItem(new LogItem("Long Run"){LogType = ELogType.eErrLog}); 

				start.Start();

				//-------------------- start fluid -------------
				for (int isNo=0; isNo<RxGlobals.PrintSystem.ColorCnt; isNo++)
				{
					TcpIp.SValue msg = new TcpIp.SValue(){no=isNo, value=(Int32)pressure.Min};
					RxGlobals.InkSupply.List[isNo].CylinderPresSet = pressure.Min;
					RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_PRESSURE, ref msg);

					TcpIp.SFluidCtrlCmd cmd = new TcpIp.SFluidCtrlCmd();
					cmd.no = isNo;
					cmd.ctrlMode = EFluidCtrlMode.ctrl_print;
					RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref cmd);
				}

				//--- wait until system stable ---------------

			//	_LongRun_Running = _CheckHeadPrinting(start, startTime.Min, checkIn, checkOut, checkPump);
				CTC_Test.Wait(startTime.Min, DisplayTimer);
				_LongRun_Running = _CheckHeadPrinting(start, 1000, checkIn, null, checkOut, checkPump);

				//_LongRun_Running=true;
				if (_LongRun_Running)
				{
					int time=0;
					const int interval=500;
					int[] testDelay = new int[CTC_Test.HEADS];
					int[] errCnt    = new int[CTC_Test.HEADS];

					RxGlobals.Events.AddItem(new LogItem("PresIn    = [{0} .. {1}]", checkIn.Min, checkIn.Max){LogType = ELogType.eErrLog}); 
					RxGlobals.Events.AddItem(new LogItem("PresOut   = [{0} .. {1}]", checkOut.Min, checkOut.Max){LogType = ELogType.eErrLog}); 
					RxGlobals.Events.AddItem(new LogItem("PumpSpeed = [{0} .. {1}]", checkPump.Min, checkPump.Max){LogType = ELogType.eErrLog}); 

					int maxErr=0;
					if (filterPar.Min>0) maxErr = filterPar.Min/interval;
					longRunTest.Start();


					while(_LongRun_Running)
					{
						Thread.Sleep(interval);
						time+=interval;
						DisplayTimer(time);
						bool running=false;
						for( int head=0; head<CTC_Test.HEADS; head++)
						{
							if (longRunTest.State[head]==CTC_Test.EN_State.running)
							{
								running=true;
								if (testDelay[head]>0) testDelay[head] -= interval;

								if (testDelay[head]<=0)
								{
									int v1=RxGlobals.HeadStat.List[head].PresIn;
									int v2=RxGlobals.HeadStat.List[head].PresOut;
									int v3=(int)RxGlobals.HeadStat.List[head].PumpSpeed;
									bool r1 = _inRange(v1, checkIn);
									bool r2 = _inRange(v2, checkOut);
									bool r3 = _inRange(v3, checkPump);

									Console.WriteLine("Conditioner[{0}].PIN={1}, POUT={2}, Pump={3}, {4} {5} {6} errCnt={7}", 
										head, v1, v2, v3, r1, r2, r3, errCnt[head]);
									if (!r1 || !r2 || !r3)
									{
										if (++errCnt[head]>maxErr)
										{
											if (!r1) RxGlobals.Events.AddItem(new LogItem("Long run all test: Head[{0}] PresIn={1}({2}..{3}) at Time={4} ms", head+1, v1, checkIn.Min, checkIn.Max, time){LogType = ELogType.eErrCont}); 
											if (!r2) RxGlobals.Events.AddItem(new LogItem("Long run all test: Head[{0}] PresOut={1}({2}..{3}) at Time={4} ms", head+1, v2, checkOut.Min, checkOut.Max, time){LogType = ELogType.eErrCont}); 
											if (!r3) RxGlobals.Events.AddItem(new LogItem("Long run all test: Head[{0}] PumpSpeed={1}({2}..{3}) at Time={4} ms", head+1, v3, checkPump.Min, checkPump.Max, time){LogType = ELogType.eErrCont}); 
											
											TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd(){no=head, ctrlMode=EFluidCtrlMode.ctrl_off };
											RxGlobals.RxInterface.SendMsg(TcpIp.CMD_CTC_HEAD_CTRL_MODE, ref msg);
											
											RxBindable.Invoke(()=>longRunTest.State[head] = CTC_Test.EN_State.failed);
											int cluster=head/4;
											for(int i=0; i<4; i++) testDelay[4*cluster+i] = recoveryPar.Min;
										}
									}
									else errCnt[head]=0;
								}
							}
						}
						if (!running)
						{
							Console.WriteLine("none running");
							_LongRun_Running = false;
						}
					}
					longRunTest.Done(CTC_Test.EN_State.ok);
				}
				_SendStop();

				//--- END ---------------------------------------
				if (onDone != null) RxBindable.Invoke(() => onDone(false));
			});
			Process.Start();
		}

		//--- Empty ---------------------------------------------------------
		public void Empty(Action<bool> onDone)
		{
			Stop(null);

			Process = new Thread(() =>
			{
				RxBindable.Invoke(() => _Tests.Add(new CTC_Test() { Name = "Empty" }));
						

				//--------------------------------------------------
				CTC_Test runEmpty = new CTC_Test() { Step = "Empty" };
				runEmpty.Start();
				RxBindable.Invoke(() => _Tests.Add(runEmpty));

				CTC_Settings settings = new CTC_Settings();
				CTC_Param waitTime1	= settings.GetParam("Empty", runEmpty.Step, "Time1",				20000, 0);
				CTC_Param waitTime2	= settings.GetParam("Empty", runEmpty.Step, "Time2",				10000, 0);

				_SetMeniscusCheck(false);
				//--- set fluid pressure
				{
					TcpIp.SFluidTestCmd msg = new TcpIp.SFluidTestCmd();
					msg.airValve = 1;
					msg.airPressure = 2000;
					msg.no = 0;
					RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_TEST, ref msg);
				}

				/*
				for (int isNo=0; isNo<RxGlobals.PrintSystem.ColorCnt; isNo++)
				{
					_SetFluidValve(FLUID_CYLINDER_0+isNo, true);
				}
				*/

				// _CtrlAirPressure(runEmpty, 0, TcpIp.HEAD_CNT, 2000);
				for (int head = 0; head < CTC_Test.HEADS; head++)
				{
					if (runEmpty.State[head] == CTC_Test.EN_State.running)
					{
						TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
						msg.no = head;
						msg.ctrlMode = EFluidCtrlMode.ctrl_test;

						RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_FLUID_CTRL_MODE, ref msg);
					}
				}

				CTC_Test.Wait(waitTime1.Min, DisplayTimer);

				//--- switch ink system off ----------------------------
				{
					TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
					msg.no = -1;
					msg.ctrlMode = EFluidCtrlMode.ctrl_off;
					RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
				}
				for (int head = 0; head < CTC_Test.HEADS; head++)
				{
					TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
					msg.no = head;
					msg.ctrlMode = EFluidCtrlMode.ctrl_off;

					RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_FLUID_CTRL_MODE, ref msg);
				}

				CTC_Test.Wait(waitTime2.Min, DisplayTimer);
				_SendStop();

				//--- END ---------------------------------------
				if (onDone != null) RxBindable.Invoke(() => onDone(false));
			});
			Process.Start();
		}


		//--- _SendStop ------------------------------------
		private void _SendStop()
		{
			{
				TcpIp.SValue msg = new TcpIp.SValue(){no=-1, value=RxGlobals.PrintSystem.HeadsPerColor};
				RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_HEADS_PER_COLOR, ref msg);
			}

			//--- switch ink system off ----------------------------
			{
				TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
				msg.no = -1;
				msg.ctrlMode = EFluidCtrlMode.ctrl_off;
				RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
			}

			for (int head = 0; head < CTC_Test.HEADS; head++)
			{
				TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
				msg.no = head;
				msg.ctrlMode = EFluidCtrlMode.ctrl_off;

				RxGlobals.RxInterface.SendMsg(TcpIp.CMD_HEAD_FLUID_CTRL_MODE, ref msg);
			}
			_SetMeniscusCheck(false);
			for (int i=FLUID_VALVE_SHUTOFF; i<=FLUID_VALVE_RELEASE; i++)
				_SetFluidValve(i, false);
			RxBindable.Invoke(()=>DisplayTimer(0));
			_FlowSensor.StopMeasurement();
			RxBindable.InvokeDelayed(500, () => DisplayFlow(""));
		}

		//--- Stop ---------------------------------------------------------
		public void Stop(Action<bool> onDone)
		{
			if (Process != null)
			{
				if(_LongRun_Running) _LongRun_Running=false;
				else Process.Abort();
				Thread.Sleep(1000);
			}
			_SendStop();
			if (onDone != null) RxBindable.Invoke(() => onDone(false));
		}

		//--- Property Tests ---------------------------------------
		private ObservableCollection<CTC_Test> _Tests = new ObservableCollection<CTC_Test>();
		public ObservableCollection<CTC_Test> Tests
		{
			get { return _Tests; }
			set { SetProperty(ref _Tests, value); }
		}

        public int HEAD_CNT { get; private set; }
	}
}
