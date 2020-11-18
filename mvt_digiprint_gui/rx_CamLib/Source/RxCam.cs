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

namespace rx_CamLib
{
    public class RxCam
    {
        #region Definitions

        private RxCamSettings _CamSettings;

        private DsDevice[] DeviceList = null;
        private bool CameraRunning = false;
        private bool GraphStarting = false;

        private string LastDsErrorMsg = "";
        private int LastDsErrorNum = 0;

        private DsDevice Camera = null;
        private IFilterGraph2 FilterGraph2 = null;
        private IBaseFilter AlignFilter = null;
        private bool AlignFilterConnected = false;
        private IFrx_AlignFilter AlignFilterIF = null;
        private IMediaControl MediaControl = null;
        private ICaptureGraphBuilder2 CaptureGraph = null;
        private IBaseFilter SourceFilter = null;
        private IBaseFilter VMR9;
        private IVMRWindowlessControl9 WindowlessCtrl9 = null;

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

        #region Interface to rx_AlignFilter

        //Keep in order as defined in dll !!!

        //Expose Interface of Align Filter
        [ComImport,
        System.Security.SuppressUnmanagedCodeSecurity,
        Guid("C915723A-FE83-4914-AAFA-C7C486A41AAC"),           //Filter Interface IFrx_AlignFilter
        InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
        public interface IFrx_AlignFilter
        {
            #region General

            //Version Info
            [PreserveSig]
            void GetVersion(IntPtr VersionInfo, out UInt32 VersionInfoSize);
            //C++ Version Info
            [PreserveSig]
            UInt32 GetCppVersion();
            //WindowHandle of Hoste
            [PreserveSig]
            void SetHostPointer(IntPtr HostHwnd);
            //Graphics Device Name
            [PreserveSig]
            void GetDeviceName(IntPtr DeviceName);
            //Debug On
            [PreserveSig]
            void SetDebug(bool DebugOn);
            //Display Frame Timing
            [PreserveSig]
            void SetFrameTiming(bool DspFrameTime);
            //Take SnapShot and Save to
            [PreserveSig]
            bool TakeSnapShot(string SnapDirectory, string SnapFileName);

            //ShowOriginalImage
            [PreserveSig]
            void SetShowOriginalImage(bool ShowOriginalImage);
            [PreserveSig]
            bool GetShowOriginalImage();

            //Overlay-Text
            [PreserveSig]
            bool SetOverlayTxt(string OverlayTxt, UInt32 OverlayTxtColor);

            #endregion

            #region Binarization

            //Binarization
            [PreserveSig]
            void SetBinarizeMode(UInt32 BinarizeMode);			// //0: off, 1: Auto, 2:Adaptive
            [PreserveSig]
            UInt32 GetBinarizeMode();

            //Threshold
            [PreserveSig]
            void SetThreshold(UInt32 Threshold);
            [PreserveSig]
            UInt32 GetThreshold();

            //Show Histogram
            [PreserveSig]
            void ShowHistogram(bool ShowHistogram);         // Switch ShowHistogram On/Off

            #endregion

            #region Dilate-Erode

            //Num Dilate-Erodes
            [PreserveSig]
            void SetNumDilateErodes(UInt32 DilateErodes);
            [PreserveSig]
            UInt32 GetNumDilateErodes();

            //Num Extra-Erodes
            [PreserveSig]
            void SetNumExtraErodes(UInt32 ExtraErodes);
            [PreserveSig]
            UInt32 GetNumExtraErodes();

            //Erode Seed
            [PreserveSig]
            void SetErodeSeedX(UInt32 ErodeSeedX);
            [PreserveSig]
            void SetErodeSeedY(UInt32 ErodeSeedY);
            [PreserveSig]
            UInt32 GetErodeSeedX();
            [PreserveSig]
            UInt32 GetErodeSeedY();

            #endregion

            #region Blob

            //Cross Color
            [PreserveSig]
            void SetCrossColor(UInt32 CrossColor);
            [PreserveSig]
            UInt32 GetCrossColor();

            //BlobOutlineColor
            [PreserveSig]
            void SetBlobOutlineColor(UInt32 BlobOutlineColor);
            [PreserveSig]
            UInt32 GetBlobOutlineColor();
            //BlobCrossColor
            [PreserveSig]
            void SetBlobCrossColor(UInt32 BlobCrossColor);
            [PreserveSig]
            UInt32 GetBlobCrossColor();
            //BlobTextColor
            [PreserveSig]
            void SetBlobTextColor(UInt32 BlobTextColor);
            [PreserveSig]
            UInt32 GetBlobTextColor();

            //BlobAspectLimit
            [PreserveSig]
            void SetBlobAspectLimit(UInt32 BlobAspectLimit);
            [PreserveSig]
            UInt32 GetBlobAspectLimit();
            //BlobAreaDivisor
            [PreserveSig]
            void SetBlobAreaDivisor(UInt32 BlobAreaDivisor);
            [PreserveSig]
            UInt32 GetBlobAreaDivisor();

            //ShowBlobOutlines
            [PreserveSig]
            void SetShowBlobOutlines(bool ShowBlobOutlines);
            [PreserveSig]
            bool GetShowBlobOutlines();
            //ShowBlobCenters
            [PreserveSig]
            void SetShowBlobCenters(bool ShowBlobCenters);
            [PreserveSig]
            bool GetShowBlobCenters();
            //ShowBlobValues
            [PreserveSig]
            void SetShowBlobValues(bool ShowBlobValues);
            [PreserveSig]
            bool GetShowBlobValues();

            //Font for Blob Values
            [PreserveSig]
            void SetBlobFont(IntPtr pLogFontStruct);
            [PreserveSig]
            void GetBlobFont(IntPtr pLogFontStruct, out UInt32 LogFontSize);

            #endregion

            #region Measurement

            //Measure in Pixels
            [PreserveSig]
            void SetMeasurePixels(bool MeasurePixels);
            [PreserveSig]
            bool GetMeasurePixels();

            //Inverse Image for White
            [PreserveSig]
            void SetInverse(bool Inverse);
            [PreserveSig]
            bool GetInverse();

            //MeasureMode
            [PreserveSig]
            void SetMeasureMode(UInt32 MeasureMode);
            [PreserveSig]
            UInt32 GetMeasureMode();

            //DisplayMode
            [PreserveSig]
            void SetDisplayMode(UInt32 DisplayMode);
            [PreserveSig]
            UInt32 GetDisplayMode();

            #endregion

            #region Line-Direction

            //Line Direction
            [PreserveSig]
            void SetLinesHorizontal(bool Horizontal);
            [PreserveSig]
            bool GetLinesHorizontal();

            #endregion

        }

        #endregion


        public RxCam()
        {
            _CamSettings = new RxCamSettings();
            _CamSettings.PropertyChanged += new System.ComponentModel.PropertyChangedEventHandler(PropertyChange);
            _CamSettings.PropertyReading += new System.ComponentModel.PropertyChangedEventHandler(PropertyRead);
        }

        ~RxCam()
        {
            if (CameraRunning) StopGraph();
        }


        #region Public Interface

        public delegate void CameraCallBack(string CallbackData);
        public event CameraCallBack CamCallBack = null;

        public event Action CamMarkFound = null;

        public delegate void PositionMeasuredCallback(int angle, int stich, int encoder);
        public event PositionMeasuredCallback CamPositionMeasured = null;

        /// <summary>
        /// Get version of library
        /// </summary>
        /// <returns>Version</returns>
        public string GetLibVersion()
        {
            return typeof(RxCam).Assembly.GetName().Version.ToString();
        }

        public RxCamSettings Settings
        {
            get { return _CamSettings; }
            set { _CamSettings = value; }
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
        /// <item><description> 0: for success or error code:</description></item>
        /// <item><description>-1: Camera already running</description></item>
        /// <item><description>-2: No cameras detected (yet)</description></item>
        /// <item><description>-3: Camera name not available</description></item></list>
        /// </returns>
        public int SelectCamera(string name)
        {
            if (DeviceList == null) DeviceList = DsDevice.GetDevicesOfCat(FilterCategory.VideoInputDevice);
            if (DeviceList == null || DeviceList.Length == 0)
            {
                LastDsErrorMsg = "No cameras detected";
                LastDsErrorNum = -2;
                return -2;
            }
            for (int i = 0; i < DeviceList.Length; i++)
            {
                if (DeviceList[i].Name.Equals(name))
                {
                    if (CameraRunning)
                    {
                        LastDsErrorMsg = "Camera already running";
                        LastDsErrorNum = -1;
                        return -1;
                    }

                    Camera = DeviceList[i];
                    _CamSettings._DeviceName = Camera.Name;
                    return 0;
                }
            }

            LastDsErrorMsg = "Selected Camera not found";
            LastDsErrorNum = -3;
            return -3;
        }

        /// <summary>
        /// Starts camera caption, call SetVideoRectangle directly afterwards and if size changed
        /// </summary>
        /// <param name="hwnd">Window handle of displaying element</param>
        /// <param name="PropDialog">Show camera property dialog</param>
        /// <returns>
        /// <list type="table">
        /// <item><description> 0: for success or error code:</description></item>
        /// <item><description>-1: Camera already running</description></item>
        /// <item><description>-2: No camera selected</description></item>
        /// <item><description>-3: Could not start camera, check GetLastDsError</description></item>
        /// </list>
        /// </returns>
        public int StartCamera(IntPtr hwnd, bool PropDialog = false)
        {
            if (CameraRunning)
            {
                LastDsErrorMsg = "Camera already running";
                LastDsErrorNum = -1;
                return -1;
            }
            if (Camera == null)
            {
                LastDsErrorMsg = "No camera selected";
                LastDsErrorNum = -2;
                return -2;
            }

            if (!BuildGraph(hwnd, PropDialog))
            {
                GraphStarting = false;
                return -3;
            }

            CameraRunning = true;
            _CamSettings._CameraRunning = CameraRunning;
            CamCallBack?.Invoke("Camera started");
            return 0;
        }

        /// <summary>
        /// Sets the display rectangle
        /// </summary>
        /// <param name="PicRectangle">ClientRectangle of displaying device</param>
        /// <returns>0: for success or error code</returns>
        public int SetVideoRectangle(System.Drawing.Rectangle PicRectangle)
        {
            if (WindowlessCtrl9 == null) return -1;
            int hResult = WindowlessCtrl9.SetVideoPosition(null, DsRect.FromRectangle(PicRectangle));
            if (hResultError(hResult)) return -1;
            return 0;
        }

        /// <summary>
        /// Stops camera caption
        /// </summary>
        /// <returns>0: for success or error code</returns>
        public int StopCamera()
        {
            if (!CameraRunning)
            {
                LastDsErrorMsg = "Camera not running";
                LastDsErrorNum = -1;
                return -1;
            }
            if (!StopGraph()) return -2;

            LastDsErrorMsg = "";
            LastDsErrorNum = 0;
            CamCallBack?.Invoke("Camera stopped");
            return 0;
        }

        /// <summary>
        /// Get Last DirectShow Error
        /// </summary>
        /// <param name="Message">Error Message</param>
        /// <returns>Error Number</returns>
        public int GetLastDsError(out string Message)
        {
            Message = LastDsErrorMsg;
            if (LastDsErrorNum == -2147467259)
                Message = "Resource used by this filter may already be in use";
            return LastDsErrorNum;
        }

        /// <summary>
        /// Opens camera setup dialog
        /// </summary>
        /// <returns>0: for success or error code</returns>
        public int ShowCameraSetup()
        {
            if (!CameraRunning)
            {
                LastDsErrorMsg = "Camera not running";
                LastDsErrorNum = -1;
                return -1;
            }
            int hResult = DisplayPropertyPages(SourceFilter, Camera.Name);
            if (hResultError(hResult))
            {
                return -1;
            }
            return 0;
        }

        public struct CamCapStruct
        {
            public string Name;
            public bool Available;
            public int Minimum;
            public int Maximum;
            public int Step;
            public int Default;
            public int Value;
            public CameraControlFlags Flags;
            public CameraControlFlags Flag;
        }

        public struct CamVideoProcStruct
        {
            public string Name;
            public bool Available;
            public int Minimum;
            public int Maximum;
            public int Step;
            public int Default;
            public int Value;
            public VideoProcAmpFlags Flags;
            public VideoProcAmpFlags Flag;
        }

        #endregion


        #region Internal

        private bool BuildGraph(IntPtr hwnd, bool propDialog)
        {
            int hResult;
            GraphStarting = true;

            //Capture Pin for Properties
            IPin SrcCapPin = null;

            //Standard Filter Chain
            FilterGraph2 = (IFilterGraph2)new FilterGraph();
            MediaControl = (IMediaControl)FilterGraph2 as IMediaControl;
            CaptureGraph = (ICaptureGraphBuilder2)new CaptureGraphBuilder2();
            hResult = CaptureGraph.SetFiltergraph(FilterGraph2);
            if (hResultError(hResult)) return false;
            VMR9 = (IBaseFilter)new VideoMixingRenderer9();
            if (!ConfigVMR9(hwnd)) return false;
            hResult = FilterGraph2.AddFilter(VMR9, "VMR 9");
            if (hResultError(hResult)) return false;
            //Insert Camera
            object source;
            Guid SourceGuid = typeof(IBaseFilter).GUID;
            try
            {
                Camera.Mon.BindToObject(null, null, ref SourceGuid, out source);
                SourceFilter = (IBaseFilter)source;
            }
            catch (Exception excep)
            {
                LastDsErrorMsg = excep.Message;
                LastDsErrorNum = -1;
                return false;
            }
            hResult = FilterGraph2.AddFilter(SourceFilter, Camera.Name);
            if (hResultError(hResult)) return false;

            hResult = DsFindPin(SourceFilter, ref SrcCapPin, PinCategory.Capture);
            if (hResultError(hResult)) return false;

            //Stream settings
            if (propDialog)
            {
                hResult = DisplayPropertyPages(SrcCapPin, "Capture");
                if (hResultError(hResult)) return false;
            }
            else
            {
                SetCamStreamCaps(_CamSettings._StreamCaps);
            }

            //Read current Camera settings
            GetCamStreamCapsList(out _CamSettings._StreamCapsList);
            GetCamCapSettingsList(out _CamSettings._CamCapsList);
            GetCamVideoProcSettingsList(out _CamSettings._CamVideoProcList);

            //Insert Align Filter
            Guid AlignGuid = new Guid("148BC1EB-2C83-418E-B9CD-E1F5BC9D1E38");  //rx_AlignFilter
            //Guid AlignGuid = new Guid("3C84E851-7D06-434F-81E1-5E68F0306E8B"); //Bieler_dsAlign
            Type comType = null;
            comType = Type.GetTypeFromCLSID(AlignGuid);
            AlignFilter = (IBaseFilter)Activator.CreateInstance(comType);
            hResult = FilterGraph2.AddFilter((IBaseFilter)AlignFilter, "Alignment");
            if (hResultError(hResult)) return false;

            AlignFilterIF = AlignFilter as IFrx_AlignFilter;
            AlignFilterConnected = true;
            if (AlignFilterIF == null)
            {
                AlignFilterConnected = false;
            }

            //Connect Camera to Align
            IPin AlignInPin = null;
            hResult = DsFindPin((IBaseFilter)AlignFilter, ref AlignInPin, "Input");
            if (hResultError(hResult)) return false;
            hResult = FilterGraph2.Connect(SrcCapPin, AlignInPin);
            if (hResultError(hResult)) return false;

            //Render Align Output Pin
            IPin AlignOutPin = null;
            hResult = DsFindPin((IBaseFilter)AlignFilter, ref AlignOutPin, "Output");
            if (hResultError(hResult)) return false;
            hResult = FilterGraph2.Render(AlignOutPin);
            if (hResultError(hResult)) return false;

            //Start Display
            FilterState pFState = FilterState.Stopped;
            hResult = MediaControl.Run();
            if (hResultError(hResult)) return false;
            try
            {
                while (pFState != FilterState.Running)
                {
                    Application.DoEvents();
                    hResult = MediaControl.GetState(10, out pFState);
                }
            }
            catch (Exception exep)
            {
                MessageBox.Show("Could not start Media Control:\n" + exep.Message, "Build Graph", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }


            //Read current stream settings
            hResult = GetCamStreamCaps(out _CamSettings._StreamCaps);

            //Set Camera settings
            SetCamCapSettingsList(_CamSettings._CamCapsList);
            SetCamVideoProcSettingsList(_CamSettings._CamVideoProcList);

            GetAllFilterSettings();

            GraphStarting = false;
            return true;
        }

        private bool StopGraph()
        {
            int hResult;

            AlignFilterIF.SetFrameTiming(false);
            AlignFilterIF.SetDebug(false);
            AlignFilterIF.SetHostPointer(IntPtr.Zero);
            AlignFilterIF.ShowHistogram(false);
            AlignFilterIF.SetOverlayTxt("", 0);
            AlignFilterIF.SetShowOriginalImage(true);
            AlignFilterIF.SetDisplayMode(0);
            AlignFilterIF.SetMeasureMode(0);
            AlignFilterIF.SetBinarizeMode(0);

            //Stop Media Control
            FilterState pFState = FilterState.Running;
            if (MediaControl != null)
            {
                hResult = MediaControl.GetState(10, out pFState);
                if (pFState == FilterState.Running)
                {
                    try
                    {
                        hResult = MediaControl.Pause();
                        hResultError(hResult);
                        while (pFState != FilterState.Paused)
                        {
                            Application.DoEvents();
                            hResult = MediaControl.GetState(10, out pFState);
                        }
                    }
                    catch (Exception exep)
                    {
                        MessageBox.Show("Could not pause Media Control:\n" + exep.Message, "Pause Graph", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                }

                if (pFState != FilterState.Stopped)
                {
                    try
                    {
                        hResult = MediaControl.Stop();
                        hResultError(hResult);
                        while (pFState != FilterState.Stopped)
                        {
                            Application.DoEvents();
                            hResult = MediaControl.GetState(10, out pFState);
                        }
                    }
                    catch (Exception exep)
                    {
                        MessageBox.Show("Could not stop Media Control:\n" + exep.Message, "Stop Graph", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                }
            }

            //Remove all Filters
            if (FilterGraph2 != null)
            {
                hResult = FilterGraph2.Abort();
                hResultError(hResult);
            }

            if (SourceFilter != null)
            {
                hResult = FilterGraph2.RemoveFilter(SourceFilter);
                hResultError(hResult);
            }

            if (VMR9 != null)
            {
                hResult = FilterGraph2.RemoveFilter(VMR9);
                hResultError(hResult);
            }
            WindowlessCtrl9 = null;
            if (VMR9 != null) Marshal.ReleaseComObject(VMR9);
            VMR9 = null;

            if (AlignFilter != null)
            {
                hResult = FilterGraph2.RemoveFilter(AlignFilter);
                hResultError(hResult);
            }
            AlignFilterConnected = false;

            if (SourceFilter != null) Marshal.ReleaseComObject(SourceFilter);
            SourceFilter = null;

            if (CaptureGraph != null) Marshal.ReleaseComObject(CaptureGraph);
            CaptureGraph = null;
            if (MediaControl != null) Marshal.ReleaseComObject(MediaControl);
            MediaControl = null;

            if (FilterGraph2 != null) Marshal.ReleaseComObject(FilterGraph2);
            FilterGraph2 = null;

            CameraRunning = false;
            _CamSettings._CameraRunning = CameraRunning;


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

        private int DsFindPin(IBaseFilter Filter, ref IPin Pin, Guid Category)
        {

            //Find specified Pin of a Filter

            int hResult = 0;
            if(!CameraRunning && !GraphStarting)
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
            Marshal.FreeCoTaskMem(pcFetched);
            pcFetched = IntPtr.Zero;
            pins = null;
            ppEnum = null;

            return -1;  //Pin not found
        }

        private int DisplayPropertyPages(object FilterElement, string ElementName)
        {
            //Display PropertyPage of Filter

            int hRes = 0;

            //Properties for Filter element
            ISpecifyPropertyPages pProp = FilterElement as ISpecifyPropertyPages;
            if (pProp != null)
            {
                // get the propertypages from the property bag
                DsCAUUID caGUID;
                hRes = pProp.GetPages(out caGUID);
                DsError.ThrowExceptionForHR(hRes);

                // create and display the OlePropertyFrame
                object[] oDevice = new[] { (object)FilterElement };
                hRes = OleCreatePropertyFrame(IntPtr.Zero, 0, 0,
                                                ElementName, 1, oDevice,
                                                caGUID.cElems, caGUID.ToGuidArray(),
                                                0, 0, 0);
                DsError.ThrowExceptionForHR(hRes);

                // release COM objects
                Marshal.FreeCoTaskMem(caGUID.pElems);
                pProp = null;
            }
            return hRes;
        }


        private bool hResultError(int hResult)
        {
            LastDsErrorMsg = DsError.GetErrorText(hResult);
            LastDsErrorNum = hResult;

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
            _CamSettings._GraphicDevice = GetFilterGraphDeviceName();
            _CamSettings._FilterVersion = GetFilterVersion();
            _CamSettings._BinarizeMode = AlignFilterIF.GetBinarizeMode();
            _CamSettings._Inverse = AlignFilterIF.GetInverse();
            _CamSettings._Threshold = AlignFilterIF.GetThreshold();
            _CamSettings._ShowProcessImage = !AlignFilterIF.GetShowOriginalImage();
            _CamSettings._ValuesFont = GetFilterValuesFont();
            _CamSettings._TextColor = AlignFilterIF.GetBlobTextColor();

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

        private LOGFONT GetFilterValuesFont()
        {
            UInt32 LogFontSize = 0;
            AlignFilterIF.GetBlobFont(IntPtr.Zero, out LogFontSize);
            IntPtr unmanaged_pInFont = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(LOGFONT)) * (int)LogFontSize);
            AlignFilterIF.GetBlobFont(unmanaged_pInFont, out LogFontSize);
            LOGFONT ValuesFont = new LOGFONT();
            ValuesFont = (LOGFONT)Marshal.PtrToStructure(unmanaged_pInFont, typeof(LOGFONT));
            Marshal.DestroyStructure(unmanaged_pInFont, typeof(LOGFONT));
            Marshal.FreeHGlobal(unmanaged_pInFont);
            return ValuesFont;
        }

        private string GetFilterVersion()
        {
            //Filter Version
            UInt32 StrLength = 0;
            string FilterVersion;
            if (AlignFilterIF != null)
            {
                AlignFilterIF.GetVersion(IntPtr.Zero, out StrLength);
                IntPtr unmanaged_pIn = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(char)) * 256);
                AlignFilterIF.GetVersion(unmanaged_pIn, out StrLength);
                FilterVersion = Marshal.PtrToStringAuto(unmanaged_pIn);
                Marshal.FreeHGlobal(unmanaged_pIn);
            }
            else
            {
                Guid AlignGuid = new Guid("148BC1EB-2C83-418E-B9CD-E1F5BC9D1E38");  //rx_AlignFilter
                Type comType = null;
                comType = Type.GetTypeFromCLSID(AlignGuid);
                AlignFilter = (IBaseFilter)Activator.CreateInstance(comType);
                AlignFilterIF = AlignFilter as IFrx_AlignFilter;
                IntPtr unmanaged_pIn = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(char)) * 256);
                AlignFilterIF.GetVersion(unmanaged_pIn, out StrLength);
                FilterVersion = Marshal.PtrToStringAuto(unmanaged_pIn);
                Marshal.FreeHGlobal(unmanaged_pIn);
                AlignFilterIF = null;
                AlignFilter = null;
            }
            return FilterVersion;
        }

        private string GetFilterGraphDeviceName()
        {
            //Graphics Device Name
            IntPtr unmanaged_pIn = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(char)) * 256);
            AlignFilterIF.GetDeviceName(unmanaged_pIn);
            string DeviceName = Marshal.PtrToStringAuto(unmanaged_pIn);
            Marshal.FreeHGlobal(unmanaged_pIn);
            return DeviceName;
        }



        private int GetCamStreamCapsList(out List<StreamCaps> StreamCapsList)
        {
            int hRes = 0;
            IPin SrcCapPin = null;
            StreamCapsList = new List<StreamCaps>();
            bool TempFilter = false;

            if(SourceFilter == null)
            {
                FilterGraph2 = (IFilterGraph2)new FilterGraph();
                CaptureGraph = (ICaptureGraphBuilder2)new CaptureGraphBuilder2();
                hRes = CaptureGraph.SetFiltergraph(FilterGraph2);
                if (hResultError(hRes)) return hRes;

                object source;
                Guid SourceGuid = typeof(IBaseFilter).GUID;
                try
                {
                    Camera.Mon.BindToObject(null, null, ref SourceGuid, out source);
                    SourceFilter = (IBaseFilter)source;
                }
                catch (Exception excep)
                {
                    LastDsErrorMsg = excep.Message;
                    LastDsErrorNum = -1;
                    return -1;
                }
                hRes = FilterGraph2.AddFilter(SourceFilter, Camera.Name);
                if (hResultError(hRes)) return hRes;

                TempFilter = true;
            }

            hRes = DsFindPin(SourceFilter, ref SrcCapPin, PinCategory.Capture);
            if (hResultError(hRes)) return hRes;

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
                    StreamCapsList.Add(Resolution);
                }
            }

