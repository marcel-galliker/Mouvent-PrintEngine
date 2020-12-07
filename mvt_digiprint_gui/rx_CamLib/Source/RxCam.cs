using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using System.Text;
using DirectShowLib;
using rx_CamLib.Models;
using RX_Common;
using static rx_CamLib.RxAlignFilter;
using System.Threading;

namespace rx_CamLib
{
    public class RxCam
    {
        #region Definitions

        public enum ENCamResult
		{
            OK                          =  0,
            Error                       = -1,
        
            Cam_notDetected             = -2,
            Cam_notFound                = -3,
            Cam_alreadyRunning          = -4,
            Cam_notSelected             = -5,
            Cam_notRunning,

            Filter_NotRegistered        = -6,

            Filter_AlreadyUsed          = -2147467259,

            Filter_DataTimeout          = -7,
            Filter_NoData               = -8,
            Filter_NoMeasurePossible    = -9
        };

        public enum ENCamCallBackInfo
        {
            RegisterCorr                =  6,
            StitchCorr                  =  5,
            AngleCorr                   =  4,
            StartLinesDetected          =  3,
            CameraStopped               =  2,
            CameraStarted               =  1,
            undefined                   =  0,
            USB_CameraDisconnected      = -1,
            DS_CaptureDeviceLost        = -2,
            DS_ErrorAbotedDisplay       = -3,
            VMR_ReconnectionFailed      = -4,
            CouldNotBuildGraph          = -5,
            GraphNotStoppedCorrectly    = -6,
            NoDataFromFilter            = -7
        }

        public enum ENMeasureMode
        {
            MeasureMode_Off         = 0,
            MeasureMode_AllLines    = 1,
            MeasureMode_StartLines  = 2,
            MeasureMode_Angle       = 3,
            MeasureMode_Stitch      = 4,
            MeasureMode_Register    = 5
        }

        public enum ENDisplayMode
        {
            Display_Off         = 0,
            Display_AllLines    = 1,
            Display_Correction  = 2
        }

        public enum ENBinarizeMode
        {
            BinarizeMode_Off            = 0,
            BinarizeMode_Manual         = 1,
            BinarizeMode_Auto           = 2,
            BinarizeMode_ColorAdaptive  = 3
        }

        public struct MeasureDataStruct
        {
            public float DPosX;         //Center of pattern offset X to center of camera
            public float DPosY;         //Center of pattern offset Y to center of camera
            public float Correction;    //Needed correction in Revolutions (Angle, Stitch) or μm (Register)
        };

        private DsDevice[] DeviceList = null;
        private bool CameraRunning = false;

        private string LastDsErrorMsg = "";
        private ENCamResult LastDsErrorNum = 0;

        private DsDevice Camera = null;
        private IFilterGraph2 FilterGraph2 = null;
        private IMediaControl MediaControl = null;
        private IMediaEventEx MediaEventEx = null;
        private ICaptureGraphBuilder2 CaptureGraph = null;
        private IBaseFilter SourceFilter = null;
        private IBaseFilter VMR9;
        private IVMRWindowlessControl9 WindowlessCtrl9 = null;
        private MessageWindow MsgWindow;
        private IntPtr pMsgWindow = IntPtr.Zero;
        private IFrx_AlignFilter halignFilter = null;

        //Callback from Filters
        private const int WM_APP = 0x8000;                  //Definitions from Windows.h
        private const int WM_APP_MEDIAEV = WM_APP + 2020;   //Callback from MediaEventEx
        private const int WM_APP_ALIGNEV = WM_APP + 2025;   //Callback from Bieler_ds_Align
        private const int WM_DEVICECHANGE = 0x0219;
        private const int DBT_DEVICEREMOVECOMPLETE = 0x8004;
        private const int DBT_DEVTYP_DEVICEINTERFACE = 0x0005;
        private const int WP_StartLines = 100;
        private const int WP_Angle = 101;
        private const int WP_Stitch = 102;
        private const int WP_Register = 103;

