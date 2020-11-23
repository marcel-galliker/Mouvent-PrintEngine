using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.CompilerServices;
using rx_CamLib.Models;
using RX_Common;
using static rx_CamLib.RxCam;
using DirectShowLib;

namespace rx_CamLib
{
    public class RxCamSettings : RxBindable
    {
		private IFrx_AlignFilter _AlignFilter;

		public void SetAlignFilter(IBaseFilter alignFilter)
		{
			_AlignFilter = alignFilter as IFrx_AlignFilter;
		}

		//--- Property CamSettings ---------------------------------------
		private RxCamSettings _DeviceSettings;
		public RxCamSettings DeviceSettings    
		{
			get { return _DeviceSettings; }
			set { SetProperty(ref _DeviceSettings,value); }
		}

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

		//--- CameraRunning -----------------
        internal bool _CameraRunning;
        public bool CameraRunning
        {
            get { return _CameraRunning; }
        }

		//--- Property ShowProcessImage ---------------------------------------
		private bool _ShowProcessImage;
		public bool ShowProcessImage
		{
			get { return _ShowProcessImage; }
			set 
			{
				SetProperty(ref _ShowProcessImage,value);
				_AlignFilter?.SetShowOriginalImage(!_ShowProcessImage);
			}
		}

		//--- Property Inverse ---------------------------------------
		private bool _Inverse;
		public bool Inverse
		{
			get { return _Inverse; }
			set 
			{ 
				SetProperty(ref _Inverse,value); 
				_AlignFilter?.SetInverse(_Inverse);
			}
		}

		//--- Property BinarizeMode ---------------------------------------
		private uint _BinarizeMode;
		public uint BinarizeMode
		{
			get { return _BinarizeMode; }
			set 
			{ 
				SetProperty(ref _BinarizeMode,value); 
				if (_AlignFilter!=null) 
				{ 
					_AlignFilter.SetBinarizeMode(_BinarizeMode);
					_AlignFilter.ShowHistogram(_BinarizeMode>0);
				}
			}
		}

		//--- Property Threshold ---------------------------------------
		private uint? _Threshold;
		public uint Threshold
		{
			get 
			{ 
				if (_Threshold==null) _Threshold=_AlignFilter?.GetThreshold();
				if (_Threshold==null) return 0;
				return (uint)_Threshold; 
			}
			set 
			{ 
				SetProperty(ref _Threshold,value); 
				_AlignFilter?.SetThreshold((uint)_Threshold);
			}
		}

	}
}