            Marshal.FreeCoTaskMem(mt.formatPtr);
            Marshal.FreeHGlobal(pSC);

            if(TempFilter)
            {
                FilterGraph2.Abort();
                FilterGraph2.RemoveFilter(SourceFilter);
                Marshal.ReleaseComObject(SourceFilter);
                SourceFilter = null;
                Marshal.ReleaseComObject(CaptureGraph);
                CaptureGraph = null;
                Marshal.ReleaseComObject(FilterGraph2);
                FilterGraph2 = null;
            }

            return StreamCapsList.Count;
        }

        private int GetCamStreamCaps(out StreamCaps StreamCaps)
        {
            int hRes = 0;
            IPin SrcCapPin = null;
            StreamCaps = new StreamCaps();
            bool TempFilter = false;

            if (SourceFilter == null)
            {
                FilterGraph2 = (IFilterGraph2)new FilterGraph();
                CaptureGraph = (ICaptureGraphBuilder2)new CaptureGraphBuilder2();
                hRes = CaptureGraph.SetFiltergraph(FilterGraph2);
                if (hResultError(hRes)) return hRes;

                object source;
                Guid SourceGuid = typeof(IBaseFilter).GUID;
                if (Camera!=null)
				{
                    try
                    {
                        Camera.Mon.BindToObject(null, null, ref SourceGuid, out source);
                        SourceFilter = (IBaseFilter)source;
                    }
                    catch (Exception excep)
                    {
                        LastDsErrorMsg = excep.Message;
                        LastDsErrorNum = -1;
                        return -1;
                    }
                    hRes = FilterGraph2.AddFilter(SourceFilter, Camera.Name);
                    if (hResultError(hRes)) return hRes;
				}

                TempFilter = true;
            }


            hRes = DsFindPin(SourceFilter, ref SrcCapPin, PinCategory.Capture);
            if (hResultError(hRes)) return hRes;
            IAMStreamConfig StreamConfig1 = (IAMStreamConfig)SrcCapPin;
            VideoInfoHeader VIHeader = new VideoInfoHeader();
            AMMediaType mt = null;
            hRes = StreamConfig1.GetFormat(out mt);
            if (hResultError(hRes)) return hRes;
            Marshal.PtrToStructure(mt.formatPtr, VIHeader);
            StreamCaps.Mediasubtype = mt.subType;
            StreamCaps.FrameRate = (int)((float)10000000 / VIHeader.AvgTimePerFrame);
            StreamCaps.Resolution = new System.Drawing.Point(VIHeader.BmiHeader.Width, VIHeader.BmiHeader.Height);

            Marshal.FreeCoTaskMem(mt.formatPtr);

            if (TempFilter)
            {
                FilterGraph2.Abort();
                FilterGraph2.RemoveFilter(SourceFilter);
                Marshal.ReleaseComObject(SourceFilter);
                SourceFilter = null;
                Marshal.ReleaseComObject(CaptureGraph);
                CaptureGraph = null;
                Marshal.ReleaseComObject(FilterGraph2);
                FilterGraph2 = null;
            }

            return 0;
        }

