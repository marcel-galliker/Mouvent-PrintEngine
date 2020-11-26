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
using System.Runtime.InteropServices;

namespace rx_CamLib
{
    public class RxAlignFilter : RxBindable
    {
		private IFrx_AlignFilter _AlignFilter;

		public RxAlignFilter(object filter)
		{
			_AlignFilter = (IFrx_AlignFilter)filter;
		}

		public IFrx_AlignFilter Handle 
		{ 
			get {return _AlignFilter; } 
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

		private LOGFONT GetFilterValuesFont()
        {
            UInt32 LogFontSize = 0;
            _AlignFilter.GetBlobFont(IntPtr.Zero, out LogFontSize);
            IntPtr unmanaged_pInFont = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(LOGFONT)) * (int)LogFontSize);
            _AlignFilter.GetBlobFont(unmanaged_pInFont, out LogFontSize);
            LOGFONT ValuesFont = new LOGFONT();
            ValuesFont = (LOGFONT)Marshal.PtrToStructure(unmanaged_pInFont, typeof(LOGFONT));
            Marshal.DestroyStructure(unmanaged_pInFont, typeof(LOGFONT));
            Marshal.FreeHGlobal(unmanaged_pInFont);
            return ValuesFont;
        }

        private string GetFilterVersion()
        {
            if (_AlignFilter==null) return null;

            UInt32 StrLength = 0;
            string FilterVersion;
            _AlignFilter.GetVersion(IntPtr.Zero, out StrLength);
            IntPtr unmanaged_pIn = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(char)) * 256);
            _AlignFilter.GetVersion(unmanaged_pIn, out StrLength);
            FilterVersion = Marshal.PtrToStringAuto(unmanaged_pIn);
            Marshal.FreeHGlobal(unmanaged_pIn);
            return FilterVersion;
        }

        private string GetFilterGraphDeviceName()
        {
            //Graphics Device Name
            IntPtr unmanaged_pIn = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(char)) * 256);
            _AlignFilter.GetDeviceName(unmanaged_pIn);
            string DeviceName = Marshal.PtrToStringAuto(unmanaged_pIn);
            Marshal.FreeHGlobal(unmanaged_pIn);
            return DeviceName;
        }



	}
}
