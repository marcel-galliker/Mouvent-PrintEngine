using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using DirectShowLib;


namespace rx_CamLib
{
    public class RxCam
    {
        #region Definitions

        private DsDevice[] DeviceList = null;
        private int SelectedCamera = -1;
        private bool CameraRunning = false;

        private string LastDsErrorMsg = "";
        private int LastDsErrorNum = 0;

        private DsDevice Camera = null;
        private IFilterGraph2 FilterGraph2 = null;
        private IMediaControl MediaControl = null;
        private ICaptureGraphBuilder2 CaptureGraph = null;
        private IBaseFilter SourceFilter = null;
        private IBaseFilter VMR9;
        private IVMRWindowlessControl9 WindowlessCtrl9 = null;

        private RxCamSettings   _Settings;

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

        #region Public Interface

        public delegate void CameraCallBack(string CallbackData);
        public event CameraCallBack CamCallBack = null;

        /// <summary>
        /// Get version of library
        /// </summary>
        /// <returns>Version</returns>
        public string GetLibVersion()
        {
            return typeof(RxCam).Assembly.GetName().Version.ToString();
        }

		//--- Property Settings ---------------------------------------
		public RxCamSettings Settings
		{
			get { return _Settings; }
			set { _Settings=value; }
		}

		/// <summary>
		/// Searches for all connected cameras
		/// </summary>
		/// <param name="CameraList">string[] that will receive all detected camera names</param>
		/// <returns>Number of detected cameras</returns>
		public int GetCameraList(out string[] CameraList)
        {
            CameraList = null;
            //Find connected cameras
            DeviceList = DsDevice.GetDevicesOfCat(FilterCategory.VideoInputDevice);
            if (DeviceList.Length < 1)
            {
                //No camera found
                CameraList = null;
                SelectedCamera = -1;
                return 0;
            }

            //one or more cameras detected
            CameraList = new string[DeviceList.Length];
            for (int i = 0; i < DeviceList.Length; i++)
            {
                CameraList[i] = DeviceList[i].Name;
            }
            return DeviceList.Length;
        }

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
        /// <param name="CameraNum">Camera number as found with GetCameraList()</param>
        /// <returns>
        /// <list type="table">
        /// <item><description> 0: for success or error code:</description></item>
        /// <item><description>-1: Camera already running</description></item>
        /// <item><description>-2: No cameras detected (yet)</description></item>
        /// <item><description>-3: Camera number not available</description></item></list>
        /// </returns>
        public int SelectCamera(int CameraNum)
        {
            if (CameraRunning)
            {
                LastDsErrorMsg = "Camera already running";
                LastDsErrorNum = -1;
                return -1;
            }
            if (DeviceList == null)
            {
                LastDsErrorMsg = "No Camera found";
                LastDsErrorNum = -2;
                SelectedCamera = -2;
                return -2;
            }
            if (DeviceList.Length < CameraNum || CameraNum < 0)
            {
                LastDsErrorMsg = "Selected Camera not found";
                LastDsErrorNum = -2;
                SelectedCamera = -2;
                return -3;
            }

            SelectedCamera = CameraNum;
            Camera = DeviceList[SelectedCamera];

            return 0;
        }
        public int SelectCamera(string name)
		{
            if (DeviceList==null) DeviceList = DsDevice.GetDevicesOfCat(FilterCategory.VideoInputDevice);
            for(int i=0; i<DeviceList.Length; i++)
			{
                if (DeviceList[i].Name.Equals(name)) return SelectCamera(i);
			}
            return -2;
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
            if (SelectedCamera < 0)
            {
                LastDsErrorMsg = "No camera selected";
                LastDsErrorNum = -2;
                return -2;
            }

            if (!BuildGraph(hwnd, PropDialog))
            {
                return -3;
            }

            CameraRunning = true;
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
            if (WindowlessCtrl9==null) return -1;
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
            return LastDsErrorNum;
        }

        #endregion

        #region Internal

        private bool BuildGraph(IntPtr hwnd, bool propDialog)
        {
            int hResult;

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
            //Show Camera Properties
            if (propDialog)
            {
                hResult = DisplayPropertyPages(SrcCapPin, "Capture");
                if (hResultError(hResult)) return false;
            }
            //Render Align Output Pin
            hResult = FilterGraph2.Render(SrcCapPin);
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

            return true;
        }

        private bool StopGraph()
        {
            int hResult;

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
            hResult = CaptureGraph.FindPin(Filter, PinDirection.Output, PinCategory.Capture, MediaType.Video, true, 0, out Pin);
            return hResult;  //Pin found or not
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

        #endregion

    }
}
