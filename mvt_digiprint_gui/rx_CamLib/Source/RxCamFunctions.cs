using RX_Common;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System.Threading.Tasks;

namespace rx_CamLib
{
	public enum ECamFunctionState
	{
		undef,
		running,
		done,
		aborted,
		error
	};

	public enum ECamFunctions
	{
		CamFindMark,			
		CamMeasurePosition
	};

	public class SMarkPosition : RxBindable
	{
		//--- Property Web ---------------------------------------
		private int _Web;
		public int Web
		{
			get { return _Web; }
			set { SetProperty(ref _Web,value); }
		}

		//--- Property Scanner ---------------------------------------
		private int _Scanner;
		public int Scanner
		{
			get { return _Scanner; }
			set { SetProperty(ref _Scanner,value); }
		}
	};

	public class SHeadPosition : RxBindable
	{
		//--- Property angle ---------------------------------------
		private int _Angle;
		public int Angle
		{
			get { return _Angle; }
			set { SetProperty(ref _Angle,value); }
		}

		//--- Property Stitch ---------------------------------------
		private int _Stitch;
		public int Stitch
		{
			get { return _Stitch; }
			set { SetProperty(ref _Stitch,value); }
		}

		//--- Property Distance ---------------------------------------
		private int _Distance;
		public int Distance
		{
			get { return _Distance; }
			set { SetProperty(ref _Distance,value); }
		}
	};

	public class RxCamFunctions : RxBindable
	{
		private RxCam _Camera;
		public RxCamFunctions(RxCam cam)
		{
			_Camera = cam;
		}

		//--- Property Simulation ---------------------------------------
		private bool _Simulation=false;
		public bool Simulation
		{
			get { return _Simulation; }
			set { SetProperty(ref _Simulation,value); }
		}

		//--- FindMark --------------------------------------------------
		public void FindMark(Action markFound)
		{
			if (_Simulation)
			{
				new Task(() =>
				{
					Task.Delay(5000).Wait();
					RxBindable.Invoke(()=>markFound());
				}).Start();
			}
		}

		//--- MeasureMark --------------------------------
		public void MeasureMark(Action<SMarkPosition> measured)
		{
			if (_Simulation)
			{
				new Task(() =>
				{
					Task.Delay(1000).Wait();
					RxBindable.Invoke(()=>
					{
						SMarkPosition pos = new SMarkPosition() {Web=100, Scanner=200 };
						measured(pos);
					});
				}).Start();
			}
		}

		//--- MeasurePosition --------------------------------
		public void MeasurePosition(Action<SHeadPosition> measured)
		{
			if (_Simulation)
			{
				new Task(() =>
				{
					Task.Delay(1000).Wait();
					RxBindable.Invoke(()=>
					{
						SHeadPosition pos = new SHeadPosition();
						pos.Angle=100;
						pos.Stitch=200;
						pos.Distance=300;
						measured(pos);
					});
				}).Start();
			}
		}

	}
}