        private int SetCamStreamCaps(StreamCaps StreamCaps)
        {
            int hRes = 0;

            bool TempFilter = false;

            if (SourceFilter == null)
            {
                FilterGraph2 = (IFilterGraph2)new FilterGraph();
                CaptureGraph = (ICaptureGraphBuilder2)new CaptureGraphBuilder2();
                hRes = CaptureGraph.SetFiltergraph(FilterGraph2);
                if (hResultError(hRes)) return hRes;

                object source;
                Guid SourceGuid = typeof(IBaseFilter).GUID;
                try
                {
                    Camera.Mon.BindToObject(null, null, ref SourceGuid, out source);
                    SourceFilter = (IBaseFilter)source;
                }
                catch (Exception excep)
                {
                    LastDsErrorMsg = excep.Message;
                    LastDsErrorNum = -1;
                    return -1;
                }
                hRes = FilterGraph2.AddFilter(SourceFilter, Camera.Name);
                if (hResultError(hRes)) return hRes;

                TempFilter = true;
            }

            IPin SrcCapPin = null;
            hRes = DsFindPin(SourceFilter, ref SrcCapPin, PinCategory.Capture);
            if (hResultError(hRes)) return hRes;
            IAMStreamConfig StreamConfig1 = (IAMStreamConfig)SrcCapPin;
            VideoInfoHeader VIHeader = new VideoInfoHeader();
            AMMediaType mt = null;
            hRes = StreamConfig1.GetFormat(out mt);
            if (hResultError(hRes)) return hRes;

            if (StreamCaps!=null)
			{
                Marshal.PtrToStructure(mt.formatPtr, VIHeader);
                VIHeader.BmiHeader.Width = Convert.ToInt32(StreamCaps.Resolution.X);
                VIHeader.BmiHeader.Height = Convert.ToInt32(StreamCaps.Resolution.Y);
                Marshal.StructureToPtr(VIHeader, mt.formatPtr, true);
                mt.subType = StreamCaps.Mediasubtype;
                hRes = StreamConfig1.SetFormat(mt);
                if (hResultError(hRes)) return hRes;
                Marshal.FreeCoTaskMem(mt.formatPtr);
			}

            if (TempFilter)
            {
                FilterGraph2.Abort();
                FilterGraph2.RemoveFilter(SourceFilter);
                Marshal.ReleaseComObject(SourceFilter);
                SourceFilter = null;
                Marshal.ReleaseComObject(CaptureGraph);
                CaptureGraph = null;
                Marshal.ReleaseComObject(FilterGraph2);
                FilterGraph2 = null;
            }

            return 0;
        }

