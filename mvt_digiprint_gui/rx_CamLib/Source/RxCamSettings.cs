using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.CompilerServices;
using rx_CamLib.Models;

namespace rx_CamLib
{
    public class RxCamSettings : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        public event PropertyChangedEventHandler PropertyReading;

        internal string _DeviceName;
        /// <summary>
        /// DeviceName as found in Device Manager
        /// </summary>
        public string DeviceName
        {
            get { return _DeviceName;}
            set
            {
                _DeviceName = value;
                OnPropertyChanged();
            }
        }

        internal bool _CameraRunning;
        public bool CameraRunning
        {
            get { return _CameraRunning; }
        }

        internal List<StreamCaps> _StreamCapsList;
        /// <summary>
        /// Returns a List of all stream capabilities (Stream Type, Resolution, FrameRate)
        /// </summary>
        public List<StreamCaps> StreamCapsList
        {
            get
            {
                if(!_CameraRunning && _DeviceName != null)
                {
                    OnPropertyReading();
                }
                return _StreamCapsList; 
            }
        }
        internal StreamCaps _StreamCaps;
        /// <summary>
        /// Gets or sets the currently used stream settings (FrameRate is read only)
        /// </summary>
        public StreamCaps StreamCaps
        {
            get
            {
                if(_CameraRunning || _StreamCaps==null) // _StreamCaps.Mediasubtype == Guid.Parse("00000000-0000-0000-0000-000000000000"))
                    OnPropertyReading();
                return _StreamCaps;
            }

            set
            {
                _StreamCaps = value;
                OnPropertyChanged();
            }
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
            public string Flags;
            public string Flag;
        }
        internal List<CamCapStruct> _CamCapsList;
        /// <summary>
        /// Gets or sets all camera functions like exposure, aperture etc. (availability depends on camera model)
        /// </summary>
        public List<CamCapStruct> CamCapsList
        {
            get
            {
                if (!_CameraRunning && _DeviceName != null)
                {
                    OnPropertyReading();
                }
                return _CamCapsList; 
            }
            set
            {
                _CamCapsList = value;
                OnPropertyChanged();
            }
        }
        internal CamCapStruct _CamCaps;
        /// <summary>
        /// Gets or sets a single camera function like exposure or aperture etc. (availability depends on camera model)
        /// </summary>
        public CamCapStruct CamCaps
        {
            set
            {
                _CamCaps = value;
                OnPropertyChanged();
            }
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
            public string Flags;
            public string Flag;
        }
        internal List<CamVideoProcStruct> _CamVideoProcList;
        /// <summary>
        /// Gets or sets all video process settings like contrast, brighness etc. (availability depends on camera model)
        /// </summary>
        public List<CamVideoProcStruct> CamVideoProcList
        {
            get
            {
                if (!_CameraRunning && _DeviceName != null)
                {
                    OnPropertyReading();
                }
                return _CamVideoProcList;
            }
            set
            {
                _CamVideoProcList = value;
                OnPropertyChanged();
            }
        }
        internal CamVideoProcStruct _CamVideoProc;
        /// <summary>
        /// Gets or sets a single video process setting  like contrast or brighness etc. (availability depends on camera model)
        /// </summary>
        public CamVideoProcStruct CamVideoProc
        {
            set
            {
                _CamVideoProc = value;
                OnPropertyChanged();
            }
        }

        internal string _FilterVersion;
        /// <summary>
        /// Version of rx_AlignFilter
        /// </summary>
        public string FilterVersion
        {
            get
            {
                if (_FilterVersion == null)
                    OnPropertyReading();
                return _FilterVersion;
            }
        }

        internal string _GraphicDevice;
        /// <summary>
        /// Graphic processor used by rx_AlignFilter
        /// </summary>
        public string GraphicDevice
        {
            get
            {
                if (!_CameraRunning)
                    return "none";
                return _GraphicDevice;
            }
        }