        //Property Display
        [DllImport("oleaut32.dll", CharSet = CharSet.Unicode, ExactSpelling = true)]
        static extern int OleCreatePropertyFrame
        (
            IntPtr hwndOwner,       //Handle to the parent window   
            int x,                  //Horizontal position of dialog box (not working)
            int y,                  //Vertical position of dialog box (not working)
            [MarshalAs(UnmanagedType.LPWStr)] string lpszCaption,   //caption of dialog box.
            int cObjects,           //Number of pointers in ppUnk
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 4, ArraySubType = UnmanagedType.IUnknown)] object[] lplpUnk,     //Array of property sheet pointers
            int cPages,             //Number of pages specified in pPageCIsID
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 6)] Guid[] lpPageClsID,  //Array of property page counters
            int lcid,               //Property Page identifier
            int dwReserved,         //NULL
            int lpvReserved         //NULL
        );

        #endregion

        public RxCam()
        {
            MsgWindow = new MessageWindow("MessageWindow");
            MsgWindow.CreateWindow();
            pMsgWindow = MsgWindow.Handle;
            UsbNotification.RegisterUsbDeviceNotification(pMsgWindow);
            MsgWindow.MsgCallBack += new MessageWindow.MessageCallBack(GetWinMessage);
        }

        ~RxCam()
        {
            if (CameraRunning) StopGraph();
            UsbNotification.UnregisterUsbDeviceNotification();
            MsgWindow.DestroyWindow();
        }


        #region Public Interface

        public delegate void CameraCallBack(ENCamCallBackInfo CallBackInfo, string ExtraInfo = "");
        public event CameraCallBack CamCallBack = null;

        public delegate void PositionMeasuredCallback(int angle, int stich, int encoder);

        /// <summary>
        /// Get version of library
        /// </summary>
        /// <returns>Version</returns>
        public string GetLibVersion()
        {
            return typeof(RxCam).Assembly.GetName().Version.ToString();
        }

        /// <summary>
        /// Searches for all connected cameras
        /// </summary>
        /// <returns>List containing all detected cameras</returns>
        public List<string> GetCameraList()
        {
            List<string> list = new List<string>();
            DeviceList = DsDevice.GetDevicesOfCat(FilterCategory.VideoInputDevice);
            for (int i = 0; i < DeviceList.Length; i++)
            {
                list.Add(DeviceList[i].Name);
            }
            return list;
        }

        /// <summary>
        /// Select the camera to use
        /// </summary>
        /// <param name="name">Camera name as found with GetCameraList()</param>
        /// <returns>
        /// <list type="table">
        /// </returns>
        public ENCamResult SelectCamera(string name)
        {
            if (DeviceList == null) DeviceList = DsDevice.GetDevicesOfCat(FilterCategory.VideoInputDevice);
            if (DeviceList == null || DeviceList.Length == 0)
            {
                LastDsErrorMsg = "No cameras detected";
                return LastDsErrorNum = ENCamResult.Cam_notDetected;
            }
            for (int i = 0; i < DeviceList.Length; i++)
            {
                if (DeviceList[i].Name.Equals(name))
                {
                    if (CameraRunning)
                    {
                        LastDsErrorMsg = "Camera already running";
                        return LastDsErrorNum = ENCamResult.Cam_alreadyRunning;
                    }

                    Camera = DeviceList[i];
                    return ENCamResult.OK;
                }
            }

            LastDsErrorMsg = "Selected Camera not found";
            return LastDsErrorNum = ENCamResult.Cam_notFound;
        }

        /// <summary>
        /// Starts camera caption, call SetVideoRectangle directly afterwards and if size changed
        /// </summary>
        /// <param name="hwnd">Window handle of displaying element</param>
        /// <returns>
        /// <list type="table">
        /// <item><description> 0: for success or error code:</description></item>
        /// </list>
        /// </returns>
        public ENCamResult StartCamera(IntPtr hwnd)
        {
            ENCamResult result;

            if (CameraRunning)
            {
                LastDsErrorMsg = "Camera already running";
                return LastDsErrorNum = ENCamResult.Cam_alreadyRunning;
            }
            if (Camera == null)
            {
                LastDsErrorMsg = "No camera selected";
                return LastDsErrorNum = ENCamResult.Cam_notSelected;
            }
            result=BuildGraph(hwnd);
            if (result!=ENCamResult.OK)
            {
                return result;
            }

            CameraRunning = true;
            CamCallBack?.Invoke(ENCamCallBackInfo.CameraStarted);
            return ENCamResult.OK;
        }

        /// <summary>
        /// Sets the display rectangle
        /// </summary>
        /// <param name="PicRectangle">ClientRectangle of displaying device</param>
        /// <returns>0: for success or error code</returns>
        public ENCamResult SetVideoRectangle(System.Drawing.Rectangle PicRectangle)
        {
            if (WindowlessCtrl9 == null) return ENCamResult.Error;
            int hResult = WindowlessCtrl9.SetVideoPosition(null, DsRect.FromRectangle(PicRectangle));
            if (hResultError(hResult)) return ENCamResult.Error;
            return 0;
        }

        /// <summary>
        /// Stops camera caption
        /// </summary>
        /// <returns>0: for success or error code</returns>
        public ENCamResult StopCamera()
        {
            if (CameraRunning) return StopGraph(); 
            return ENCamResult.OK;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="MeasureMode"></param>
        /// <returns>0: for success or error code</returns>
        public ENCamResult SetMeasureMode(ENMeasureMode MeasureMode)
        {
            if (!CameraRunning) return ENCamResult.Cam_notRunning;
            halignFilter.SetMeasureMode((UInt32)MeasureMode);
            return ENCamResult.OK;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="DisplayMode"></param>
        /// <returns>0: for success or error code</returns>
        public ENCamResult SetDisplayMode(ENDisplayMode DisplayMode)
        {
            if (!CameraRunning) return ENCamResult.Cam_notRunning;
            halignFilter.SetDisplayMode((UInt32)DisplayMode);
            return ENCamResult.OK;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="BinarizeMode"></param>
        /// <returns>0: for success or error code</returns>
        public ENCamResult SetBinarizationMode(ENBinarizeMode BinarizeMode)
        {
            if (!CameraRunning) return ENCamResult.Cam_notRunning;
            halignFilter.SetBinarizeMode((UInt32)BinarizeMode);
            return ENCamResult.OK;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="AspectLimit"></param>
        /// <returns>0: for success or error code</returns>
        public ENCamResult SetLineAspectLimit(UInt32 AspectLimit)
        {
            if (!CameraRunning) return ENCamResult.Cam_notRunning;
            halignFilter.SetBlobAspectLimit(AspectLimit);
            return ENCamResult.OK;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="NumMeasures"></param>
        /// <returns>0: for success or error code</returns>
        public ENCamResult DoMeasures(UInt32 NumMeasures)
        {
            if (!CameraRunning) return ENCamResult.Cam_notRunning;
            ENMeasureMode CurrentMode = (ENMeasureMode)halignFilter.GetMeasureMode();
            if (CurrentMode == ENMeasureMode.MeasureMode_Off || 
                CurrentMode == ENMeasureMode.MeasureMode_StartLines) return ENCamResult.Filter_NoMeasurePossible;
            if (halignFilter.DoMeasures(NumMeasures)) return ENCamResult.OK;
            else return ENCamResult.Filter_NoMeasurePossible;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="UpsideDown"></param>
        /// <returns>0: for success or error code</returns>
        public ENCamResult SetCameraUpsideDown(bool UpsideDown)
        {
            if (!CameraRunning) return ENCamResult.Cam_notRunning;
            halignFilter.SetUpsideDown(UpsideDown);
            return ENCamResult.OK;
        }

        /// <summary>
        /// Searches for horizontal lines, default = false (vertical)
        /// </summary>
        /// <param name="Horizontal"></param>
        /// <returns>0: for success or error code</returns>
        public ENCamResult SetLinesHorizontal(bool Horizontal)
        {
            if (!CameraRunning) return ENCamResult.Cam_notRunning;
            halignFilter.SetLinesHorizontal(Horizontal);
            return ENCamResult.OK;
        }

        #endregion


        #region Internal

        private void GetWinMessage(ref Message Msg)
        {
            Int64 WParam = Msg.WParam.ToInt64();
            Int64 LParam = Msg.LParam.ToInt64();

            switch (Msg.Msg)
            {
                //USB Device change
                case WM_DEVICECHANGE:
                {
                    switch (WParam)
                    {
                        case DBT_DEVICEREMOVECOMPLETE:
                        {
                            if (CameraRunning)
                            {
                                if (Marshal.ReadInt32(Msg.LParam, 4) == DBT_DEVTYP_DEVICEINTERFACE)
                                {
                                    bool CamStillHere = false;
                                    DsDevice[] NewDeviceList = DsDevice.GetDevicesOfCat(FilterCategory.VideoInputDevice);
                                    for (int i = 0; i < NewDeviceList.Length; i++)
                                    {
                                        if (NewDeviceList[i].Name == Camera.Name)
                                        {
                                            CamStillHere = true;
                                            break;
                                        }
                                    }
                                    if (!CamStillHere)
                                    {
                                        CamCallBack?.Invoke(ENCamCallBackInfo.USB_CameraDisconnected);
                                        StopCamera();
                                    }
                                }
                            }
                            break;
                        }
                    }
                    break;
                }

                //DirectShow Messages
                case WM_APP_MEDIAEV:
                {
                    EventCode EvCode;
                    IntPtr lParam1, lParam2;
                    int hr = 0;
                    if (CameraRunning)
                    {
                        hr = MediaEventEx.GetEvent(out EvCode, out lParam1, out lParam2, 0);
                        switch (EvCode)
                        {
                            case EventCode.DeviceLost:
                            {
                                if (CameraRunning)
                                {
                                    CamCallBack?.Invoke(ENCamCallBackInfo.DS_CaptureDeviceLost);
                                    StopCamera();
                                }
                                break;
                            }
                            case EventCode.ErrorAbort:
                            {
                                if (CameraRunning)
                                {
                                    CamCallBack?.Invoke(ENCamCallBackInfo.DS_ErrorAbotedDisplay);
                                    StopCamera();
                                }
                                break;
                            }
                            case EventCode.ErrorAbortEx:
                            {
                                if (CameraRunning)
                                {
                                    CamCallBack?.Invoke(ENCamCallBackInfo.DS_ErrorAbotedDisplay);
                                    StopCamera();
                                }
                                break;
                            }
                            case EventCode.StErrStopped:
                            {
                                if (CameraRunning)
                                {
                                    CamCallBack?.Invoke(ENCamCallBackInfo.DS_ErrorAbotedDisplay);
                                    StopCamera();
                                }
                                break;
                            }
                            case EventCode.VMRReconnectionFailed:
                            {
                                if (CameraRunning)
                                {
                                    CamCallBack?.Invoke(ENCamCallBackInfo.VMR_ReconnectionFailed);
                                    StopCamera();
                                }
                                break;
                            }
                            case EventCode.ClockUnset:
                            {
                                if (CameraRunning)
                                {
                                    CamCallBack?.Invoke(ENCamCallBackInfo.DS_CaptureDeviceLost);
                                    StopCamera();
                                }
                                break;
                            }
                        }
                    }
                    break;
                }

                //rx_AlignFilter Messages
                case WM_APP_ALIGNEV:
                {
                    MeasureDataStruct CorrectionData;
                    ENCamResult Result;

                    switch (WParam)
                    {
                        case WP_StartLines:
                            CamCallBack?.Invoke(ENCamCallBackInfo.StartLinesDetected, Msg.LParam.ToInt64().ToString());
                            break;
                        case WP_Angle:
                            Result = GetMeasureData(out CorrectionData);
                            if(Result == ENCamResult.OK)
                                CamCallBack?.Invoke(ENCamCallBackInfo.AngleCorr, CorrectionData.Correction.ToString() + ";" +
                                    CorrectionData.DPosX.ToString() + ";" + CorrectionData.DPosY.ToString());
                            else CamCallBack?.Invoke(ENCamCallBackInfo.NoDataFromFilter, Result.ToString());
                            break;
                        case WP_Stitch:
                            Result = GetMeasureData(out CorrectionData);
                            if (Result == ENCamResult.OK)
                                CamCallBack?.Invoke(ENCamCallBackInfo.StitchCorr, CorrectionData.Correction.ToString() + ";" +
                                    CorrectionData.DPosX.ToString() + ";" + CorrectionData.DPosY.ToString());
                            else CamCallBack?.Invoke(ENCamCallBackInfo.NoDataFromFilter, Result.ToString());
                            break;
                        case WP_Register:
                            Result = GetMeasureData(out CorrectionData);
                            if (Result == ENCamResult.OK)
                                CamCallBack?.Invoke(ENCamCallBackInfo.RegisterCorr, CorrectionData.Correction.ToString() + ";" +
                                    CorrectionData.DPosX.ToString() + ";" + CorrectionData.DPosY.ToString());
                            else CamCallBack?.Invoke(ENCamCallBackInfo.NoDataFromFilter, Result.ToString());
                            break;
                    }
                    break;
                }
            }
        }

        private ENCamResult BuildGraph(IntPtr hwnd)
        {
            try
            {
                int hResult;
                //Capture Pin for Properties
                IPin SrcCapPin = null;

                //Standard Filter Chain
                if (MediaControl != null)
                {
                    hResult = Marshal.ReleaseComObject(MediaControl);
                    if (hResultError(hResult)) return ENCamResult.Error;
                    MediaControl = null;
                }
                if (FilterGraph2 != null)
                {
                    FilterGraph2 = null;
                }
                if (CaptureGraph != null)
                {
                    hResult = Marshal.ReleaseComObject(CaptureGraph);
                    if (hResultError(hResult)) return ENCamResult.Error;
                    CaptureGraph = null;
                }

                FilterGraph2 = (IFilterGraph2)new FilterGraph();
                MediaControl = (IMediaControl)FilterGraph2 as IMediaControl;
                CaptureGraph = (ICaptureGraphBuilder2)new CaptureGraphBuilder2();

                hResult = CaptureGraph.SetFiltergraph(FilterGraph2);
                if (hResultError(hResult)) return ENCamResult.Error;

                if (!InsertVMR(hwnd)) return ENCamResult.Error;
                if (!InsertCamera(out SrcCapPin)) return ENCamResult.Error;
                if (InsertAlignFilter() != ENCamResult.OK) return ENCamResult.Error;

                if (!ConnectCamToAlign(SrcCapPin)) return ENCamResult.Error;
                if (!RenderAlignOut()) return ENCamResult.Error;

                //Set Media Event Callback
                MediaEventEx = (IMediaEventEx)FilterGraph2 as IMediaEventEx;
                MediaEventEx.SetNotifyWindow(pMsgWindow, WM_APP_MEDIAEV, IntPtr.Zero);

                if (!StartDisplay()) return ENCamResult.Error;

                hResult = Marshal.ReleaseComObject(SrcCapPin);
                if (hResultError(hResult)) return ENCamResult.Error;

                GetAllFilterSettings();
            }
            catch (Exception excep)
            {
                CamCallBack?.Invoke(ENCamCallBackInfo.CouldNotBuildGraph, excep.Message);
                return ENCamResult.Error;
            }

            return LastDsErrorNum = ENCamResult.OK;
        }

        private ENCamResult StopGraph()
        {
            int hResult;
//            IFrx_AlignFilter halignFilter = CamGlobals.AlignFilter.Handle;

            halignFilter.SetFrameTiming(false);
            halignFilter.SetDebug(false);
            halignFilter.SetHostPointer(IntPtr.Zero);
            halignFilter.ShowHistogram(false);
            halignFilter.SetOverlayTxt("", 0);
            halignFilter.SetShowOriginalImage(true);
            halignFilter.SetDisplayMode(0);
            halignFilter.SetMeasureMode(0);
            halignFilter.SetBinarizeMode(0);

            try
            {
                //Stop Media Control
                if (MediaControl != null)
                {
                    hResult = ChFilterState(ref SourceFilter, "Pause");
                    if (hResultError(hResult)) return ENCamResult.Error;
                    hResult = ChMediaState(ref MediaControl, "Stop");
                    if (hResultError(hResult)) return ENCamResult.Error;
                    MediaControl = null;
                }

                hResult = Marshal.ReleaseComObject(halignFilter);
                if (hResultError(hResult)) return ENCamResult.Error;
                halignFilter = null;

                //Remove all filters
                IEnumFilters pFilterEnum;
                hResult = FilterGraph2.EnumFilters(out pFilterEnum);
                if (hResultError(hResult)) return ENCamResult.Error;
                IBaseFilter[] filters = new IBaseFilter[1];
                IntPtr pfetched = Marshal.AllocCoTaskMem(4);

                while (pFilterEnum.Next(1, filters, pfetched) == 0)
                {
                    hResult = FilterGraph2.RemoveFilter(filters[0]);
                    if (hResultError(hResult)) return ENCamResult.Error;
                    hResult = pFilterEnum.Reset();
                    if (hResultError(hResult)) return ENCamResult.Error;
                    hResult = Marshal.ReleaseComObject(filters[0]);
                    if (hResultError(hResult)) return ENCamResult.Error;
                    filters[0] = null;
                }
                hResult = Marshal.ReleaseComObject(pFilterEnum);
                if (hResultError(hResult)) return ENCamResult.Error;
                Marshal.FreeCoTaskMem(pfetched);
                pfetched = IntPtr.Zero;
                hResult = FilterGraph2.Abort();
                if (hResultError(hResult)) return ENCamResult.Error;
                hResult = Marshal.ReleaseComObject(FilterGraph2);
                if (hResultError(hResult)) return ENCamResult.Error;
                FilterGraph2 = null;
            }
            catch (Exception excep)
            {
                CamCallBack?.Invoke(ENCamCallBackInfo.GraphNotStoppedCorrectly, excep.Message);
                return ENCamResult.Error;
            }

            CameraRunning = false;
            return ENCamResult.OK;
        }

        private bool InsertVMR(IntPtr hwnd)
        {
            int hResult;

            VMR9 = (IBaseFilter)new VideoMixingRenderer9();
            if (!ConfigVMR9(hwnd)) return false;
            hResult = FilterGraph2.AddFilter(VMR9, "VMR 9");
            if (hResultError(hResult)) return false;

            return true;
        }

        private bool ConfigVMR9(IntPtr hwnd)
        {
            //Setup Video Mixing Renderer 9

            int hResult;

            IVMRFilterConfig9 filterConfig = (IVMRFilterConfig9)VMR9;

            //Only 1 Input
            hResult = filterConfig.SetNumberOfStreams(1);
            if (hResultError(hResult)) return false;

            // Change VMR9 mode to Windowless
            hResult = filterConfig.SetRenderingMode(VMR9Mode.Windowless);
            if (hResultError(hResult)) return false;

            WindowlessCtrl9 = (IVMRWindowlessControl9)VMR9;

            // Set "Parent" window
            hResult = WindowlessCtrl9.SetVideoClippingWindow(hwnd);
            if (hResultError(hResult)) return false;

            // Set Aspect-Ratio
            hResult = WindowlessCtrl9.SetAspectRatioMode(VMR9AspectRatioMode.LetterBox);
            if (hResultError(hResult)) return false;

            return true;
        }

        private bool InsertCamera(out IPin SrcCapPin)
        {
            int hResult;
            //Capture Pin for Properties
            SrcCapPin = null;

            //Insert Camera
            object source;
            Guid SourceGuid = typeof(IBaseFilter).GUID;
            try
            {
                Camera.Mon.BindToObject(null, null, ref SourceGuid, out source);
                SourceFilter = (IBaseFilter)source;
                CamGlobals.CamDevice = new CamDevice(SourceFilter);
            }
            catch (Exception excep)
            {
                LastDsErrorMsg = excep.Message;
                LastDsErrorNum = ENCamResult.Error;
                return false;
            }
            hResult = FilterGraph2.AddFilter(SourceFilter, Camera.Name);
            if (hResultError(hResult)) return false;

            hResult = DsFindPin(SourceFilter, ref SrcCapPin, PinCategory.Capture);
            if (hResultError(hResult)) return false;

            return true;
        }

        private ENCamResult InsertAlignFilter()
        {
            int hResult;

            //Insert Align Filter
            Guid AlignGuid = new Guid("148BC1EB-2C83-418E-B9CD-E1F5BC9D1E38");  //rx_AlignFilter
            Type comType = null;
            comType = Type.GetTypeFromCLSID(AlignGuid);

            try
            {
                CamGlobals.AlignFilter = new RxAlignFilter(Activator.CreateInstance(comType));
            }
            catch (Exception e)
            {
                if ((uint)e.HResult == 0x80040154) return LastDsErrorNum = ENCamResult.Filter_NotRegistered;
                return LastDsErrorNum = ENCamResult.Error;
            }
            halignFilter = CamGlobals.AlignFilter.Handle;

            hResult = FilterGraph2.AddFilter(halignFilter, "Alignment");
            if (hResultError(hResult)) return ENCamResult.Error;

            //Set Callback
            if (pMsgWindow != IntPtr.Zero) halignFilter.SetHostPointer(pMsgWindow);

            return ENCamResult.OK;
        }

        private bool ConnectCamToAlign(IPin SrcCapPin)
        {
            int hResult;

            //Connect Camera to Align
            IFrx_AlignFilter halignFilter = CamGlobals.AlignFilter.Handle;
            IPin AlignInPin = null;
            hResult = DsFindPin(halignFilter, ref AlignInPin, "Input");
            if (hResultError(hResult)) return false;
            hResult = FilterGraph2.Connect(SrcCapPin, AlignInPin);
            if (hResultError(hResult)) return false;

            hResult = Marshal.ReleaseComObject(AlignInPin);
            if (hResultError(hResult)) return false;

            return true;
        }

        private bool RenderAlignOut()
        {
            int hResult;

            //Render Align Output Pin
            IFrx_AlignFilter halignFilter = CamGlobals.AlignFilter.Handle;
            IPin AlignOutPin = null;
            hResult = DsFindPin(halignFilter, ref AlignOutPin, "Output");
            if (hResultError(hResult)) return false;
            hResult = FilterGraph2.Render(AlignOutPin);
            if (hResultError(hResult)) return false;

            return true;
        }

        private bool StartDisplay()
        {
            int hResult;

            hResult = ChMediaState(ref MediaControl, "Run");
            if (hResultError(hResult)) return false;

            return true;
        }

        private int ChMediaState(ref IMediaControl MediaControl, string Action)
        {
            int hResult = 0;
            FilterState TargetState = FilterState.Stopped;
            FilterState pFState = FilterState.Running;
            hResult = MediaControl.GetState(100, out pFState);
            if (hResultError(hResult)) return hResult;

            switch (Action)
            {
                case "Pause":
                    TargetState = FilterState.Paused;
                    if (pFState != TargetState)
                        hResult = MediaControl.Pause();
                    break;
                case "Run":
                    TargetState = FilterState.Running;
                    if (pFState != TargetState)
                        hResult = MediaControl.Run();
                    break;
                case "Stop":
                    TargetState = FilterState.Stopped;
                    if (pFState != TargetState)
                        hResult = MediaControl.Stop();
                    break;
            }
            if (hResultError(hResult)) return hResult;

            while (pFState != TargetState)
            {
                Application.DoEvents();
                hResult = MediaControl.GetState(100, out pFState);
                if (hResultError(hResult)) return hResult;
            }

            return hResult;
        }

        private int ChFilterState(ref IBaseFilter Filter, string Action)
        {
            int hResult = 0;
            FilterState TargetState = FilterState.Stopped;
            FilterState pFState;
            hResult = Filter.GetState(100, out pFState);
            if (hResultError(hResult)) return hResult;

            switch (Action)
            {
                case "Pause":
                    TargetState = FilterState.Paused;
                    if (pFState != TargetState)
                        hResult = Filter.Pause();
                    break;
                case "Run":
                    TargetState = FilterState.Running;
                    if (pFState != TargetState)
                        hResult = Filter.Run(0);
                    break;
                case "Stop":
                    TargetState = FilterState.Stopped;
                    if (pFState != TargetState)
                        hResult = Filter.Stop();
                    break;
            }
            if (hResultError(hResult)) return hResult;

            while (pFState != TargetState)
            {
                Application.DoEvents();
                hResult = Filter.GetState(100, out pFState);
                if (hResultError(hResult)) return hResult;
            }

            return hResult;
        }

        private int DsFindPin(IBaseFilter Filter, ref IPin Pin, Guid Category)
        {

            //Find specified Pin of a Filter

            int hResult = 0;
            if (FilterGraph2 == null)
                hResult = CaptureGraph.FindPin(Filter, PinDirection.Output, PinCategory.Capture, MediaType.Video, true, 0, out Pin);
            else
                hResult = CaptureGraph.FindPin(Filter, PinDirection.Output, PinCategory.Capture, MediaType.Video, false, 0, out Pin);
            return hResult;  //Pin found or not
        }

        private int DsFindPin(IBaseFilter Filter, ref IPin Pin, string PinName)
        {
            //Find specified Pin of a Filter

            int hResult = 0;

            //All Pins of Filter
            IEnumPins ppEnum;
            hResult = Filter.EnumPins(out ppEnum);
            if (hResult != 0) return hResult;

            IPin[] pins = new IPin[1];
            IntPtr pcFetched = Marshal.AllocCoTaskMem(4);

            //Find specified Pin
            while (ppEnum.Next(1, pins, pcFetched) == 0)
            {
                PinInfo pinfo;
                pins[0].QueryPinInfo(out pinfo);
                if (pinfo.name == PinName)
                {
                    Pin = pins[0];      //Set Pin
                    DsUtils.FreePinInfo(pinfo);
                    Marshal.FreeCoTaskMem(pcFetched);
                    pcFetched = IntPtr.Zero;
                    pins = null;
                    ppEnum = null;
                    return 0;   //return success
                }
                DsUtils.FreePinInfo(pinfo);
            }
            Marshal.ReleaseComObject(pins);
            pins = null;
            Marshal.FreeCoTaskMem(pcFetched);
            pcFetched = IntPtr.Zero;
            Marshal.ReleaseComObject(ppEnum);
            ppEnum = null;

            return -1;  //Pin not found
        }

        private bool hResultError(int hResult)
        {
            LastDsErrorMsg = DsError.GetErrorText(hResult);
            LastDsErrorNum = (ENCamResult)hResult;

            if (hResult < 0)
            {
                //Error
                return true;
            }
            else if (hResult > 1)
            {
                //Message
                return false;
            }
            return false;
        }

        private void GetAllFilterSettings()
        {
            //and many more to come:
            //GetCppVersion();
            //SetHostPointer(IntPtr HostHwnd);
            //TakeSnapShot(string SnapDirectory, string SnapFileName);

            //SetNumDilateErodes(UInt32 DilateErodes);
            //UInt32 GetNumDilateErodes();
            //void SetNumExtraErodes(UInt32 ExtraErodes);
            //UInt32 GetNumExtraErodes();
            //void SetErodeSeedX(UInt32 ErodeSeedX);
            //void SetErodeSeedY(UInt32 ErodeSeedY);
            //UInt32 GetErodeSeedX();
            //UInt32 GetErodeSeedY();

            //void SetCrossColor(UInt32 CrossColor);
            //UInt32 GetCrossColor();
            //void SetBlobOutlineColor(UInt32 BlobOutlineColor);
            //UInt32 GetBlobOutlineColor();
            //void SetBlobCrossColor(UInt32 BlobCrossColor);
            //UInt32 GetBlobCrossColor();
            //void SetBlobAspectLimit(UInt32 BlobAspectLimit);
            //UInt32 GetBlobAspectLimit();
            //void SetBlobAreaDivisor(UInt32 BlobAreaDivisor);
            //UInt32 GetBlobAreaDivisor();
            //void SetShowBlobOutlines(bool ShowBlobOutlines);
            //bool GetShowBlobOutlines();
            //void SetShowBlobCenters(bool ShowBlobCenters);
            //bool GetShowBlobCenters();
            //void SetShowBlobValues(bool ShowBlobValues);
            //bool GetShowBlobValues();
            //void SetBlobFont(IntPtr pLogFontStruct);
            //void GetBlobFont(IntPtr pLogFontStruct, out UInt32 LogFontSize);

            //void SetMeasurePixels(bool MeasurePixels);
            //bool GetMeasurePixels();
            //void SetInverse(bool Inverse);
            //bool GetInverse();
            //void SetMeasureMode(UInt32 MeasureMode);
            //UInt32 GetMeasureMode();
            //void SetDisplayMode(UInt32 DisplayMode);
            //UInt32 GetDisplayMode();

            //void SetLinesHorizontal(bool Horizontal);
            //bool GetLinesHorizontal();

        }

        private ENCamResult GetMeasureData(out MeasureDataStruct MeasureData)
        {
            MeasureData = new MeasureDataStruct();

            //Raw data list
            List<MeasureDataStruct> CorrectionList;
            ENCamResult DataResult = GetMeasureDataList(out CorrectionList);
            if (DataResult != ENCamResult.OK) return DataResult;

            //Average within 1 sigma
            AverageData(CorrectionList, ref MeasureData);

            return ENCamResult.OK;
        }

        private ENCamResult GetMeasureDataList(out List<MeasureDataStruct> MeasureDataList)
        {
            MeasureDataList = new List<MeasureDataStruct>();
            UInt32 DataListSize32 = 0;
            System.Timers.Timer tmr_Timeout = new System.Timers.Timer();
            tmr_Timeout.Elapsed += new System.Timers.ElapsedEventHandler(TimerElapsed);
            tmr_Timeout.Interval = 500;
            tmr_Timeout.Enabled = true;

            //Wait for List to be ready
            while (DataListSize32 == 0)
            {
                //Get Size
                halignFilter.GetMeasureResults(IntPtr.Zero, out DataListSize32);
                if (DataListSize32 != 0) break;
                Thread.Sleep(30);   //Wait app. 1 frame
                Application.DoEvents();
                if (tmr_Timeout.Enabled == false)
                {
                    tmr_Timeout.Elapsed -= TimerElapsed;
                    return ENCamResult.Filter_DataTimeout;
                }
            }
            tmr_Timeout.Enabled = false;
            tmr_Timeout.Elapsed -= TimerElapsed;

            //Allocate memory
            IntPtr unmanaged_pInList = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(MeasureDataStruct)) * (int)DataListSize32);
            //Get Data from Filter
            int DataListSize = (int)DataListSize32;
            if (!halignFilter.GetMeasureResults(unmanaged_pInList, out DataListSize32)) return ENCamResult.Filter_NoData;
            if (DataListSize32 != DataListSize || DataListSize32 == 0)
            {
                Marshal.FreeHGlobal(unmanaged_pInList);
                return ENCamResult.Filter_NoData;
            }

            //Fill Data to InList
            IntPtr current = unmanaged_pInList;
            for (int i = 0; i < DataListSize32; i++)
            {
                MeasureDataList.Add((MeasureDataStruct)Marshal.PtrToStructure(current, typeof(MeasureDataStruct)));
                Marshal.DestroyStructure(current, typeof(MeasureDataStruct));
                current = (IntPtr)((long)current + Marshal.SizeOf(typeof(MeasureDataStruct)));
            }
            Marshal.FreeHGlobal(unmanaged_pInList);

            return ENCamResult.OK;
        }

        private void AverageData(List<MeasureDataStruct> CorrectionList, ref MeasureDataStruct MeasureData)
        {
            //Average Data within 1 sigma

            //Mean
            float MeanCorr = 0;
            float MeanDX = 0;
            float MeanDY = 0;
            for (int i = 0; i < CorrectionList.Count; i++)
            {
                MeanCorr += CorrectionList[i].Correction;
                MeanDX += CorrectionList[i].DPosX;
                MeanDY += CorrectionList[i].DPosY;
            }
            MeanCorr /= CorrectionList.Count;
            MeanDX /= CorrectionList.Count;
            MeanDY /= CorrectionList.Count;

            //Variance
            double VarCorr = 0;
            double VarDX = 0;
            double VarDY = 0;
            for (int i = 0; i < CorrectionList.Count; i++)
            {
                VarCorr += Math.Pow(CorrectionList[i].Correction - MeanCorr, 2);
                VarDX += Math.Pow(CorrectionList[i].DPosX - MeanDX, 2);
                VarDY += Math.Pow(CorrectionList[i].DPosY - MeanDY, 2);
            }
            VarCorr /= CorrectionList.Count;
            VarDX /= CorrectionList.Count;
            VarDY /= CorrectionList.Count;

            //StdDev
            VarCorr = Math.Sqrt(VarCorr);
            VarDX = Math.Sqrt(VarDX);
            VarDY = Math.Sqrt(VarDY);

            //Average within 1 sigma
            int Counter = 0;
            for (int i = 0; i < CorrectionList.Count; i++)
            {
                if (CorrectionList[i].Correction <= MeanCorr + VarCorr &&
                   CorrectionList[i].Correction >= MeanCorr - VarCorr &&
                   CorrectionList[i].DPosX <= MeanDX + VarDX &&
                   CorrectionList[i].DPosX >= MeanDX - VarDX &&
                   CorrectionList[i].DPosY <= MeanDY + VarDY &&
                   CorrectionList[i].DPosY >= MeanDY - VarDY)
                {
                    Counter++;
                    MeasureData.Correction += CorrectionList[i].Correction;
                    MeasureData.DPosX += CorrectionList[i].DPosX;
                    MeasureData.DPosY += CorrectionList[i].DPosY;
                }
            }
            MeasureData.Correction /= Counter;
            MeasureData.DPosX /= Counter;
            MeasureData.DPosY /= Counter;
        }

        private void TimerElapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            System.Timers.Timer tmr = (System.Timers.Timer)sender;
            tmr.Enabled = false;
        }


        //--- GetCamStreamCapsList ------------------------------------
        public List<StreamCaps> GetCamStreamCapsList()
        {
            int hRes = 0;
            IPin SrcCapPin = null;
            List<StreamCaps> list = new List<StreamCaps>();

            if(SourceFilter == null) return null;

            hRes = DsFindPin(SourceFilter, ref SrcCapPin, PinCategory.Capture);
            if (hResultError(hRes)) return null;

            IAMStreamConfig videoConfig = SrcCapPin as IAMStreamConfig;
            int capsCount, capSize;
            hRes = videoConfig.GetNumberOfCapabilities(out capsCount, out capSize);

            VideoInfoHeader VIHeader = new VideoInfoHeader();
            IntPtr pSC = Marshal.AllocHGlobal(capSize);

            AMMediaType mt = null;

            List<Guid> GuidList = new List<Guid>();
            List<int> FrameRates = new List<int>();
            for (int i = 0; i < capsCount; ++i)
            {
                StreamCaps Resolution = new StreamCaps();
                hRes = videoConfig.GetStreamCaps(i, out mt, pSC);
                if (hResultError(hRes)) continue;
                Resolution.Mediasubtype = mt.subType;
                if (mt.formatType.ToString() == "05589f80-c356-11ce-bf01-00aa0055595a") //only VideoInfoHeader(1)
                {
                    Marshal.PtrToStructure(mt.formatPtr, VIHeader);
                    Resolution.FrameRate = (int)((float)10000000 / VIHeader.AvgTimePerFrame);
                    Resolution.Resolution = new System.Drawing.Point(VIHeader.BmiHeader.Width, VIHeader.BmiHeader.Height);
                    list.Add(Resolution);
                }
            }

            Marshal.FreeCoTaskMem(mt.formatPtr);
            Marshal.FreeHGlobal(pSC);

            return list;
        }

        public StreamCaps GetCamStreamCaps()
        {
            int hRes = 0;
            IPin SrcCapPin = null;
            StreamCaps StreamCaps = new StreamCaps();

            if (SourceFilter == null) return null;

            hRes = DsFindPin(SourceFilter, ref SrcCapPin, PinCategory.Capture);
            if (hResultError(hRes)) return null;
            IAMStreamConfig StreamConfig1 = (IAMStreamConfig)SrcCapPin;
            VideoInfoHeader VIHeader = new VideoInfoHeader();
            AMMediaType mt = null;
            hRes = StreamConfig1.GetFormat(out mt);
            if (hResultError(hRes)) return null;
            Marshal.PtrToStructure(mt.formatPtr, VIHeader);
            StreamCaps.Mediasubtype = mt.subType;
            StreamCaps.FrameRate = (int)((float)10000000 / VIHeader.AvgTimePerFrame);
            StreamCaps.Resolution = new System.Drawing.Point(VIHeader.BmiHeader.Width, VIHeader.BmiHeader.Height);

            Marshal.FreeCoTaskMem(mt.formatPtr);

            return StreamCaps;
        }

        private ENCamResult SetCamStreamCaps(StreamCaps StreamCaps)
        {
            int hRes = 0;

            if (SourceFilter == null) return ENCamResult.Error;

            IPin SrcCapPin = null;
            hRes = DsFindPin(SourceFilter, ref SrcCapPin, PinCategory.Capture);
            if (hResultError(hRes)) return LastDsErrorNum = ENCamResult.Error;
            IAMStreamConfig StreamConfig1 = (IAMStreamConfig)SrcCapPin;
            VideoInfoHeader VIHeader = new VideoInfoHeader();
            AMMediaType mt = null;
            hRes = StreamConfig1.GetFormat(out mt);
            if (hResultError(hRes)) return LastDsErrorNum = ENCamResult.Error;

            if (StreamCaps!=null)
			{
                Marshal.PtrToStructure(mt.formatPtr, VIHeader);
                VIHeader.BmiHeader.Width = Convert.ToInt32(StreamCaps.Resolution.X);
                VIHeader.BmiHeader.Height = Convert.ToInt32(StreamCaps.Resolution.Y);
                Marshal.StructureToPtr(VIHeader, mt.formatPtr, true);
                mt.subType = StreamCaps.Mediasubtype;
                hRes = StreamConfig1.SetFormat(mt);
                if (hResultError(hRes)) return LastDsErrorNum = ENCamResult.Error;
                Marshal.FreeCoTaskMem(mt.formatPtr);
			}

            return ENCamResult.OK;
        }

        /*
        private int SetCamCapSetting(CamDeviceSettings CamCap)
        {
            IAMCameraControl CamCtrl = SourceFilter as IAMCameraControl;
            CameraControlProperty Property = (CameraControlProperty)Enum.Parse(typeof(CameraControlProperty), CamCap.Name, true);
            if (Enum.IsDefined(typeof(CameraControlProperty), Property))
            {
                CameraControlFlags Flag = (CameraControlFlags)CamCap.Flag;
                if (Enum.IsDefined(typeof(CameraControlFlags), Flag))
                {
                    int hRes = CamCtrl.Set(Property, CamCap.Value, Flag);
                    if (hResultError(hRes)) return hRes;
                    return hRes;
                }
            }
            return -1;
        }
        */

        #endregion
    }
}