        private int GetCamCapSettingsList(out List<RxCamSettings.CamCapStruct> CamCapsList)
        {
            CamCapsList = new List<RxCamSettings.CamCapStruct>();
            int hRes = 0;
            IPin SrcCapPin = null;
            bool TempFilter = false;

            if (SourceFilter == null)
            {
                FilterGraph2 = (IFilterGraph2)new FilterGraph();
                CaptureGraph = (ICaptureGraphBuilder2)new CaptureGraphBuilder2();
                hRes = CaptureGraph.SetFiltergraph(FilterGraph2);
                if (hResultError(hRes)) return hRes;

                object source;
                Guid SourceGuid = typeof(IBaseFilter).GUID;
                try
                {
                    Camera.Mon.BindToObject(null, null, ref SourceGuid, out source);
                    SourceFilter = (IBaseFilter)source;
                }
                catch (Exception excep)
                {
                    LastDsErrorMsg = excep.Message;
                    LastDsErrorNum = -1;
                    return -1;
                }
                hRes = FilterGraph2.AddFilter(SourceFilter, Camera.Name);
                if (hResultError(hRes)) return hRes;

                TempFilter = true;
            }

            IAMCameraControl CamCtrl = SourceFilter as IAMCameraControl;
            foreach (CameraControlProperty Property in Enum.GetValues(typeof(CameraControlProperty)))
            {
                RxCamSettings.CamCapStruct CamCap = new RxCamSettings.CamCapStruct();
                CamCap.Name = Property.ToString();
                CamCap.Available = false;
                CameraControlFlags ccf;
                if (CamCtrl.GetRange(Property, out CamCap.Minimum, out CamCap.Maximum, out CamCap.Step, out CamCap.Default, out ccf) == 0)
                {
                    CamCap.Flags = ccf.ToString();
                    if (CamCtrl.Get(Property, out CamCap.Value, out ccf) != 0)
                        CamCap.Value = CamCap.Default;
                    CamCap.Flag = ccf.ToString();
                    CamCap.Available = true;
                    CamCapsList.Add(CamCap);
                }
            }

            if (TempFilter)
            {
                FilterGraph2.Abort();
                FilterGraph2.RemoveFilter(SourceFilter);
                Marshal.ReleaseComObject(SourceFilter);
                SourceFilter = null;
                Marshal.ReleaseComObject(CaptureGraph);
                CaptureGraph = null;
                Marshal.ReleaseComObject(FilterGraph2);
                FilterGraph2 = null;
            }

            return CamCapsList.Count;
        }

