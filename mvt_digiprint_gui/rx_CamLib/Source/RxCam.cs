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
            if (CameraRunning) return StopGraph(); 
            return ENCamResult.OK;
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
            try
            {
                object source;
                Guid SourceGuid = typeof(IBaseFilter).GUID;
                Camera.Mon.BindToObject(null, null, ref SourceGuid, out source);
                SourceFilter = (IBaseFilter)source;
                CamGlobals.CamDevice = new CamDevice(SourceFilter);
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
                CamGlobals.AlignFilter = new RxAlignFilter(Activator.CreateInstance(comType));
            }
            catch(Exception e)
			{
                return LastDsErrorNum = ENCamResult.Filter_NotRegistered;
			}

            IFrx_AlignFilter halignFilter = CamGlobals.AlignFilter.Handle;

            hResult = FilterGraph2.AddFilter(halignFilter, "Alignment");
            if (hResultError(hResult)) return LastDsErrorNum = ENCamResult.Error;

            //Connect Camera to Align
            IPin AlignInPin = null;
            hResult = DsFindPin(halignFilter, ref AlignInPin, "Input");
            if (hResultError(hResult)) return LastDsErrorNum = ENCamResult.Error;
            hResult = FilterGraph2.Connect(SrcCapPin, AlignInPin);
            if (hResultError(hResult)) return LastDsErrorNum = ENCamResult.Error;

            //Render Align Output Pin
            IPin AlignOutPin = null;
            hResult = DsFindPin(halignFilter, ref AlignOutPin, "Output");
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

        private ENCamResult StopGraph()
        {
            int hResult;
            IFrx_AlignFilter halignFilter = CamGlobals.AlignFilter.Handle;

            halignFilter.SetFrameTiming(false);
            halignFilter.SetDebug(false);
            halignFilter.SetHostPointer(IntPtr.Zero);
            halignFilter.ShowHistogram(false);
            halignFilter.SetOverlayTxt("", 0);
            halignFilter.SetShowOriginalImage(true);
            halignFilter.SetDisplayMode(0);
            halignFilter.SetMeasureMode(0);
            halignFilter.SetBinarizeMode(0);

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

            if (halignFilter != null)
            {
                hResult = FilterGraph2.RemoveFilter(halignFilter);
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
            return ENCamResult.OK;
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
