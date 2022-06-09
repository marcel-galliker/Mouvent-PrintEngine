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

		//--- Constructor -----------------------------------------------------------------
		public CTC_Operation()
		{
			Clear();
		}

		//--- Clear ----------------------------
		public void Clear()
		{
			_Tests.Clear();
			_Tests.Add(CTC_Test.Overall);
		}

		//--- ElectronicsTest ---------------------------------------------------------
		public void ElectronicsTest()
		{
			Thread process=new Thread(()=>
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
			});
			process.Start();
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