        private int SetCamCapSettingsList(List<RxCamSettings.CamCapStruct> CamCapsList)
        {
            for (int i = 0; i < CamCapsList.Count; i++)
            {
                int RetVal = SetCamCapSetting(CamCapsList[i]);
                if (RetVal != 0) return RetVal;
            }

            return 0;
        }

        private int SetCamCapSetting(RxCamSettings.CamCapStruct CamCap)
        {
            IAMCameraControl CamCtrl = SourceFilter as IAMCameraControl;
            CameraControlProperty Property = (CameraControlProperty)Enum.Parse(typeof(CameraControlProperty), CamCap.Name, true);
            if (Enum.IsDefined(typeof(CameraControlProperty), Property))
            {
                CameraControlFlags Flag = (CameraControlFlags)Enum.Parse(typeof(CameraControlFlags), CamCap.Flag, true);
                if (Enum.IsDefined(typeof(CameraControlFlags), Flag))
                {
                    int hRes = CamCtrl.Set(Property, CamCap.Value, Flag);
                    if (hResultError(hRes)) return hRes;
                    return hRes;
                }
            }
            return -1;
        }

        private int GetCamVideoProcSettingsList(out List<RxCamSettings.CamVideoProcStruct> CamVideoProcList)
        {
            CamVideoProcList = new List<RxCamSettings.CamVideoProcStruct>();
            int hRes = 0;
            IPin SrcCapPin = null;
            bool TempFilter = false;

            if (SourceFilter == null)
            {
                FilterGraph2 = (IFilterGraph2)new FilterGraph();
                CaptureGraph = (ICaptureGraphBuilder2)new CaptureGraphBuilder2();
                hRes = CaptureGraph.SetFiltergraph(FilterGraph2);
                if (hResultError(hRes)) return hRes;

                object source;
                Guid SourceGuid = typeof(IBaseFilter).GUID;
                try
                {
                    Camera.Mon.BindToObject(null, null, ref SourceGuid, out source);
                    SourceFilter = (IBaseFilter)source;
                }
                catch (Exception excep)
                {
                    LastDsErrorMsg = excep.Message;
                    LastDsErrorNum = -1;
                    return -1;
                }
                hRes = FilterGraph2.AddFilter(SourceFilter, Camera.Name);
                if (hResultError(hRes)) return hRes;

                TempFilter = true;
            }

            IAMVideoProcAmp Vproc = SourceFilter as IAMVideoProcAmp;
            foreach (VideoProcAmpProperty Property in Enum.GetValues(typeof(VideoProcAmpProperty)))
            {
                RxCamSettings.CamVideoProcStruct CamVideoProc = new RxCamSettings.CamVideoProcStruct();
                CamVideoProc.Name = Property.ToString();
                CamVideoProc.Available = false;
                VideoProcAmpFlags vpaf;
                if (Vproc.GetRange(Property, out CamVideoProc.Minimum, out CamVideoProc.Maximum, out CamVideoProc.Step, out CamVideoProc.Default, out vpaf) == 0)
                {
                    CamVideoProc.Flags = vpaf.ToString();
                    if (Vproc.Get(Property, out CamVideoProc.Value, out vpaf) != 0)
                        CamVideoProc.Value = CamVideoProc.Default;
                    CamVideoProc.Flag = vpaf.ToString();
                    CamVideoProc.Available = true;
                    CamVideoProcList.Add(CamVideoProc);
                }
            }

            if (TempFilter)
            {
                FilterGraph2.Abort();
                FilterGraph2.RemoveFilter(SourceFilter);
                Marshal.ReleaseComObject(SourceFilter);
                SourceFilter = null;
                Marshal.ReleaseComObject(CaptureGraph);
                CaptureGraph = null;
                Marshal.ReleaseComObject(FilterGraph2);
                FilterGraph2 = null;
            }

            return CamVideoProcList.Count;
        }

