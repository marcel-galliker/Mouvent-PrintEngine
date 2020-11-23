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
using static rx_CamLib.RxCamSettings;

namespace rx_CamLib
{
    public class RxCam
    {
        #region Definitions

        public enum ENCamResult
		{
            OK                      =  0,
            Error                   = -1,
        
            Cam_notDetected         = -2,
            Cam_notFound            = -3,
            Cam_alreadyRunning      = -4,
            Cam_notSelected         = -5,
            Cam_notRunning,

            Filter_NotRegistered    = -6,

            Filter_AlreadyUsed      = -2147467259,
		};

        private DsDevice[] DeviceList = null;
        private bool CameraRunning = false;
        private bool GraphStarting = false;

        private string LastDsErrorMsg = "";
        private ENCamResult LastDsErrorNum = 0;

        private DsDevice Camera = null;
        private IFilterGraph2 FilterGraph2 = null;
        private IFrx_AlignFilter _RxAlignFilter = null;
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

        public RxCam()
        {
        }

        ~RxCam()
        {
            if (CameraRunning) StopGraph();
        }


        #region Public Interface

        public delegate void CameraCallBack(string CallbackData);
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

        private RxCamSettings _CamSettings = new RxCamSettings();
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
                    _CamSettings._DeviceName = Camera.Name;
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
                GraphStarting = false;
                return result;
            }

            CameraRunning = true;
            CamCallBack?.Invoke("Camera started");
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
            if (!CameraRunning)
            {
                LastDsErrorMsg = "Camera not running";
                return LastDsErrorNum = ENCamResult.Cam_notRunning;
            }
            if (!StopGraph()) return LastDsErrorNum=ENCamResult.Error;
            
            CamCallBack?.Invoke("Camera stopped");
            LastDsErrorMsg = "";
            return LastDsErrorNum = ENCamResult.OK;
        }

        /// <summary>
        /// Get Last DirectShow Error
        /// </summary>
        /// <param name="Message">Error Message</param>
        /// <returns>Error Number</returns>
        public ENCamResult GetLastDsError(out string Message)
        {
            Message = LastDsErrorMsg;
            if (LastDsErrorNum == ENCamResult.Filter_AlreadyUsed)
                Message = "Resource used by this filter may already be in use";
            return LastDsErrorNum;
        }

        #endregion


        #region Internal

        private ENCamResult BuildGraph(IntPtr hwnd)
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
            if (hResultError(hResult)) return ENCamResult.Error;
            VMR9 = (IBaseFilter)new VideoMixingRenderer9();
            if (!ConfigVMR9(hwnd)) return ENCamResult.Error;
            hResult = FilterGraph2.AddFilter(VMR9, "VMR 9");
            if (hResultError(hResult)) return ENCamResult.Error;
            //Insert Camera
            object source;
            Guid SourceGuid = typeof(IBaseFilter).GUID;
            try
            {
                Camera.Mon.BindToObject(null, null, ref SourceGuid, out source);
                SourceFilter = (IBaseFilter)source;
                CamDeviceSettings.SetDevice(SourceFilter);
            }
            catch (Exception excep)
            {
                LastDsErrorMsg = excep.Message;
                return LastDsErrorNum = ENCamResult.Error;
            }
            hResult = FilterGraph2.AddFilter(SourceFilter, Camera.Name);
            if (hResultError(hResult)) return LastDsErrorNum = ENCamResult.Error;

            hResult = DsFindPin(SourceFilter, ref SrcCapPin, PinCategory.Capture);
            if (hResultError(hResult)) return LastDsErrorNum = ENCamResult.Error;

            //    SetCamStreamCaps(_CamSettings._StreamCaps);

            //Read current Camera settings
