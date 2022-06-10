using RX_Common;
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

		private const bool _Simulation = true;
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

			_Tests.Clear();
			CTC_Test.Overall.ResetState();
			_Tests.Add(CTC_Test.Overall);
		}

		//--- ElectronicsTest ---------------------------------------------------------
		public void ElectronicsTest(Action onDone)
		{
			Process = new Thread(()=>
			{
				CTC_Test result;

				RxBindable.Invoke(()=>_Tests.Add(new CTC_Test(){Name="Electronics" }));

				//--------------------------------------------------
				result = new CTC_Test() { Step = "Connected" };
				result.SimuRunning();
				RxBindable.Invoke(() => _Tests.Add(result));

				if (_Simulation)
				{
					Thread.Sleep(1000);
					RxBindable.Invoke(() => result.SimuFailed(new List<int>() { 1, 9 }));
				}

				//----------------------------------------------------------------
				result = new CTC_Test() { Step = "Cooler Pressure" };
				result.SimuRunning();

				RxBindable.Invoke(() => _Tests.Add(result));
				Thread.Sleep(1000);
				if (_Simulation)
				{
					Thread.Sleep(1000);
					RxBindable.Invoke(() => result.SimuFailed(new List<int>() { 4 }));
				}

				//----------------------------------------------------------------
				result = new CTC_Test() { Step = "Cooler Temperature" };
				result.SimuRunning();

				RxBindable.Invoke(() => _Tests.Add(result));
				Thread.Sleep(1000);
				if (_Simulation)
				{
					Thread.Sleep(1000);
					RxBindable.Invoke(() => result.SimuFailed(new List<int>() { 7 }));
				}

				//----------------------------------------------------------------
				result = new CTC_Test() { Step = "Power -36V" };

				if (_Simulation)
				{
					Thread.Sleep(1000);
					RxBindable.Invoke(() => result.SimuFailed(new List<int>() { 10 }));
					RxBindable.Invoke(() => _Tests.Add(result));
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
				CTC_Test result;

				RxBindable.Invoke(() => _Tests.Add(new CTC_Test() { Name = "Leak Test" }));

				//--------------------------------------------------
				result = new CTC_Test() { Step = "Ink Tank Pressure" };
				result.SimuRunning();
				RxBindable.Invoke(() => _Tests.Add(result));

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

		//--- Off ---------------------------------------------------------
		public void Off(Action onDone)
		{
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