        private int SetCamVideoProcSettingsList(List<RxCamSettings.CamVideoProcStruct> CamVideoProcList)
        {
            for (int i = 0; i < CamVideoProcList.Count; i++)
            {
                int RetVal = SetCamVideoProc(CamVideoProcList[i]);
                if (RetVal != 0) return RetVal;
            }

            return 0;
        }

        private int SetCamVideoProc(RxCamSettings.CamVideoProcStruct CamVideoProc)
        {
            IAMVideoProcAmp Vproc = SourceFilter as IAMVideoProcAmp;

            VideoProcAmpProperty Property = (VideoProcAmpProperty)Enum.Parse(typeof(VideoProcAmpProperty), CamVideoProc.Name, true);
            if (Enum.IsDefined(typeof(VideoProcAmpProperty), Property))
            {
                VideoProcAmpFlags Flag = (VideoProcAmpFlags)Enum.Parse(typeof(VideoProcAmpFlags), CamVideoProc.Flag, true);
                if (Enum.IsDefined(typeof(VideoProcAmpFlags), Flag))
                {
                    int hRes = Vproc.Set(Property, CamVideoProc.Value, Flag);
                    if (hResultError(hRes)) return hRes;
                    return hRes;
                }
            }
            return 0;
        }



        private void PropertyChange(object sender, PropertyChangedEventArgs e)
        {
            switch (e.PropertyName)
            {
                case "DeviceName":
                    if (!CameraRunning)
                        SelectCamera(_CamSettings._DeviceName);
                    break;

                case "StreamCaps":
                    SetCamStreamCaps(_CamSettings._StreamCaps);
                    break;

                case "CamCapsList":
                    if (CameraRunning)
                    {
                        SetCamCapSettingsList(_CamSettings._CamCapsList);
                        GetCamCapSettingsList(out _CamSettings._CamCapsList);
                    }
                    break;

                case "CamCaps":
                    if (CameraRunning)
                    {
                        SetCamCapSetting(_CamSettings._CamCaps);
                        GetCamCapSettingsList(out _CamSettings._CamCapsList);
                    }
                    break;

                case "CamVideoProcList":
                    if (CameraRunning)
                    {
                        SetCamVideoProcSettingsList(_CamSettings._CamVideoProcList);
                        GetCamVideoProcSettingsList(out _CamSettings._CamVideoProcList);
                    }
                    break;

                case "CamVideoProc":
                    if (CameraRunning)
                    {
                        SetCamVideoProc(_CamSettings._CamVideoProc);
                        GetCamVideoProcSettingsList(out _CamSettings._CamVideoProcList);
                    }
                    break;

                case "ShowProcessImage":
                    if (CameraRunning && AlignFilter != null && AlignFilterConnected)
                    {
                        AlignFilterIF.SetShowOriginalImage(!_CamSettings._ShowProcessImage);
                    }
                    break;

                case "OverlayText":
                    if (CameraRunning && AlignFilter != null && AlignFilterConnected)
                    {
                        AlignFilterIF.SetOverlayTxt(_CamSettings._OverlayText, 0xFFFFFF);
                    }
                    break;

                case "BinarizeMode":
                    if (CameraRunning && AlignFilter != null && AlignFilterConnected)
                    {
                        AlignFilterIF.SetBinarizeMode(_CamSettings._BinarizeMode);
                    }
                    break;

                case "Inverse":
                    if (CameraRunning && AlignFilter != null && AlignFilterConnected)
                    {
                        AlignFilterIF.SetInverse(_CamSettings._Inverse);
                    }
                    break;

                case "ShowHistogram":
                    if (CameraRunning && AlignFilter != null && AlignFilterConnected)
                    {
                        AlignFilterIF.ShowHistogram(_CamSettings._ShowHistogram);
                    }
                    break;

                case "Threshold":
                    if (CameraRunning && AlignFilter != null && AlignFilterConnected)
                    {
                        AlignFilterIF.SetThreshold(_CamSettings._Threshold);
                    }
                    break;

                case "TextColor":
                    if (CameraRunning && AlignFilter != null && AlignFilterConnected)
                    {
                        AlignFilterIF.SetBlobTextColor(_CamSettings._TextColor);
                    }
                    break;

                case "ValuesFont":
                    if (CameraRunning && AlignFilter != null && AlignFilterConnected)
                    {
                        IntPtr unmanaged_pOutFont = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(LOGFONT)));
                        Marshal.StructureToPtr(_CamSettings._ValuesFont, unmanaged_pOutFont, false);
                        AlignFilterIF.SetBlobFont(unmanaged_pOutFont);
                        Marshal.DestroyStructure(unmanaged_pOutFont, typeof(LOGFONT));
                        Marshal.FreeHGlobal(unmanaged_pOutFont);
                    }
                    break;