        internal string _OverlayText;
        /// <summary>
        /// Overlay a text
        /// </summary>
        public string OverlayText
        {
            set
            {
                _OverlayText = value;
                OnPropertyChanged(); 
            }
        }

        internal bool _ShowProcessImage;
        /// <summary>
        /// Show original or binarized image
        /// </summary>
        public bool ShowProcessImage
        {
            get { return _ShowProcessImage; }
            set
            {
                _ShowProcessImage = value;
                OnPropertyChanged();
            }
        }

        internal UInt32 _BinarizeMode;
        /// <summary>
        /// BinarisationMode 0: off, 1: Manual threshold, 1: Auto (color independent), 2:Adaptive (multicolor pattern)
        /// </summary>
        public UInt32 BinarizeMode
        {
            get { return _BinarizeMode; }
            set
            {
                _BinarizeMode = value;
                OnPropertyChanged();
            }
        }

        internal bool _ShowHistogram;
        /// <summary>
        /// Show binarization histogram (for debugging)
        /// </summary>
        public bool ShowHistogram
        {
            get { return _ShowHistogram; }
            set
            {
                _ShowHistogram = value;
                OnPropertyChanged();
            }
        }

        internal UInt32 _Threshold;
        /// <summary>
        /// Get current binarization threshod or set manual threshold
        /// </summary>
        public UInt32 Threshold
        {
            get { return _Threshold; }
            set
            {
                _Threshold = value;
                OnPropertyChanged();
            }
        }

        internal LOGFONT _ValuesFont;
        /// <summary>
        /// Font to display values and text overlay
        /// </summary>
        public LOGFONT ValuesFont
        {
            get { return _ValuesFont; }
            set
            {
                _ValuesFont = value;
                OnPropertyChanged();
            }
        }

        internal UInt32 _TextColor;
        /// <summary>
        /// Color for values and text overlay
        /// </summary>
        public UInt32 TextColor
        {
            get { return _TextColor; }
            set
            {
                _TextColor = value;
                OnPropertyChanged();
            }
        }

        internal bool _Inverse;
        /// <summary>
        /// inverse binarisation for white on black
        /// </summary>
        public bool Inverse
        {
            get { return _Inverse; }
            set
            {
                _Inverse = value;
                OnPropertyChanged();
            }
        }

        internal bool _Horizontal;
        /// <summary>
        /// Check for horizontal lines, default = vertical
        /// </summary>
        public bool Horizontal
        {
            get { return _Horizontal; }
            set
            {
                _Horizontal = value;
                OnPropertyChanged();
            }
        }

        internal bool _DebugMode;
        /// <summary>
        /// Open rx_AlignFilter debug window
        /// </summary>
        public bool DebugMode
        {
            get { return _DebugMode; }
            set
            { 
                _DebugMode = value;
                OnPropertyChanged();
            }
        }

        internal bool _DspFrameTime;
        /// <summary>
        /// Display frame timing (debug)
        /// </summary>
        public bool DspFrameTime
        {
            get { return _DspFrameTime; }
            set
            {
                _DspFrameTime = value;
                OnPropertyChanged();
            }
        }

        internal UInt32 _Measuremode;
        /// <summary>
        /// Sets the measuring mode of rs_AlignFilter
        /// </summary>
        public UInt32 Measuremode
        {
            get { return _Measuremode; }
            set
            {
                _Measuremode = value;
                OnPropertyChanged();
            }
        }

        internal UInt32 _Displaymode;
        /// <summary>
        /// Sets the measuring mode of rs_AlignFilter
        /// </summary>
        public UInt32 Displaymode
        {
            get { return _Displaymode; }
            set
            {
                _Displaymode = value;
                OnPropertyChanged();
            }
        }


        protected void OnPropertyChanged([CallerMemberName] string name = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
        }
        protected void OnPropertyReading([CallerMemberName] string name = null)
        {
            PropertyReading?.Invoke(this, new PropertyChangedEventArgs(name));
        }


    }
}