//            GetCamStreamCapsList(out _CamSettings._StreamCapsList);
//            GetCamCapSettingsList(out _CamSettings._CamCapsList);
//            GetCamVideoProcSettingsList(out _CamSettings._CamVideoProcList);

            //Insert Align Filter
            Guid AlignGuid = new Guid("148BC1EB-2C83-418E-B9CD-E1F5BC9D1E38");  //rx_AlignFilter
            //Guid AlignGuid = new Guid("3C84E851-7D06-434F-81E1-5E68F0306E8B"); //Bieler_dsAlign
            Type comType = null;
            comType = Type.GetTypeFromCLSID(AlignGuid);
			try 
            { 
                _RxAlignFilter = (IFrx_AlignFilter)Activator.CreateInstance(comType);
                Settings.SetAlignFilter(_RxAlignFilter);
            }
            catch(Exception e)
			{
                return LastDsErrorNum = ENCamResult.Filter_NotRegistered;
			}
            hResult = FilterGraph2.AddFilter(_RxAlignFilter, "Alignment");
            if (hResultError(hResult)) return LastDsErrorNum = ENCamResult.Error;

            //Connect Camera to Align
            IPin AlignInPin = null;
            hResult = DsFindPin(_RxAlignFilter, ref AlignInPin, "Input");
            if (hResultError(hResult)) return LastDsErrorNum = ENCamResult.Error;
            hResult = FilterGraph2.Connect(SrcCapPin, AlignInPin);
            if (hResultError(hResult)) return LastDsErrorNum = ENCamResult.Error;

            //Render Align Output Pin
            IPin AlignOutPin = null;
            hResult = DsFindPin(_RxAlignFilter, ref AlignOutPin, "Output");
            if (hResultError(hResult)) return LastDsErrorNum = ENCamResult.Error;
            hResult = FilterGraph2.Render(AlignOutPin);
            if (hResultError(hResult)) return LastDsErrorNum = ENCamResult.Error;

            //Start Display
            FilterState pFState = FilterState.Stopped;
            hResult = MediaControl.Run();
            if (hResultError(hResult)) return LastDsErrorNum = ENCamResult.Error;
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
                return LastDsErrorNum = ENCamResult.Error;
            }


            //Read current stream settings
//            hResult = GetCamStreamCaps(out _CamSettings._StreamCaps);

            //Set Camera settings
