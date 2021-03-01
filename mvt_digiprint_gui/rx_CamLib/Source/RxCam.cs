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
            ColorStitchCorr             = 10,
            OcrResult                   =  9,
            DebugCallBack               =  8,
            StartLinesContinuous        =  7,
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
            NoDataFromFilter            = -7,
            MeasureTimeout              = -8,
            StartLinesTimeout           = -9
        }

        public enum ENMeasureMode
        {
            MeasureMode_Off         = 0,
            MeasureMode_AllLines    = 1,
            MeasureMode_StartLines  = 2,
            MeasureMode_Angle       = 3,
            MeasureMode_Stitch      = 4,
            MeasureMode_Register    = 5,
            MeasureMode_StartLinesCont = 6,
            MeasureMode_ColorStitch = 7,
            MeasureMode_OCR = 8,
        }

        public enum ENDisplayMode
        {
            Display_Off         = 0,
            Display_AllLines    = 1,
            Display_Correction  = 2,
            Display_OCR         = 3,
        }

        public enum ENBinarizeMode
        {
            BinarizeMode_Off            = 0,
            BinarizeMode_Manual         = 1,
            BinarizeMode_Auto           = 2,
            BinarizeMode_ColorAdaptive  = 3,    
            BinarizeMode_RGB            = 4
        }

        public enum LineLayoutEnum
        {
            LineLayout_Undefined = 0,
            LineLayout_Covering = 1,
            LineLayout_FromTop = 2,
            LineLayout_FromBot = 3,
            LineLayout_FromLeft = 4,
            LineLayout_FromRight = 5
        };

        public enum LineAttachEnum
        {
            LineAttach_Undefined = 0,
            LineAttach_None = 1,
            LineAttach_Top = 2,
            LineAttach_Bot = 3,
            LineAttach_Left = 4,
            LineAttach_Right = 5
        };

        public struct CallBackDataStruct
        {
            public ENCamResult CamResult;       //Error or other details
            public float DPosX;                 //Center of pattern offset X to center of camera, Angle, Stitch, Register: μm, StartLines: px
            public float DPosY;                 //Center of pattern offset Y to center of camera, Angle, Stitch, Register: μm, StartLines: px
            public float Value_1;               //Angle, Stitch, Register: Correction Value in Rev or μm (Register, ColorStitch), StartLines: number of lines
            public LineLayoutEnum LineLayout;   //Angle, Stitch, Register: 0, StartLines: Lines layout (Top/Right/Covering/Bottom/Left)  
            public LineAttachEnum LineAttach;   //Angle, Stitch, Register: 0, StartLines: Line attached to camera limits (Top/Right/None/Bottom/Left)
            public bool micron;                 //Measure is in μm
            public int NumMeasures;             //Number of successful measures
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
            public char[] OcrChars;             //OCR results as ASCII chars
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string Info;                 // Debug Info etc.
        };

        public enum OcrLearnCmdEnum
        {
            ReadImage = 0,
            Next = 1,
            Edit = 2,
            SaveAs = 3,
            Quit = 4,
            DeleteLearnedOCR = 5,
            ReloadOCR = 6,
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
        private StreamCaps CurrentCaps = null;

        //Callback from Filters
        private const int WM_APP = 0x8000;                  //Definitions from Windows.h
        private const int WM_APP_MEDIAEV = WM_APP + 2020;   //Callback from MediaEventEx
        private const int WM_APP_ALIGNEV = WM_APP + 2025;   //Callback from Bieler_ds_Align
        private const int WM_DEVICECHANGE = 0x0219;         //from winuser.h
        private const int WM_COPYDATA = 0x004A;             //from winuser.h
        private const int DBT_DEVICEREMOVECOMPLETE = 0x8004;    //from DBT.H
        private const int DBT_DEVTYP_DEVICEINTERFACE = 0x0005;
        private const int WP_StartLines = 100;
        private const int WP_Angle = 101;
        private const int WP_Stitch = 102;
        private const int WP_Register = 103;
        private const int WP_StartLinesCont = 104;
        private const int WP_MeasureTimeout = 105;
        private const int WP_StartLinesTimeout = 106;
        private const int WP_CallBackDebug = 107;
        private const int WP_ReadOCR = 108;
        private const int WP_ColorStitch = 109;

        private const int LP_None = 0;
        private const int LP_TO1st = 100;

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

        public delegate void CameraCallBack(ENCamCallBackInfo CallBackInfo, CallBackDataStruct CallBackData);
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
            result = BuildGraph(hwnd);
            if (result != ENCamResult.OK)
            {
                return result;
            }

            CameraRunning = true;
            CallBackDataStruct CallbackData = new CallBackDataStruct();
            CamCallBack?.Invoke(ENCamCallBackInfo.CameraStarted, CallbackData);
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
            if (!CameraRunning) return ENCamResult.Cam_notRunning;

            CallBackDataStruct CallbackData = new CallBackDataStruct();
            CallbackData.CamResult = StopGraph();
            if(CallbackData.CamResult != ENCamResult.OK)
                CamCallBack?.Invoke(ENCamCallBackInfo.GraphNotStoppedCorrectly, CallbackData);
            else
                CamCallBack?.Invoke(ENCamCallBackInfo.CameraStopped, CallbackData);
            return CallbackData.CamResult;
        }

        /// <summary>
        /// Returns a list with all possible stream settings for the selected camera
        /// </summary>
        /// <returns>null if not successful</returns>
        public List<StreamCaps> GetCamStreamCapsList()
        {
            int hResult = 0;
            IPin SrcCapPin = null;
            List<StreamCaps> list = new List<StreamCaps>();

            if (Camera == null) return null;

            bool DummyChain = false;
            if (SourceFilter == null)
            {
                //Build DummyChain
                if (!CreateDummyChain(ref SrcCapPin)) return null;
                DummyChain = true;
            }
            else
            {
                hResult = DsFindPin(SourceFilter, ref SrcCapPin, PinCategory.Capture);
                if (hResultError(hResult)) return null;
            }

            IAMStreamConfig videoConfig = SrcCapPin as IAMStreamConfig;
            int capsCount, capSize;
            hResult = videoConfig.GetNumberOfCapabilities(out capsCount, out capSize);

            VideoInfoHeader VIHeader = new VideoInfoHeader();
            IntPtr pSC = Marshal.AllocHGlobal(capSize);

            AMMediaType mt = null;

            List<Guid> GuidList = new List<Guid>();
            List<int> FrameRates = new List<int>();
            for (int i = 0; i < capsCount; ++i)
            {
                StreamCaps Resolution = new StreamCaps();
                hResult = videoConfig.GetStreamCaps(i, out mt, pSC);
                if (hResultError(hResult)) continue;
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

            if (DummyChain) { if (!RemoveDummyChain()) return null; }

            return list;
        }

        /// <summary>
        /// Returns the currently active stream settings
        /// </summary>
        /// <returns>null if not successful</returns>
        public StreamCaps GetCamStreamCaps()
        {
            int hResult = 0;
            IPin SrcCapPin = null;
            StreamCaps StreamCaps = new StreamCaps();

            if (Camera == null) return null;

            bool DummyChain = false;
            if (SourceFilter == null)
            {
                //Build DummyChain
                if (!CreateDummyChain(ref SrcCapPin)) return null;
                DummyChain = true;
            }
            else
            {
                hResult = DsFindPin(SourceFilter, ref SrcCapPin, PinCategory.Capture);
                if (hResultError(hResult)) return null;
            }

            hResult = DsFindPin(SourceFilter, ref SrcCapPin, PinCategory.Capture);
            if (hResultError(hResult)) return null;

            IAMStreamConfig StreamConfig1 = (IAMStreamConfig)SrcCapPin;
            VideoInfoHeader VIHeader = new VideoInfoHeader();
            AMMediaType mt = null;
            hResult = StreamConfig1.GetFormat(out mt);
            if (hResultError(hResult)) return null;
            Marshal.PtrToStructure(mt.formatPtr, VIHeader);
            StreamCaps.Mediasubtype = mt.subType;
            StreamCaps.FrameRate = (int)((float)10000000 / VIHeader.AvgTimePerFrame);
            StreamCaps.Resolution = new System.Drawing.Point(VIHeader.BmiHeader.Width, VIHeader.BmiHeader.Height);

            Marshal.FreeCoTaskMem(mt.formatPtr);

            if (DummyChain) { if (!RemoveDummyChain()) return null; }

            return StreamCaps;
        }

        /// <summary>
        /// Sets stream settings for next start of selected camera
        /// </summary>
        /// <param name="StreamCaps"></param>
        /// <returns>0: for success or error code</returns>
        public ENCamResult SetCamStreamCaps(StreamCaps StreamCaps)
        {
            if (CameraRunning) return ENCamResult.Cam_alreadyRunning;
            CurrentCaps = StreamCaps;
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
        public ENMeasureMode GetMeasureMode()
        {
            if (!CameraRunning) return ENMeasureMode.MeasureMode_Off;
            return (ENMeasureMode)halignFilter.GetMeasureMode();
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
        public ENDisplayMode GetDisplayMode()
        {
            if (!CameraRunning) return ENDisplayMode.Display_Off;
            return (ENDisplayMode)halignFilter.GetDisplayMode();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="ShowHistogram"></param>
        /// <returns>0: for success or error code</returns>
        public ENCamResult ShowHistogram(bool ShowHistogram)
        {
            if (!CameraRunning) return ENCamResult.Cam_notRunning;
            halignFilter.ShowHistogram(ShowHistogram);
            return ENCamResult.OK;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="ShowProcessImage"></param>
        /// <returns>0: for success or error code</returns>
        public ENCamResult ShowProcessImage(bool ShowProcessImage)
        {
            if (!CameraRunning) return ENCamResult.Cam_notRunning;
            halignFilter.SetShowOriginalImage(!ShowProcessImage);
            return ENCamResult.OK;
        }
        public bool GetShowProcessImage()
        {
            if (!CameraRunning) return false;
            return !halignFilter.GetShowOriginalImage();
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
        public ENBinarizeMode GetBinarizeMode()
        {
            if (!CameraRunning) return ENBinarizeMode.BinarizeMode_Off;
            return (ENBinarizeMode)halignFilter.GetBinarizeMode();
        }

        /// <summary>
        /// returns the currently used threshold
        /// </summary>
        /// <returns>current threshold or 0 if not available</returns>
        public UInt32 GetThreshold()
        {
            if (!CameraRunning) return 0;
            return halignFilter.GetThreshold();
        }

        /// <summary>
        /// Manual binarisation threshod (0 - 255)
        /// </summary>
        /// <param name="Threshold"></param>
        /// <returns>0: for success or error code</returns>
        public ENCamResult SetThreshold(UInt32 Threshold)
        {
            if (!CameraRunning) return ENCamResult.Cam_notRunning;
            halignFilter.SetThreshold(Threshold);
            return ENCamResult.OK;
        }

        /// <summary>
        /// sets the minimal lentgh/width ration for a blob to be accepted as a line
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
        /// Executes the desired number of measurements, returns result and center position of pattern through CameraCallBack 
        /// </summary>
        /// <param name="NumMeasures">number of measurements to be taken</param>
        /// <param name="TO_1st">number of camera frames until 1st valid measurement is found, 0 is no tiemeout</param>
        /// <param name="TO_End">number of camera frames until all measurements are done, o is no timeout</param>
        /// <returns>0: for success or error code</returns>
        public ENCamResult DoMeasures(ENMeasureMode MeasureMode, UInt32 NumMeasures, UInt32 TO_1st, UInt32 TO_End)
        {
            if (!CameraRunning) return ENCamResult.Cam_notRunning;

            halignFilter.SetMeasureMode((UInt32)MeasureMode);
            
            if (MeasureMode == ENMeasureMode.MeasureMode_Off ||
                MeasureMode == ENMeasureMode.MeasureMode_StartLines ||
                MeasureMode == ENMeasureMode.MeasureMode_StartLinesCont) return ENCamResult.Filter_NoMeasurePossible;

            if (halignFilter.DoMeasures(NumMeasures, TO_1st, TO_End)) return ENCamResult.OK;
            else return ENCamResult.Filter_NoMeasurePossible;
        }

        /// <summary>
        /// Sets measurement to "Upside-Down", default = false (top of camera is top of image)
        /// </summary>
        /// <param name="UpsideDown"></param>
        /// <returns>0: for success or error code</returns>
        public ENCamResult SetCameraUpsideDown(bool UpsideDown)
        {
            if (!CameraRunning) return ENCamResult.Cam_notRunning;
            halignFilter.SetUpsideDown(UpsideDown);
            return ENCamResult.OK;
        }
        public bool GetCameraUpsideDown()
        {
            if (!CameraRunning) return false;
            return halignFilter.GetUpsideDown();
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
        public bool GetLinesHorizontal()
        {
            if (!CameraRunning) return false;
            return halignFilter.GetLinesHorizontal();
        }

        /// <summary>
        /// Checks for white lines on dark background, default = false (dark lines)
        /// </summary>
        /// <param name="Inverse"></param>
        /// <returns>0: for success or error code</returns>
        public ENCamResult SetLinesInverse(bool Inverse)
        {
            if (!CameraRunning) return ENCamResult.Cam_notRunning;
            halignFilter.SetInverse(Inverse);
            return ENCamResult.OK;
        }
        public bool GetInverseLines()
        {
            if (!CameraRunning) return false;
            return halignFilter.GetInverse();
        }

        /// <summary>
        /// set the minimum number of lines to be detected as Start-Lines
        /// </summary>
        /// <param name="MinNumStartLines">Minimum number of lines, must be > 0 </param>
        /// <returns>OK if successful</returns>
        public ENCamResult SetMinNumStartLines(UInt32 MinNumStartLines)
        {
            if (!CameraRunning) return ENCamResult.Cam_notRunning;
            if (MinNumStartLines == 0) return ENCamResult.Error;

            halignFilter.SetMinNumStartLines(MinNumStartLines);
            return ENCamResult.OK;
        }

        /// <summary>
        /// 
        /// </summary>
        public UInt32 NumDilateErodes
        {
            get
            {
                if (!CameraRunning) return 0;
                return halignFilter.GetNumDilateErodes();
            }
            set
            {
                if (CameraRunning) halignFilter.SetNumDilateErodes(value);
            }
        }

        public UInt32 NumExtraErodes
        {
            get
            {
                if (!CameraRunning) return 0;
                return halignFilter.GetNumExtraErodes();
            }
            set
            {
                if (CameraRunning) halignFilter.SetNumExtraErodes(value);
            }
        }

        public System.Drawing.Point ErodeSeed
        {
            get
            {
                System.Drawing.Point Seed = new System.Drawing.Point(0, 0);
                if (!CameraRunning) return Seed;
                Seed.X = (int)halignFilter.GetErodeSeedX();
                Seed.Y = (int)halignFilter.GetErodeSeedY();
                return Seed;
            }
            set
            {
                if (CameraRunning)
                {
                    halignFilter.SetErodeSeedX((UInt32)value.X);
                    halignFilter.SetErodeSeedY((UInt32)value.Y);
                }
            }
        }

        public bool ShowBlobOutlines
        {
            get
            {
                if (!CameraRunning) return false;
                return halignFilter.GetShowBlobOutlines();
            }
            set
            {
                if (CameraRunning) halignFilter.SetShowBlobOutlines(value);
            }
        }

        public bool ShowBlobCenters
        {
            get
            {
                if (!CameraRunning) return false;
                return halignFilter.GetShowBlobCenters();
            }
            set
            {
                if (CameraRunning) halignFilter.SetShowBlobCenters(value);
            }
        }

        public System.Drawing.Color CrossColor
        {
            get
            {
                if (!CameraRunning) return System.Drawing.Color.Transparent;
                UInt32 Current = halignFilter.GetBlobCrossColor();
                UInt32 R = Current & 0xff;
                UInt32 G = (Current & 0xff00) >> 8;
                UInt32 B = (Current & 0xff0000) >> 16;
                return System.Drawing.Color.FromArgb((int)R, (int)G, (int)B);
            }
            set
            {
                if (CameraRunning) halignFilter.SetBlobCrossColor((UInt32)((value.B << 16) + (value.G << 8) + value.R));
            }
        }

        public System.Drawing.Color BlobOutlineColor
        {
            get
            {
                if (!CameraRunning) return System.Drawing.Color.Transparent;
                UInt32 Current = halignFilter.GetBlobOutlineColor();
                UInt32 R = Current & 0xff;
                UInt32 G = (Current & 0xff00) >> 8;
                UInt32 B = (Current & 0xff0000) >> 16;
                return System.Drawing.Color.FromArgb((int)R, (int)G, (int)B);
            }
            set
            {
                if (CameraRunning) halignFilter.SetBlobOutlineColor((UInt32)((value.B << 16) + (value.G << 8) + value.R));
            }
        }

        public System.Drawing.Color BlobTextColor
        {
            get
            {
                if (!CameraRunning) return System.Drawing.Color.Transparent;
                UInt32 Current = halignFilter.GetBlobTextColor();
                UInt32 R = Current & 0xff;
                UInt32 G = (Current & 0xff00) >> 8;
                UInt32 B = (Current & 0xff0000) >> 16;
                return System.Drawing.Color.FromArgb((int)R, (int)G, (int)B);
            }
            set
            {
                if (CameraRunning) halignFilter.SetBlobTextColor((UInt32)((value.B << 16) + (value.G << 8) + value.R));
            }
        }

        public LOGFONT BlobTextFont
        {
            get
            {
                if (!CameraRunning) return null;

                //Get Size
                UInt32 FontSize = 0;
                halignFilter.GetBlobFont(IntPtr.Zero, out FontSize);
                if (FontSize == 0) return null;

                //Allocate memory
                IntPtr unmanaged_pInFont = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(LOGFONT)));
                //Get Data from Filter
                int DataFontSize = (int)FontSize;
                halignFilter.GetBlobFont(unmanaged_pInFont, out FontSize);
                if (FontSize != DataFontSize || FontSize == 0)
                {
                    Marshal.FreeHGlobal(unmanaged_pInFont);
                    return null;
                }
                LOGFONT Current = (LOGFONT)Marshal.PtrToStructure((IntPtr)unmanaged_pInFont, typeof(LOGFONT));
                Marshal.FreeHGlobal(unmanaged_pInFont);
                 
                return Current;
            }
            set
            {
                IntPtr unmanaged_pOutFont = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(LOGFONT)));
                Marshal.StructureToPtr(value, unmanaged_pOutFont, false);
                halignFilter.SetBlobFont(unmanaged_pOutFont);
                Marshal.DestroyStructure(unmanaged_pOutFont, typeof(LOGFONT));
                Marshal.FreeHGlobal(unmanaged_pOutFont);
            }
        }

        public System.Drawing.Color OverlayTextColor
        {
            get
            {
                if (!CameraRunning) return System.Drawing.Color.Transparent;
                UInt32 Current = halignFilter.GetOverlayTextColor();
                UInt32 R = Current & 0xff;
                UInt32 G = (Current & 0xff00) >> 8;
                UInt32 B = (Current & 0xff0000) >> 16;
                return System.Drawing.Color.FromArgb((int)R, (int)G, (int)B);
            }
            set
            {
                if (CameraRunning) halignFilter.SetOverlayTextColor((UInt32)((value.B << 16) + (value.G << 8) + value.R));
            }
        }

        public LOGFONT OverlayTextFont
        {
            get
            {
                if (!CameraRunning) return null;

                //Get Size
                UInt32 FontSize = 0;
                halignFilter.GetOverlayFont(IntPtr.Zero, out FontSize);
                if (FontSize == 0) return null;

                //Allocate memory
                IntPtr unmanaged_pInFont = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(LOGFONT)));
                //Get Data from Filter
                int DataFontSize = (int)FontSize;
                halignFilter.GetOverlayFont(unmanaged_pInFont, out FontSize);
                if (FontSize != DataFontSize || FontSize == 0)
                {
                    Marshal.FreeHGlobal(unmanaged_pInFont);
                    return null;
                }
                LOGFONT Current = (LOGFONT)Marshal.PtrToStructure((IntPtr)unmanaged_pInFont, typeof(LOGFONT));
                Marshal.FreeHGlobal(unmanaged_pInFont);

                return Current;
            }
            set
            {
                IntPtr unmanaged_pOutFont = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(LOGFONT)));
                Marshal.StructureToPtr(value, unmanaged_pOutFont, false);
                halignFilter.SetOverlayFont(unmanaged_pOutFont);
                Marshal.DestroyStructure(unmanaged_pOutFont, typeof(LOGFONT));
                Marshal.FreeHGlobal(unmanaged_pOutFont);
            }
        }

        public string OverlayText
        {
            set
            {
                if(CameraRunning)
                    halignFilter.SetOverlayTxt(value);
            }
        }

        /// <summary>
        /// Distance between 2 lines in μm, default = 677.333 (32 inkjet pixels)
        /// </summary>
        public float StartLineDistance
        {
            set
            {
                if (CameraRunning) halignFilter.SetStartLinesDistance(value);
            }
        }

        /// <summary>
        /// Timeout in camera frames for finding Start-Lines, default: 0 = Timeout disabled
        /// </summary>
        public UInt32 StartLineTimeout
        {
            set
            {
                if (CameraRunning) halignFilter.SetStartLinesTimeout(value);
            }
        }

        //Debug
        /// <summary>
        /// Opens a console window with debug information, default = false
        /// </summary>
        /// <param name="DebugOn"></param>
        public void SetDebug(bool DebugOn)
        {
            if (CameraRunning) halignFilter.SetDebug(DebugOn);
        }

        //Callback Debug
        /// <summary>
        /// Enables debug callback-messages, default = false
        /// </summary>
        /// <param name="CallbackDebug"></param>
        public void SetCallbackDebug(bool CallbackDebug)
        {
            if (CameraRunning && pMsgWindow != IntPtr.Zero) halignFilter.SetCallbackDebug(CallbackDebug);
        }

        //Debug to Logfile
        /// <summary>
        /// Writes debug messages to %temp%\rxAlignFilter\rxAlignFilter.log, default = false
        /// </summary>
        /// <param name="DebugToFile"></param>
        public void SetDebugLogToFile(bool DebugToFile)
        {
            if (CameraRunning && pMsgWindow != IntPtr.Zero) halignFilter.SetDebugLogToFile(DebugToFile);
        }

        //Number of OCR characters
        /// <summary>
        /// Set the number of OCR characters to be read 
        /// </summary>
        /// <param name="NumOcrChars">range: 1 - 4, default = 2</param>
        /// <returns>true for success, false if camera is not running or value is out of range</returns>
        public bool SetNumOcrChars(UInt32 NumOcrChars)
        {
            if (NumOcrChars > 4 || NumOcrChars < 1) return false;
            if (CameraRunning)
            {
                halignFilter.SetNumOcrChars(NumOcrChars);
                return true;
            }
            return false;
        }

        /// <summary>
        /// All learning functionality for OCR reading
        /// </summary>
        /// <param name="LearnCommand">OcrLearnCmdEnum</param>
        /// <param name="OcrChar">only used with LearnCommand = SaveAs: the learned character</param>
        /// <returns>true for success</returns>
        public bool SetLearnOCR(OcrLearnCmdEnum LearnCommand, char OcrChar = '\0')
        {
            if (CameraRunning)
            {
                halignFilter.SetLearnOcr((UInt32)LearnCommand, OcrChar);
                return true;
            }
            return false;
        }

        /// <summary>
        /// Displays frame processing time in debug console, default = false
        /// </summary>
        /// <param name="FrameTime"></param>
        public void ShowFrameTime(bool FrameTime)
        {
            if (CameraRunning) halignFilter.SetFrameTiming(FrameTime);
        }

        /// <summary>
        /// Sets the thicknes ratio between center and outer lines for MeasureModeEnum.MeasureMode_Register only,
        /// >+1 : center line is thicker, <-1 : center line is thinner, default 1.5,
        /// Values between -1 and +1 switch this function off
        /// </summary>
        public float RegisterMidOuterRatio
        {
            set
            {
                if (CameraRunning) halignFilter.SetRegisterMidOuterRatio(value);
            }
        }

        #endregion


        #region Internal

        public struct COPYDATASTRUCT
        {
            public IntPtr dwData;
            public int cbData;
            public IntPtr lpData;
        }

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
                                        CallBackDataStruct CallbackData = new CallBackDataStruct();
                                        CamCallBack?.Invoke(ENCamCallBackInfo.USB_CameraDisconnected, CallbackData);
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
                    while (hr == 0 && MediaControl != null)
                    {
                        hr = MediaEventEx.GetEvent(out EvCode, out lParam1, out lParam2, 0);
                        switch (EvCode)
                        {
                            case EventCode.DeviceLost:
                            {
                                if (CameraRunning)
                                {
                                    CallBackDataStruct CallbackData = new CallBackDataStruct();
                                    CamCallBack?.Invoke(ENCamCallBackInfo.DS_CaptureDeviceLost, CallbackData);
                                    StopCamera();
                                }
                                break;
                            }
                            case EventCode.ErrorAbort:
                            {
                                if (CameraRunning)
                                {
                                    CallBackDataStruct CallbackData = new CallBackDataStruct();
                                    CamCallBack?.Invoke(ENCamCallBackInfo.DS_ErrorAbotedDisplay, CallbackData);
                                    StopCamera();
                                }
                                break;
                            }
                            case EventCode.ErrorAbortEx:
                            {
                                if (CameraRunning)
                                {
                                    CallBackDataStruct CallbackData = new CallBackDataStruct();
                                    CamCallBack?.Invoke(ENCamCallBackInfo.DS_ErrorAbotedDisplay, CallbackData);
                                    StopCamera();
                                }
                                break;
                            }
                            case EventCode.StErrStopped:
                            {
                                if (CameraRunning)
                                {
                                    CallBackDataStruct CallbackData = new CallBackDataStruct();
                                    CamCallBack?.Invoke(ENCamCallBackInfo.DS_ErrorAbotedDisplay, CallbackData);
                                    StopCamera();
                                }
                                break;
                            }
                            case EventCode.VMRReconnectionFailed:
                            {
                                if (CameraRunning)
                                {
                                    CallBackDataStruct CallbackData = new CallBackDataStruct();
                                    CamCallBack?.Invoke(ENCamCallBackInfo.VMR_ReconnectionFailed, CallbackData);
                                    StopCamera();
                                }
                                break;
                            }
                            case EventCode.ClockUnset:
                            {
                                if (CameraRunning)
                                {
                                    CallBackDataStruct CallbackData = new CallBackDataStruct();
                                    CamCallBack?.Invoke(ENCamCallBackInfo.DS_CaptureDeviceLost, CallbackData);
                                    StopCamera();
                                }
                                break;
                            }
                        }
                        if(MediaControl != null) MediaEventEx.FreeEventParams(EvCode, lParam1, lParam2);
                    }
                    break;
                }

                //rx_AlignFilter Messages
                case WM_APP_ALIGNEV:
                {
                    CallBackDataStruct CallBackData;
                    ENCamResult Result;

                    switch (WParam)
                    {
                        case WP_StartLines:
                            Result = GetMeasureData(out CallBackData);
                            CallBackData.CamResult = Result;
                            CamCallBack?.Invoke(ENCamCallBackInfo.StartLinesDetected, CallBackData);
                            break;
                        case WP_StartLinesTimeout:
                            CallBackData = new CallBackDataStruct();
                            CallBackData.CamResult = new ENCamResult();
                            CallBackData.DPosX = float.NaN;
                            CallBackData.DPosY = float.NaN;
                            CallBackData.Value_1 = 0;
                            CallBackData.NumMeasures = 0;
                            CallBackData.LineLayout = LineLayoutEnum.LineLayout_Undefined;
                            CallBackData.LineAttach = LineAttachEnum.LineAttach_Undefined;
                            CallBackData.CamResult = ENCamResult.Filter_NoData;
                            CallBackData.Info = ENCamCallBackInfo.StartLinesTimeout.ToString();
                            CamCallBack?.Invoke(ENCamCallBackInfo.StartLinesDetected, CallBackData);
                            break;
                        case WP_StartLinesCont:
                            Result = GetMeasureData(out CallBackData);
                            CallBackData.CamResult = Result;
                            CamCallBack?.Invoke(ENCamCallBackInfo.StartLinesContinuous, CallBackData);
                            break;
                        case WP_Angle:
                            if(LParam == LP_TO1st)
                            {
                                //1st Measure Timeout
                                Create1stTO(out CallBackData);
                            }
                            else
                            {
                                Result = GetMeasureData(out CallBackData);
                                CallBackData.CamResult = Result;
                            }
                            CamCallBack?.Invoke(ENCamCallBackInfo.AngleCorr, CallBackData);
                            break;
                        case WP_Stitch:
                            if (LParam == LP_TO1st)
                            {
                                //1st Measure Timeout
                                Create1stTO(out CallBackData);
                            }
                            else
                            {
                                Result = GetMeasureData(out CallBackData);
                                CallBackData.CamResult = Result;
                            }
                            CamCallBack?.Invoke(ENCamCallBackInfo.StitchCorr, CallBackData);
                            break;
                        case WP_ColorStitch:
                            if (LParam == LP_TO1st)
                            {
                                //1st Measure Timeout
                                Create1stTO(out CallBackData);
                            }
                            else
                            {
                                Result = GetMeasureData(out CallBackData);
                                CallBackData.CamResult = Result;
                            }
                            CamCallBack?.Invoke(ENCamCallBackInfo.ColorStitchCorr, CallBackData);
                            break;
                        case WP_Register:
                            if (LParam == LP_TO1st)
                            {
                                //1st Measure Timeout
                                Create1stTO(out CallBackData);
                            }
                            else
                            {
                                Result = GetMeasureData(out CallBackData);
                                CallBackData.CamResult = Result;
                            }
                            CamCallBack?.Invoke(ENCamCallBackInfo.RegisterCorr, CallBackData);
                            break;
                        case WP_MeasureTimeout:
                            //Sollte nicht mehr eintreten
                            CallBackData = new CallBackDataStruct();
                            CallBackData.CamResult = new ENCamResult();
                            CamCallBack?.Invoke(ENCamCallBackInfo.MeasureTimeout, CallBackData);
                            break;
                        case WP_CallBackDebug:
                            break;
                        case WP_ReadOCR:
                            Result = GetMeasureData(out CallBackData);
                            CallBackData.CamResult = Result;
                            CamCallBack?.Invoke(ENCamCallBackInfo.OcrResult, CallBackData);
                            break;
                    }
                    break;
                }

                case WM_COPYDATA:
                {
                    COPYDATASTRUCT CopyData = new COPYDATASTRUCT();
                    CopyData = (COPYDATASTRUCT)Marshal.PtrToStructure(Msg.LParam, typeof(COPYDATASTRUCT));
                    byte[] data = new byte[CopyData.cbData];
                    Marshal.Copy(CopyData.lpData, data, 0, CopyData.cbData);
                    Encoding UnicodeStr = Encoding.ASCII;
                    char[] InfoString = UnicodeStr.GetChars(data);
                    CallBackDataStruct CallBackData = new CallBackDataStruct();
                    CallBackData.Info = new string(InfoString);
                    CamCallBack?.Invoke(ENCamCallBackInfo.DebugCallBack, CallBackData);
                    break;
                }
            }
        }

        private void Create1stTO(out CallBackDataStruct CallBackData)
        {
            CallBackData = new CallBackDataStruct();
            CallBackData.CamResult = new ENCamResult();
            CallBackData.DPosX = float.NaN;
            CallBackData.DPosY = float.NaN;
            CallBackData.Info = "1st Measure Timeout";
            CallBackData.LineAttach = LineAttachEnum.LineAttach_Undefined;
            CallBackData.LineLayout = LineLayoutEnum.LineLayout_Undefined;
            CallBackData.CamResult = ENCamResult.Filter_NoData;
            CallBackData.NumMeasures = 0;
            CallBackData.Value_1 = float.NaN;
        }

        private ENCamResult BuildGraph(IntPtr hwnd)
        {
            try
            {
                int hResult;
                //Capture Pin for Properties
                IPin SrcCapPin = null;


                if (!BuildFilterChain()) return ENCamResult.Error;

                if (!InsertVMR(hwnd)) return ENCamResult.Error;
                if (!InsertCamera(out SrcCapPin)) return ENCamResult.Error;
                if (!SetStreamCaps(SrcCapPin)) return ENCamResult.Error;
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
                CallBackDataStruct callBackData = new CallBackDataStruct();
                callBackData.Info = excep.Message;
                CamCallBack?.Invoke(ENCamCallBackInfo.CouldNotBuildGraph, callBackData);
                return ENCamResult.Error;
            }

            return LastDsErrorNum = ENCamResult.OK;
        }

        private ENCamResult StopGraph()
        {
            int hResult;

            halignFilter.SetFrameTiming(false);
            //halignFilter.SetDebug(false);
            halignFilter.SetHostPointer(IntPtr.Zero);
            halignFilter.ShowHistogram(false);
            halignFilter.SetOverlayTxt("");
            halignFilter.SetShowOriginalImage(true);
            halignFilter.SetDisplayMode(0);
            halignFilter.SetMeasureMode(0);
            halignFilter.SetBinarizeMode(0);

            try
            {
                //Stop Media notification and empty queue
                MediaEventEx.SetNotifyWindow(IntPtr.Zero, 0, IntPtr.Zero);
                EventCode EvCode;
                IntPtr lParam1, lParam2;
                int hr = 0;
                while (hr == 0)
                {
                    hr = MediaEventEx.GetEvent(out EvCode, out lParam1, out lParam2, 0);
                    MediaEventEx.FreeEventParams(EvCode, lParam1, lParam2);
                }

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

                if (SourceFilter != null) SourceFilter = null;
            }
            catch (Exception excep)
            {
                CallBackDataStruct callBackData = new CallBackDataStruct();
                callBackData.Info = excep.Message;
                CamCallBack?.Invoke(ENCamCallBackInfo.GraphNotStoppedCorrectly, callBackData);
                return ENCamResult.Error;
            }

            CameraRunning = false;
            return ENCamResult.OK;
        }

        private bool BuildFilterChain()
        {
            //Standard Filter Chain
            int hResult = 0;

            if (MediaControl != null)
            {
                hResult = Marshal.ReleaseComObject(MediaControl);
                if (hResultError(hResult)) return false;
                MediaControl = null;
            }
            if (FilterGraph2 != null)
            {
                FilterGraph2 = null;
            }
            if (CaptureGraph != null)
            {
                hResult = Marshal.ReleaseComObject(CaptureGraph);
                if (hResultError(hResult)) return false;
                CaptureGraph = null;
            }

            FilterGraph2 = (IFilterGraph2)new FilterGraph();
            MediaControl = (IMediaControl)FilterGraph2 as IMediaControl;
            CaptureGraph = (ICaptureGraphBuilder2)new CaptureGraphBuilder2();

            hResult = CaptureGraph.SetFiltergraph(FilterGraph2);
            if (hResultError(hResult)) return false;

            return true;
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

        private bool SetStreamCaps(IPin SrcCapPin)
        {
            int hRes = 0;

            if (CurrentCaps != null)
            {
                IAMStreamConfig StreamConfig1 = (IAMStreamConfig)SrcCapPin;
                VideoInfoHeader VIHeader = new VideoInfoHeader();
                AMMediaType mt = null;
                hRes = StreamConfig1.GetFormat(out mt);
                if (hResultError(hRes)) return false;

                Marshal.PtrToStructure(mt.formatPtr, VIHeader);
                VIHeader.BmiHeader.Width = Convert.ToInt32(CurrentCaps.Resolution.X);
                VIHeader.BmiHeader.Height = Convert.ToInt32(CurrentCaps.Resolution.Y);
                Marshal.StructureToPtr(VIHeader, mt.formatPtr, true);
                mt.subType = CurrentCaps.Mediasubtype;
                hRes = StreamConfig1.SetFormat(mt);
                if (hResultError(hRes)) return false;
                Marshal.FreeCoTaskMem(mt.formatPtr);
            }

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

            //Enable CallBackDebug
            //if (pMsgWindow != IntPtr.Zero) halignFilter.SetCallbackDebug(true);

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

        private bool CreateDummyChain(ref IPin SrcCapPin)
        {
            //Build DummyChain
            if (!BuildFilterChain()) return false;
            if (!InsertCamera(out SrcCapPin)) return false;

            return true;
        }

        private bool RemoveDummyChain()
        {
            int hResult = 0;

            //Remove DummyChain
            IEnumFilters pFilterEnum;
            hResult = FilterGraph2.EnumFilters(out pFilterEnum);
            if (hResultError(hResult)) return false;
            IBaseFilter[] filters = new IBaseFilter[1];
            IntPtr pfetched = Marshal.AllocCoTaskMem(4);

            while (pFilterEnum.Next(1, filters, pfetched) == 0)
            {
                hResult = FilterGraph2.RemoveFilter(filters[0]);
                if (hResultError(hResult)) return false;
                hResult = pFilterEnum.Reset();
                if (hResultError(hResult)) return false;
                hResult = Marshal.ReleaseComObject(filters[0]);
                if (hResultError(hResult)) return false;
                filters[0] = null;
            }
            hResult = Marshal.ReleaseComObject(pFilterEnum);
            if (hResultError(hResult)) return false;
            Marshal.FreeCoTaskMem(pfetched);
            pfetched = IntPtr.Zero;
            hResult = FilterGraph2.Abort();
            if (hResultError(hResult)) return false;
            hResult = Marshal.ReleaseComObject(FilterGraph2);
            if (hResultError(hResult)) return false;
            FilterGraph2 = null;
            MediaControl = null;

            if (SourceFilter != null) SourceFilter = null;

            return true;
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

        private ENCamResult GetMeasureData(out CallBackDataStruct MeasureData)
        {
            MeasureData = new CallBackDataStruct();
            MeasureData.OcrChars = new char[4];

            //Raw data list
            List<CallBackDataStruct> CorrectionList;
            ENCamResult DataResult = GetMeasureDataList(out CorrectionList);
            if (DataResult != ENCamResult.OK && DataResult != ENCamResult.Filter_NoData && DataResult != ENCamResult.Filter_DataTimeout) return DataResult;

            if (CorrectionList.Count > 0 && CorrectionList[CorrectionList.Count - 1].Info != "")
                MeasureData.Info = CorrectionList[CorrectionList.Count - 1].Info;

            if (CorrectionList.Count == 0 || DataResult == ENCamResult.Filter_NoData)
            {
                MeasureData.CamResult = new ENCamResult();
                MeasureData.DPosX = float.NaN;
                MeasureData.DPosY = float.NaN;
                MeasureData.Value_1 = float.NaN;
                MeasureData.NumMeasures = 0;
                MeasureData.Info = ENCamCallBackInfo.NoDataFromFilter.ToString();
                MeasureData.LineLayout = LineLayoutEnum.LineLayout_Undefined;
                MeasureData.CamResult = ENCamResult.Filter_NoData;
                MeasureData.Info = ENCamCallBackInfo.MeasureTimeout.ToString();
            }
            else if(CorrectionList.Count == 1)
            {
                MeasureData = CorrectionList[0];
                MeasureData.NumMeasures = 1;
            }
            else
            {
                int NumMeasures = CorrectionList.Count;
                //Average within 1 sigma
                if (CorrectionList.Count > 1)
                    AverageData(CorrectionList, ref MeasureData);
                else MeasureData = CorrectionList[0];
                MeasureData.NumMeasures = NumMeasures;
            }

            return ENCamResult.OK;
        }

        private ENCamResult GetMeasureDataList(out List<CallBackDataStruct> MeasureDataList)
        {
            MeasureDataList = new List<CallBackDataStruct>();
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
            IntPtr unmanaged_pInList = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(CallBackDataStruct)) * (int)DataListSize32);
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
                MeasureDataList.Add((CallBackDataStruct)Marshal.PtrToStructure(current, typeof(CallBackDataStruct)));
                Marshal.DestroyStructure(current, typeof(CallBackDataStruct));
                current = (IntPtr)((long)current + Marshal.SizeOf(typeof(CallBackDataStruct)));
            }
            Marshal.FreeHGlobal(unmanaged_pInList);

            if(MeasureDataList.Count == 0) return ENCamResult.Filter_NoData;

            return ENCamResult.OK;
        }

        private void AverageData(List<CallBackDataStruct> CorrectionList, ref CallBackDataStruct MeasureData)
        {
            //Average Data within 1 sigma

            //Mean
            float MeanV1 = 0;
            int MeanLineLayout = 0;
            int MeanLineAttach = 0;
            float MeanDX = 0;
            float MeanDY = 0;
            int MeanOcr_0 = 0;
            int MeanOcr_1 = 0;
            int MeanOcr_2 = 0;
            int MeanOcr_3 = 0;
            for (int i = 0; i < CorrectionList.Count; i++)
            {
                MeanV1 += CorrectionList[i].Value_1;
                MeanLineLayout += (int)CorrectionList[i].LineLayout;
                MeanLineAttach += (int)CorrectionList[i].LineAttach;
                MeanDX += CorrectionList[i].DPosX;
                MeanDY += CorrectionList[i].DPosY;
                MeanOcr_0 += CorrectionList[i].OcrChars[0];
                MeanOcr_1 += CorrectionList[i].OcrChars[1];
                MeanOcr_2 += CorrectionList[i].OcrChars[2];
                MeanOcr_3 += CorrectionList[i].OcrChars[3];
            }
            MeanV1 /= CorrectionList.Count;
            MeanLineLayout /= CorrectionList.Count;
            MeanLineAttach /= CorrectionList.Count;
            MeanDX /= CorrectionList.Count;
            MeanDY /= CorrectionList.Count;
            MeanOcr_0 /= CorrectionList.Count;
            MeanOcr_1 /= CorrectionList.Count;
            MeanOcr_2 /= CorrectionList.Count;
            MeanOcr_3 /= CorrectionList.Count;

            //Variance
            double VarV1 = 0;
            double VarLineLayout = 0;
            double VarLineAttach = 0;
            double VarDX = 0;
            double VarDY = 0;
            double VarOcr_0 = 0;
            double VarOcr_1 = 0;
            double VarOcr_2 = 0;
            double VarOcr_3 = 0;
            for (int i = 0; i < CorrectionList.Count; i++)
            {
                VarV1 += Math.Pow(CorrectionList[i].Value_1 - MeanV1, 2);
                VarLineLayout += Math.Pow((int)CorrectionList[i].LineLayout - MeanLineLayout, 2);
                VarLineAttach += Math.Pow((int)CorrectionList[i].LineAttach - MeanLineAttach, 2);
                VarDX += Math.Pow(CorrectionList[i].DPosX - MeanDX, 2);
                VarDY += Math.Pow(CorrectionList[i].DPosY - MeanDY, 2);
                VarOcr_0 += Math.Pow(CorrectionList[i].OcrChars[0] - MeanOcr_0, 2);
                VarOcr_1 += Math.Pow(CorrectionList[i].OcrChars[1] - MeanOcr_1, 2);
                VarOcr_2 += Math.Pow(CorrectionList[i].OcrChars[2] - MeanOcr_2, 2);
                VarOcr_3 += Math.Pow(CorrectionList[i].OcrChars[3] - MeanOcr_3, 2);
            }
            VarV1 /= CorrectionList.Count;
            VarLineLayout /= CorrectionList.Count;
            VarLineAttach /= CorrectionList.Count;
            VarDX /= CorrectionList.Count;
            VarDY /= CorrectionList.Count;
            VarOcr_0 /= CorrectionList.Count;
            VarOcr_1 /= CorrectionList.Count;
            VarOcr_2 /= CorrectionList.Count;
            VarOcr_3 /= CorrectionList.Count;

            //StdDev
            VarV1 = Math.Sqrt(VarV1);
            VarLineLayout = Math.Sqrt(VarLineLayout);
            VarLineAttach = Math.Sqrt(VarLineAttach);
            VarDX = Math.Sqrt(VarDX);
            VarDY = Math.Sqrt(VarDY);
            VarOcr_0 = Math.Sqrt(VarOcr_0);
            VarOcr_1 = Math.Sqrt(VarOcr_1);
            VarOcr_2 = Math.Sqrt(VarOcr_2);
            VarOcr_3 = Math.Sqrt(VarOcr_3);

            //Average within 1 sigma
            int Counter = 0;
            int OcrSum_0 = 0;
            int OcrSum_1 = 0;
            int OcrSum_2 = 0;
            int OcrSum_3 = 0;

            for (int i = 0; i < CorrectionList.Count; i++)
            {
                if (CorrectionList[i].Value_1 <= MeanV1 + VarV1 &&
                    CorrectionList[i].Value_1 >= MeanV1 - VarV1 &&
                    (int)CorrectionList[i].LineLayout <= MeanLineLayout + VarLineLayout &&
                    (int)CorrectionList[i].LineLayout >= MeanLineLayout - VarLineLayout &&
                    (int)CorrectionList[i].LineAttach <= MeanLineAttach + VarLineAttach &&
                    (int)CorrectionList[i].LineAttach >= MeanLineAttach - VarLineAttach &&
                    CorrectionList[i].DPosX <= MeanDX + VarDX &&
                    CorrectionList[i].DPosX >= MeanDX - VarDX &&
                    CorrectionList[i].DPosY <= MeanDY + VarDY &&
                    CorrectionList[i].DPosY >= MeanDY - VarDY &&
                    CorrectionList[i].OcrChars[0] <= MeanOcr_0 + VarOcr_0 &&
                    CorrectionList[i].OcrChars[0] >= MeanOcr_0 - VarOcr_0 &&
                    CorrectionList[i].OcrChars[1] <= MeanOcr_1 + VarOcr_1 &&
                    CorrectionList[i].OcrChars[1] >= MeanOcr_1 - VarOcr_1 &&
                    CorrectionList[i].OcrChars[2] <= MeanOcr_2 + VarOcr_2 &&
                    CorrectionList[i].OcrChars[2] >= MeanOcr_2 - VarOcr_2 &&
                    CorrectionList[i].OcrChars[3] <= MeanOcr_3 + VarOcr_3 &&
                    CorrectionList[i].OcrChars[3] >= MeanOcr_3 - VarOcr_3 )
                {
                    Counter++;
                    MeasureData.Value_1 += CorrectionList[i].Value_1;
                    MeasureData.LineLayout += (int)CorrectionList[i].LineLayout;
                    MeasureData.LineAttach += (int)CorrectionList[i].LineAttach;
                    MeasureData.DPosX += CorrectionList[i].DPosX;
                    MeasureData.DPosY += CorrectionList[i].DPosY;
                    OcrSum_0 += CorrectionList[i].OcrChars[0];
                    OcrSum_1 += CorrectionList[i].OcrChars[1];
                    OcrSum_2 += CorrectionList[i].OcrChars[2];
                    OcrSum_3 += CorrectionList[i].OcrChars[3];
                }
            }
            if (Counter > 0)
			{
                MeasureData.Value_1 /= Counter;
                MeasureData.LineLayout = (LineLayoutEnum)((int)MeasureData.LineLayout / Counter);
                MeasureData.LineAttach = (LineAttachEnum)((int)MeasureData.LineAttach / Counter);
                MeasureData.DPosX /= Counter;
                MeasureData.DPosY /= Counter;
                MeasureData.OcrChars[0] = (char)(OcrSum_0 / Counter);
                MeasureData.OcrChars[1] = (char)(OcrSum_1 / Counter);
                MeasureData.OcrChars[2] = (char)(OcrSum_2 / Counter);
                MeasureData.OcrChars[3] = (char)(OcrSum_3 / Counter);
            }
        }

        private void TimerElapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            System.Timers.Timer tmr = (System.Timers.Timer)sender;
            tmr.Enabled = false;
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
