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
		error
	};

	public enum ECamFunction
	{
		CamNoFunction,
		CamConfirmFocus,	
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

		//--- FindMark --------------------------------------------------
		public void FindMark(bool horizontal, int timeout=0)
		{
			RxBindable.Invoke(()=>
			{
				_Camera.SetBinarizationMode(RxCam.ENBinarizeMode.BinarizeMode_Auto);
				//set very small LineAspectLimit for StartLines
				_Camera.SetLinesHorizontal(horizontal);
				_Camera.SetLineAspectLimit(3);
				_Camera.NumExtraErodes=1;
				_Camera.StartLineTimeout=(uint)timeout;
				_Camera.SetDisplayMode(RxCam.ENDisplayMode.Display_AllLines);
				if (horizontal) 
				{
					_Camera.SetMinNumStartLines(1);
					if (timeout>0) _Camera.SetMeasureMode(RxCam.ENMeasureMode.MeasureMode_StartLines);
					else	       _Camera.SetMeasureMode(RxCam.ENMeasureMode.MeasureMode_StartLinesCont);
					if (SimuCallback!=null)
					{	
						CallBackDataStruct CallbackData = new CallBackDataStruct();
						CallbackData.DPosX=0;
						CallbackData.DPosY=0;
						CallbackData.Value_1 = (float)1.2;
						CallbackData.LineAttach = LineAttachEnum.LineAttach_None;
						if (timeout>0) 
						{
							CallbackData.LineLayout = LineLayoutEnum.LineLayout_Covering;
							SimuCallback.Invoke(RxCam.ENCamCallBackInfo.StartLinesDetected,   CallbackData);
						}
						else		   
						{
							CallbackData.LineLayout = LineLayoutEnum.LineLayout_FromRight;
							SimuCallback.Invoke(RxCam.ENCamCallBackInfo.StartLinesContinuous, CallbackData);
						}
					}
				}
				else            
				{
					_Camera.SetMinNumStartLines(3);
					_Camera.SetMeasureMode(RxCam.ENMeasureMode.MeasureMode_StartLines);
					if (SimuCallback!=null)
					{	
						CallBackDataStruct CallbackData = new CallBackDataStruct();
						SimuCallback.Invoke(RxCam.ENCamCallBackInfo.StartLinesDetected, CallbackData);
					}
				}
			});
        }

		//--- MeasureAngle --------------------------------
		public RxCam.ENCamResult MeasureAngle()
		{
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
			RxBindable.Invoke(()=>
			{
				_Camera.SetBinarizationMode(RxCam.ENBinarizeMode.BinarizeMode_Auto);
				_Camera.SetLinesHorizontal(false);
				_Camera.NumExtraErodes=3;
				_Camera.SetLineAspectLimit(5);
				_Camera.SetDisplayMode(RxCam.ENDisplayMode.Display_Correction);
				_Camera.DoMeasures(RxCam.ENMeasureMode.MeasureMode_Angle, 5, 10, 15);
			});
			return RxCam.ENCamResult.OK;
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

			RxBindable.Invoke(()=>
			{
				_Camera.SetBinarizationMode(RxCam.ENBinarizeMode.BinarizeMode_Auto);
				_Camera.SetLinesHorizontal(false);
				_Camera.NumExtraErodes=3;
				_Camera.SetLineAspectLimit(5);
				_Camera.SetDisplayMode(RxCam.ENDisplayMode.Display_Correction);
				_Camera.DoMeasures(RxCam.ENMeasureMode.MeasureMode_Stitch, 5, 10, 15);
			});
			return RxCam.ENCamResult.OK;
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

			RxBindable.Invoke(()=>
			{
				_Camera.SetBinarizationMode(RxCam.ENBinarizeMode.BinarizeMode_Auto);
				_Camera.SetLinesHorizontal(true);
				_Camera.NumExtraErodes=3;
				_Camera.RegisterMidOuterRatio=(float)1.5;
				_Camera.SetLineAspectLimit(5);
				_Camera.SetDisplayMode(RxCam.ENDisplayMode.Display_Correction);
				_Camera.DoMeasures(RxCam.ENMeasureMode.MeasureMode_Register, 5, 10, 15);
			});
			return RxCam.ENCamResult.OK;
		}
	}
}
