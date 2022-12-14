using RX_Common;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using static rx_CamLib.RxCam;

namespace rx_CamLib
{
	public enum ECamFunctionState
	{
		undef,
		printing,
		runningCam,
		endingCam,
		waitRob,
		runningRob,
		done,
		aborted,
		error,
		manualCw,
		manualCcw
	};

	public enum ECamFunction
	{
		CamDummy,
		CamPrint,
		CamConfirmFocus,	
		CamFindLines_Vertical,		// vertical lines (WEB)	
		CamFindLine_Horzizontal,	// horizontal line (WEB)
		CamFindFirstAngle,
		CamMeasureAngle,
		CamMeasureStitch,
		CamMeasureDist,
		CamMeasureRegAngle,
		CamMeasureRegStitch,
		CamMoveScan,
		CamMoveWeb,
		I1Calibrate,
		I1Measure,
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
		public event CameraCallBack SimuCallback = null;
		private bool _Simulation=false;
		public bool Simulation
		{
			get { return _Simulation; }
			set { SetProperty(ref _Simulation,value); }
		}

		//--- Off ------------------------------------
		public void Off()
		{
			RxBindable.Invoke(()=>_Camera.SetMeasureMode(RxCam.ENMeasureMode.MeasureMode_Off));
		}

		//--- FindLines_Vertical -------------------------
		public void FindLines_Vertical()
		{
			RxBindable.Invoke(() =>
			{
				_Camera.SetBinarizationMode(RxCam.ENBinarizeMode.BinarizeMode_Auto);
				//set very small LineAspectLimit for StartLines
				_Camera.SetLinesHorizontal(false);
				_Camera.SetLineAspectLimit(3);
				_Camera.NumExtraErodes=1;
				_Camera.StartLineTimeout=0;
				_Camera.SetDisplayMode(RxCam.ENDisplayMode.Display_AllLines);
				_Camera.SetMinNumStartLines(3);
				_Camera.SetMeasureMode(RxCam.ENMeasureMode.MeasureMode_StartLines);
				if (SimuCallback!=null)
				{	
					CallBackDataStruct CallbackData = new CallBackDataStruct();
					SimuCallback.Invoke(RxCam.ENCamCallBackInfo.StartLinesDetected, CallbackData);
				}
			});
        }

		//--- FindLine_Horizontal --------------------------------------------------
		public void FindLine_Horizontal()
		{
			RxBindable.Invoke(()=>
			{
				_Camera.SetBinarizationMode(RxCam.ENBinarizeMode.BinarizeMode_Auto);
				//set very small LineAspectLimit for StartLines
				_Camera.SetLinesHorizontal(true);
				_Camera.SetLineAspectLimit(3);
				_Camera.NumExtraErodes=1;
				_Camera.StartLineTimeout=5;
				_Camera.SetDisplayMode(RxCam.ENDisplayMode.Display_AllLines);
				_Camera.SetMinNumStartLines(1);
				_Camera.SetMeasureMode(RxCam.ENMeasureMode.MeasureMode_StartLines);
				if (SimuCallback!=null)
				{	
					CallBackDataStruct CallbackData = new CallBackDataStruct();
					CallbackData.DPosX=0;
					CallbackData.DPosY=0;
					CallbackData.Value_1 = (float)1.2;
					CallbackData.LineAttach = LineAttachEnum.LineAttach_None;
					CallbackData.LineLayout = LineLayoutEnum.LineLayout_Covering;
					SimuCallback.Invoke(RxCam.ENCamCallBackInfo.StartLinesDetected,   CallbackData);
				}
			});
        }

		//--- FindLineEnd -----------------------------------------------
		public void FindLineEnd()
		{
			RxBindable.Invoke(()=>
			{
				_Camera.SetBinarizationMode(RxCam.ENBinarizeMode.BinarizeMode_Auto);
				//set very small LineAspectLimit for StartLines
				_Camera.SetLinesHorizontal(true);
				_Camera.SetLineAspectLimit(3);
				_Camera.NumExtraErodes=1;
				_Camera.StartLineTimeout=0;
				_Camera.SetDisplayMode(RxCam.ENDisplayMode.Display_AllLines);
				_Camera.SetMinNumStartLines(1);
				_Camera.SetMeasureMode(RxCam.ENMeasureMode.MeasureMode_StartLinesCont);
				if (SimuCallback!=null)
				{	
					CallBackDataStruct CallbackData = new CallBackDataStruct();
					CallbackData.DPosX=0;
					CallbackData.DPosY=0;
					CallbackData.Value_1 = (float)1.2;
					CallbackData.LineAttach = LineAttachEnum.LineAttach_None;
					CallbackData.LineLayout = LineLayoutEnum.LineLayout_FromRight;
					SimuCallback.Invoke(RxCam.ENCamCallBackInfo.StartLinesContinuous, CallbackData);
				}
			});
        }

