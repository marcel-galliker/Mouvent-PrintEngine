using RX_Common;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace rx_CamLib
{
	public enum ECamFunctionState
	{
		undef,
		running,
		ending,
		done,
		aborted,
		error
	};

	public enum ECamFunction
	{
		CamNoFunction,
		CamFindMark_1,	// vertical lines (WEB)	
		CamFindMark_2,	// horizontal line (WEB)
		CamFindMark_3,	// end of line (SCAN)
		CamMeasureAngle,
		CamMeasureStitch,
		CamMeasureDist,
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
			Console.WriteLine ("create RxCamFunctions");
			_Camera.SetBinarizationMode(RxCam.ENBinarizeMode.BinarizeMode_Auto);
		}

		//--- Property Simulation ---------------------------------------
		private bool _Simulation=false;
		public bool Simulation
		{
			get { return _Simulation; }
			set { SetProperty(ref _Simulation,value); }
		}

		//--- Off ------------------------------------
		public void Off()
		{
			_Camera.SetMeasureMode(RxCam.ENMeasureMode.MeasureMode_Off);
		}

		//--- FindMark --------------------------------------------------
		public void FindMark(bool horizontal)
		{
			_Camera.SetBinarizationMode(RxCam.ENBinarizeMode.BinarizeMode_Auto);
			//set very small LineAspectLimit for StartLines
			_Camera.SetLinesHorizontal(horizontal);
			_Camera.SetLineAspectLimit(3);
			_Camera.NumExtraErodes=1;
			_Camera.SetDisplayMode(RxCam.ENDisplayMode.Display_AllLines);
			if (horizontal) 
			{
				_Camera.SetMinNumStartLines(1);
				_Camera.SetMeasureMode(RxCam.ENMeasureMode.MeasureMode_StartLinesCont);
			}
			else            
			{
				_Camera.SetMinNumStartLines(3);
				_Camera.SetMeasureMode(RxCam.ENMeasureMode.MeasureMode_StartLines);
			}
            if (_Simulation)
            {
                new Task(() =>
                {
                    Task.Delay(5000).Wait();
                  //  RxBindable.Invoke(() => markFound());	Callback!
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

		//--- MeasureAngle --------------------------------
		public void MeasureAngle()
		{
			_Camera.SetBinarizationMode(RxCam.ENBinarizeMode.BinarizeMode_Auto);
			_Camera.SetLinesHorizontal(false);
			_Camera.NumExtraErodes=3;
			_Camera.SetLineAspectLimit(5);
			_Camera.SetDisplayMode(RxCam.ENDisplayMode.Display_Correction);
			_Camera.DoMeasures(RxCam.ENMeasureMode.MeasureMode_Angle, 10, 10);
		}

		//--- MeasureStitch --------------------------------
		public void MeasureStitch()
		{
			_Camera.SetBinarizationMode(RxCam.ENBinarizeMode.BinarizeMode_Auto);
			_Camera.SetLinesHorizontal(false);
			_Camera.NumExtraErodes=3;
			_Camera.SetLineAspectLimit(5);
			_Camera.SetDisplayMode(RxCam.ENDisplayMode.Display_Correction);
			_Camera.DoMeasures(RxCam.ENMeasureMode.MeasureMode_Stitch, 10, 10);
		}
	}
}