                case "Horizontal":
                    if (CameraRunning && AlignFilter != null && AlignFilterConnected)
                    {
                        AlignFilterIF.SetLinesHorizontal(_CamSettings._Horizontal);
                    }
                    break;

                case "DebugMode":
                    if (CameraRunning && AlignFilter != null && AlignFilterConnected)
                    {
                        AlignFilterIF.SetDebug(_CamSettings._DebugMode);
                    }
                    break;

                case "DspFrameTime":
                    if (CameraRunning && AlignFilter != null && AlignFilterConnected)
                    {
                        AlignFilterIF.SetFrameTiming(_CamSettings._DspFrameTime);
                    }
                    break;

                case "Measuremode":
                    AlignFilterIF.SetMeasureMode(_CamSettings._Measuremode);
                    break;

                case "Displaymode":
                    AlignFilterIF.SetDisplayMode(_CamSettings._Displaymode);
                    break;

            }
        }

        private void PropertyRead(object sender, PropertyChangedEventArgs e)
        {
            switch (e.PropertyName)
            {
                case "FilterVersion":
                {
                    _CamSettings._FilterVersion = GetFilterVersion();
                }
                break;

                case "StreamCapsList":
                {
                    GetCamStreamCapsList(out _CamSettings._StreamCapsList);
                }
                break;

                case "StreamCaps":
                {
                    GetCamStreamCaps(out _CamSettings._StreamCaps);
                }
                break;

                case "CamCapsList":
                {
                    GetCamCapSettingsList(out _CamSettings._CamCapsList);
                }
                break;

                case "CamVideoProcList":
                {
                    GetCamVideoProcSettingsList(out _CamSettings._CamVideoProcList);
                }
                break;


            }

        }

        #endregion
    }

    #region LogFont Class and Structures

    public enum FontWeight : int
        {
            FW_DONTCARE = 0,
            FW_THIN = 100,
            FW_EXTRALIGHT = 200,
            FW_LIGHT = 300,
            FW_NORMAL = 400,
            FW_MEDIUM = 500,
            FW_SEMIBOLD = 600,
            FW_BOLD = 700,
            FW_EXTRABOLD = 800,
            FW_HEAVY = 900,
        }
    public enum FontCharSet : byte
    {
        ANSI_CHARSET = 0,
        DEFAULT_CHARSET = 1,
        SYMBOL_CHARSET = 2,
        SHIFTJIS_CHARSET = 128,
        HANGEUL_CHARSET = 129,
        HANGUL_CHARSET = 129,
        GB2312_CHARSET = 134,
        CHINESEBIG5_CHARSET = 136,
        OEM_CHARSET = 255,
        JOHAB_CHARSET = 130,
        HEBREW_CHARSET = 177,
        ARABIC_CHARSET = 178,
        GREEK_CHARSET = 161,
        TURKISH_CHARSET = 162,
        VIETNAMESE_CHARSET = 163,
        THAI_CHARSET = 222,
        EASTEUROPE_CHARSET = 238,
        RUSSIAN_CHARSET = 204,
        MAC_CHARSET = 77,
        BALTIC_CHARSET = 186,
    }
    public enum FontPrecision : byte
    {
        OUT_DEFAULT_PRECIS = 0,
        OUT_STRING_PRECIS = 1,
        OUT_CHARACTER_PRECIS = 2,
        OUT_STROKE_PRECIS = 3,
        OUT_TT_PRECIS = 4,
        OUT_DEVICE_PRECIS = 5,
        OUT_RASTER_PRECIS = 6,
        OUT_TT_ONLY_PRECIS = 7,
        OUT_OUTLINE_PRECIS = 8,
        OUT_SCREEN_OUTLINE_PRECIS = 9,
        OUT_PS_ONLY_PRECIS = 10,
    }
    public enum FontClipPrecision : byte
    {
        CLIP_DEFAULT_PRECIS = 0,
        CLIP_CHARACTER_PRECIS = 1,
        CLIP_STROKE_PRECIS = 2,
        CLIP_MASK = 0xf,
        CLIP_LH_ANGLES = (1 << 4),
        CLIP_TT_ALWAYS = (2 << 4),
        CLIP_DFA_DISABLE = (4 << 4),
        CLIP_EMBEDDED = (8 << 4),
    }
    public enum FontQuality : byte
    {
        DEFAULT_QUALITY = 0,
        DRAFT_QUALITY = 1,
        PROOF_QUALITY = 2,
        NONANTIALIASED_QUALITY = 3,
        ANTIALIASED_QUALITY = 4,
        CLEARTYPE_QUALITY = 5,
        CLEARTYPE_NATURAL_QUALITY = 6,
    }
    [Flags]
    public enum FontPitchAndFamily : byte
    {
        DEFAULT_PITCH = 0,
        FIXED_PITCH = 1,
        VARIABLE_PITCH = 2,
        FF_DONTCARE = (0 << 4),
        FF_ROMAN = (1 << 4),
        FF_SWISS = (2 << 4),
        FF_MODERN = (3 << 4),
        FF_SCRIPT = (4 << 4),
        FF_DECORATIVE = (5 << 4),
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
    public class LOGFONT
    {
        public uint lfHeight;
        public uint lfWidth;
        public uint lfEscapement;
        public uint lfOrientation;
        public FontWeight lfWeight;
        [MarshalAs(UnmanagedType.U1)]
        public bool lfItalic;
        [MarshalAs(UnmanagedType.U1)]
        public bool lfUnderline;
        [MarshalAs(UnmanagedType.U1)]
        public bool lfStrikeOut;
        public FontCharSet lfCharSet;
        public FontPrecision lfOutPrecision;
        public FontClipPrecision lfClipPrecision;
        public FontQuality lfQuality;
        public FontPitchAndFamily lfPitchAndFamily;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string lfFaceName;

        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();
            sb.Append("LOGFONT\n");
            sb.AppendFormat("   lfHeight: {0}\n", lfHeight);
            sb.AppendFormat("   lfWidth: {0}\n", lfWidth);
            sb.AppendFormat("   lfEscapement: {0}\n", lfEscapement);
            sb.AppendFormat("   lfOrientation: {0}\n", lfOrientation);
            sb.AppendFormat("   lfWeight: {0}\n", lfWeight);
            sb.AppendFormat("   lfItalic: {0}\n", lfItalic);
            sb.AppendFormat("   lfUnderline: {0}\n", lfUnderline);
            sb.AppendFormat("   lfStrikeOut: {0}\n", lfStrikeOut);
            sb.AppendFormat("   lfCharSet: {0}\n", lfCharSet);
            sb.AppendFormat("   lfOutPrecision: {0}\n", lfOutPrecision);
            sb.AppendFormat("   lfClipPrecision: {0}\n", lfClipPrecision);
            sb.AppendFormat("   lfQuality: {0}\n", lfQuality);
            sb.AppendFormat("   lfPitchAndFamily: {0}\n", lfPitchAndFamily);
            sb.AppendFormat("   lfFaceName: {0}\n", lfFaceName);

            return sb.ToString();
        }
    }

    #endregion

}