		//--- MeasureAngle --------------------------------
		public RxCam.ENCamResult MeasureAngle(bool first)
		{
			Console.WriteLine("RxCamFubctions.MeasureAngle (first={0})", first);
			if (SimuCallback!=null)
			{	
				new Task(() =>
				{
					CallBackDataStruct CallbackData = new CallBackDataStruct();
					CallbackData.Value_1 = (float)1.2;
					Thread.Sleep(100);
					SimuCallback.Invoke(RxCam.ENCamCallBackInfo.AngleCorr, CallbackData);
				}).Start();
				return RxCam.ENCamResult.OK;
			}
			//			RxBindable.Invoke(()=>
			object result = RxBindable.InvokeFct(() =>
			{
				_Camera.SetMeasureMode(RxCam.ENMeasureMode.MeasureMode_Off);
				Thread.Sleep(100);
				_Camera.SetBinarizationMode(RxCam.ENBinarizeMode.BinarizeMode_Auto);
				_Camera.SetLinesHorizontal(false);
				_Camera.NumExtraErodes = 3;
				_Camera.SetLineAspectLimit(5);
				_Camera.SetDisplayMode(RxCam.ENDisplayMode.Display_Correction);
				ENCamResult res = RxCam.ENCamResult.Error;
				if (first) res = _Camera.DoMeasures(RxCam.ENMeasureMode.MeasureMode_Angle, 1, 0, 0);
				else res = _Camera.DoMeasures(RxCam.ENMeasureMode.MeasureMode_Angle, 10, 5, 17); // 5, 10, 15
				return res;
			});
			return (ENCamResult)result;
		}

		//--- MeasureStitch --------------------------------
		public RxCam.ENCamResult MeasureStitch()
		{
			if (SimuCallback!=null)
			{	
				new Task(() =>
				{
					CallBackDataStruct CallbackData = new CallBackDataStruct();
					CallbackData.Value_1 = (float)1.2;
					Thread.Sleep(100);
					SimuCallback.Invoke(RxCam.ENCamCallBackInfo.StitchCorr, CallbackData);
				}).Start();
				return RxCam.ENCamResult.OK;
			}

			object result = RxBindable.InvokeFct(() =>
			{
				_Camera.SetMeasureMode(RxCam.ENMeasureMode.MeasureMode_Off);
				Thread.Sleep(100);
				_Camera.SetBinarizationMode(RxCam.ENBinarizeMode.BinarizeMode_Auto);
				_Camera.SetLinesHorizontal(false);
				_Camera.NumExtraErodes=3;
				_Camera.SetLineAspectLimit(5);
				_Camera.SetDisplayMode(RxCam.ENDisplayMode.Display_Correction);
				//	_Camera.DoMeasures(RxCam.ENMeasureMode.MeasureMode_Stitch, 5, 10, 15);
				return _Camera.DoMeasures(RxCam.ENMeasureMode.MeasureMode_Stitch, 10, 5, 17);
			});
			return (ENCamResult)result;
		}

		//--- MeasureDist --------------------------------
		public RxCam.ENCamResult MeasureDist()
		{
			if (SimuCallback!=null)
			{	
				new Task(() =>
				{
					CallBackDataStruct CallbackData = new CallBackDataStruct();
					CallbackData.Value_1 = (float)1.2;
					Thread.Sleep(100);
					SimuCallback.Invoke(RxCam.ENCamCallBackInfo.RegisterCorr, CallbackData);
				}).Start();
				return RxCam.ENCamResult.OK;
			}

			object result = RxBindable.InvokeFct(() =>
			{
				_Camera.SetMeasureMode(RxCam.ENMeasureMode.MeasureMode_Off);
				Thread.Sleep(100);
				_Camera.SetBinarizationMode(RxCam.ENBinarizeMode.BinarizeMode_Auto);
				_Camera.SetLinesHorizontal(true);
				_Camera.NumExtraErodes=3;
				_Camera.RegisterMidOuterRatio=(float)1.5;
				_Camera.SetLineAspectLimit(5);
				_Camera.SetDisplayMode(RxCam.ENDisplayMode.Display_Correction);
				return _Camera.DoMeasures(RxCam.ENMeasureMode.MeasureMode_Register, 5, 10, 15);
			});
			return (ENCamResult) result;
		}
	}
}