//            SetCamCapSettingsList(_CamSettings._CamCapsList);
//            SetCamVideoProcSettingsList(_CamSettings._CamVideoProcList);


            GetAllFilterSettings();

            GraphStarting = false;
            return LastDsErrorNum = ENCamResult.OK;
        }

        private bool StopGraph()
        {
            int hResult;

            _RxAlignFilter.SetFrameTiming(false);
            _RxAlignFilter.SetDebug(false);
            _RxAlignFilter.SetHostPointer(IntPtr.Zero);
            _RxAlignFilter.ShowHistogram(false);
            _RxAlignFilter.SetOverlayTxt("", 0);
            _RxAlignFilter.SetShowOriginalImage(true);
            _RxAlignFilter.SetDisplayMode(0);
            _RxAlignFilter.SetMeasureMode(0);
            _RxAlignFilter.SetBinarizeMode(0);

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

            if (_RxAlignFilter != null)
            {
                hResult = FilterGraph2.RemoveFilter(_RxAlignFilter);
                hResultError(hResult);
            }

            if (SourceFilter != null) Marshal.ReleaseComObject(SourceFilter);
            SourceFilter = null;

            if (CaptureGraph != null) Marshal.ReleaseComObject(CaptureGraph);
            CaptureGraph = null;
            if (MediaControl != null) Marshal.ReleaseComObject(MediaControl);
            MediaControl = null;

            if (FilterGraph2 != null) Marshal.ReleaseComObject(FilterGraph2);
            FilterGraph2 = null;

            CameraRunning = false;
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

        private LOGFONT GetFilterValuesFont()
        {
            UInt32 LogFontSize = 0;
            _RxAlignFilter.GetBlobFont(IntPtr.Zero, out LogFontSize);
            IntPtr unmanaged_pInFont = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(LOGFONT)) * (int)LogFontSize);
            _RxAlignFilter.GetBlobFont(unmanaged_pInFont, out LogFontSize);
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
            if (_RxAlignFilter != null)
            {
                _RxAlignFilter.GetVersion(IntPtr.Zero, out StrLength);
                IntPtr unmanaged_pIn = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(char)) * 256);
                _RxAlignFilter.GetVersion(unmanaged_pIn, out StrLength);
                FilterVersion = Marshal.PtrToStringAuto(unmanaged_pIn);
                Marshal.FreeHGlobal(unmanaged_pIn);
            }
            else
            {
                Guid AlignGuid = new Guid("148BC1EB-2C83-418E-B9CD-E1F5BC9D1E38");  //rx_AlignFilter
                Type comType = null;
                comType = Type.GetTypeFromCLSID(AlignGuid);
                IntPtr unmanaged_pIn = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(char)) * 256);
                _RxAlignFilter.GetVersion(unmanaged_pIn, out StrLength);
                FilterVersion = Marshal.PtrToStringAuto(unmanaged_pIn);
                Marshal.FreeHGlobal(unmanaged_pIn);
                _RxAlignFilter = null;
            }
            return FilterVersion;
        }

        private string GetFilterGraphDeviceName()
        {
            //Graphics Device Name
            IntPtr unmanaged_pIn = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(char)) * 256);
            _RxAlignFilter.GetDeviceName(unmanaged_pIn);
            string DeviceName = Marshal.PtrToStringAuto(unmanaged_pIn);
            Marshal.FreeHGlobal(unmanaged_pIn);
            return DeviceName;
        }

        //--- GetDeviceProperties ---------------------------------
        public List<CamDeviceSettings> GetDeviceProperties()
        {
            List<CamDeviceSettings> list = new List<CamDeviceSettings>();
            foreach (VideoProcAmpProperty property in Enum.GetValues(typeof(VideoProcAmpProperty)))
            {
                CamDeviceSettings prop=new CamDeviceSettings(property);
                if (prop.Available) list.Add(prop);
            }
            return list;
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

        /*
        private ENCamResult SetCamStreamCaps(StreamCaps StreamCaps)
        {
            int hRes = 0;

            if (SourceFilter == null) return null;

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

            return 0;
        }
        */

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

        /*
        private int GetCamVideoProcSettingsList(out List<CamDeviceSettings> CamVideoProcList)
        {
            CamVideoProcList = new List<CamDeviceSettings>();
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
                    return (int)(LastDsErrorNum = ENCamResult.Error);
                }
                hRes = FilterGraph2.AddFilter(SourceFilter, Camera.Name);
                if (hResultError(hRes)) return hRes;

                TempFilter = true;
            }

            IAMVideoProcAmp Vproc = SourceFilter as IAMVideoProcAmp;
            foreach (VideoProcAmpProperty Property in Enum.GetValues(typeof(VideoProcAmpProperty)))
            {
                CamDeviceSettings CamVideoProc = new CamDeviceSettings(Property);
                CamVideoProc.Available = false;
                VideoProcAmpFlags vpaf;
                if (Vproc.GetRange(Property, out int min, out int max, out int step, out int def, out vpaf) == 0)
                {
                    CamVideoProc.Minimum = min;
                    CamVideoProc.Maximum = max;
                    CamVideoProc.Step    = step;
                    CamVideoProc.Default = def;

                    CamVideoProc.Flags = (AutoFlag)vpaf;
                    if (Vproc.Get(Property, out int value, out vpaf) != 0)
                        CamVideoProc.Value = def;
                    else CamVideoProc.Value = value;

                    CamVideoProc.Flag = (AutoFlag)vpaf;
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
        */
        /*
        private int SetCamVideoProcSettingsList(List<CamDeviceSettings> CamVideoProcList)
        {
            for (int i = 0; i < CamVideoProcList.Count; i++)
            {
                int RetVal = SetCamVideoProc(CamVideoProcList[i]);
                if (RetVal != 0) return RetVal;
            }

            return 0;
        }
        */
        /*
        private int SetCamVideoProc(CamDeviceSettings CamVideoProc)
        {
            IAMVideoProcAmp Vproc = SourceFilter as IAMVideoProcAmp;

            VideoProcAmpProperty Property = (VideoProcAmpProperty)Enum.Parse(typeof(VideoProcAmpProperty), CamVideoProc.Name, true);
            if (Enum.IsDefined(typeof(VideoProcAmpProperty), Property))
            {
                VideoProcAmpFlags Flag = (VideoProcAmpFlags)CamVideoProc.Flag;
                if (Enum.IsDefined(typeof(VideoProcAmpFlags), Flag))
                {
                    int hRes = Vproc.Set(Property, CamVideoProc.Value, Flag);
                    if (hResultError(hRes)) return hRes;
                    return hRes;
                }
            }
            return 0;
        }
        */

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
