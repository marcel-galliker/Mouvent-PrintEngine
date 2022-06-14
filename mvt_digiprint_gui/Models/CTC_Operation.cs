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

		//--- LeakTest ---------------------------------------------------------
		public void LeakTest(Action onDone)
		{
			Process = new Thread(() =>
			{
				RxBindable.Invoke(() => _Tests.Add(new CTC_Test() { Name = "Leak Test" }));

				//--------------------------------------------------
				CTC_Test start = new CTC_Test() { Step = "Ink Tank Pressure" };
				
				CTC_Settings settings = new CTC_Settings();
				CTC_Param startPar1 = settings.GetParam("Leak Test", start.Step, "Pressure", 500, 0);
				CTC_Param startPar2 = settings.GetParam("Leak Test", start.Step, "WaitTime", 10000, 0);
				CTC_Param startPar3 = settings.GetParam("Leak Test", start.Step, "PIN",		 10, 20);

				//--------------------------------------------------
				start.SimuRunning();
				RxBindable.Invoke(() => _Tests.Add(start));
				// Start pumping 

				TcpIp.SCTC_OperationMsg msg = new TcpIp.SCTC_OperationMsg(){ cmd=TcpIp.cdc_leak_test, step=1, par=(Int32)startPar1.Min};
				for(msg.headNo=0; msg.headNo<CTC_Test.HEADS; msg.headNo++)
				{
					RxGlobals.RxInterface.SendMsg(TcpIp.CMD_CTC_OPERATION, ref msg);
				}

				bool ok=false;
				do
				{
					Thread.Sleep(200);
					ok=true;
					for(msg.headNo=0; msg.headNo<CTC_Test.HEADS; msg.headNo++)
					{
						if (start.State[msg.headNo]==CTC_Test.EN_State.running)
						{
							int isNo=msg.headNo/RxGlobals.PrintSystem.HeadsPerColor;
							Console.WriteLine("CylinderPres[{0}]={1}", msg.headNo, RxGlobals.InkSupply.List[isNo].CylinderPres);
							if (RxGlobals.InkSupply.List[isNo].CylinderPres < startPar1.Min) ok=false;
						}
					}
				} while (!ok);

				//--- stop pumping ------------------------------				
				msg.step=2;
				msg.par=0;
				for(msg.headNo=0; msg.headNo<CTC_Test.HEADS; msg.headNo++)
				{
					RxGlobals.RxInterface.SendMsg(TcpIp.CMD_CTC_OPERATION, ref msg);
				}

				Console.WriteLine("Cylinder Pressure ok");
				Thread.Sleep(startPar2.Min);
				// check
				Console.WriteLine("Checking Conditioners");
				for(int head=0; head<CTC_Test.HEADS; head++)
				{
					if (start.State[head]==CTC_Test.EN_State.running)
					{
						HeadStat stat = RxGlobals.HeadStat.List[head];
						Console.WriteLine("Head[{0}].PresIn={1}", head, stat.PresIn);
						start.SetResult(head, stat.PresIn>=startPar3.Min && stat.PresIn<=startPar3.Max);
					}
				}

				/*
				if (_Simulation)
				{
					Thread.Sleep(1000);
					RxBindable.Invoke(() => result.SimuFailed(new List<int>() { 1, 9 }));
				}

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
				*/

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
