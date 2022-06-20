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

		//--- _CtrlAirPressure ---------------------------------------
		private bool _CtrlAirPressure(CTC_Test test, int pressure, bool shutoffValve)
		{
			TcpIp.SFluidTestCmd msg = new TcpIp.SFluidTestCmd();
			msg.airValve = 1;
			msg.airPressure=(Int32)(pressure*11/10);
			msg.shutoffValve = Convert.ToInt32(shutoffValve);
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
				if (!ok) return false;
			}
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

				bool ok = _CtrlAirPressure(test1, startPar1.Min, false);

				//--- check air pressure stays in range ---------------------
				Console.WriteLine("AirPressure Pressure ok for {0} sek", startPar2.Min);
				TcpIp.SFluidTestCmd msg = new TcpIp.SFluidTestCmd();
				msg.shutoffValve  = 0;
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

				if (ok) 
				{
					CTC_Test test2 = new CTC_Test() { Step = "Ink Tank Pressure" };
					RxBindable.Invoke(() => _Tests.Add(test2));

					_SetHeadValve(test2, VALVE_FLUSH);
					//--- set Shutoff valves ------------------------------				
					ok = _CtrlAirPressure(test2, startPar1.Min, true);

					//--- check conditioner IN-Pressure stays in range ---------------------
					Console.WriteLine("conditioner IN-Pressure ok for {0} sek", startPar2.Min);
					ok=_CheckHeadPIN(test2, startPar2.Min, startPar3.Min);
				}

				if (ok)
				{

				}

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
				CTC_Test result;

				RxBindable.Invoke(() => _Tests.Add(new CTC_Test() { Name = "Valve Test" }));

				//--------------------------------------------------
				result = new CTC_Test() { Step = "test 1" };
				result.SimuRunning();
				RxBindable.Invoke(() => _Tests.Add(result));

				if (_Simulation)
				{
					Thread.Sleep(1000);
					RxBindable.Invoke(() => result.SimuFailed(new List<int>() { 1, 9 }));
				}

				//--------------------------------------------------
				result = new CTC_Test() { Step = "test 2" };
				result.SimuRunning();
				RxBindable.Invoke(() => _Tests.Add(result));

				if (_Simulation)
				{
					Thread.Sleep(1000);
					RxBindable.Invoke(() => result.SimuFailed(new List<int>() { 1, 9 }));
				}

				//--------------------------------------------------
				result = new CTC_Test() { Step = "test 3" };
				result.SimuRunning();
				RxBindable.Invoke(() => _Tests.Add(result));

				if (_Simulation)
				{
					Thread.Sleep(1000);
					RxBindable.Invoke(() => result.SimuFailed(new List<int>() { 1, 9 }));
				}

				//--------------------------------------------------
				result = new CTC_Test() { Step = "test 4" };
				result.SimuRunning();
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
				result.SimuRunning();
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

		//--- Stop ---------------------------------------------------------
		public void Stop(Action onDone)
		{
			//--- switch ink system off ----------------------------
			{
				TcpIp.SFluidCtrlCmd msg = new TcpIp.SFluidCtrlCmd();
				msg.no = -1;
				msg.ctrlMode = EFluidCtrlMode.ctrl_off;
				RxGlobals.RxInterface.SendMsg(TcpIp.CMD_FLUID_CTRL_MODE, ref msg);
			}

			Process = new Thread(() =>
			{
				CTC_Test result;

				//--------------------------------------------------
				result = new CTC_Test() { Name = "Off" };
				result.SimuRunning();
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
