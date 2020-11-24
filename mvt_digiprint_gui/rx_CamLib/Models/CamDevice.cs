using DirectShowLib;
using RX_Common;
using System;
using System.Collections.Generic;
using System.Text;

namespace rx_CamLib.Models
{
    public class CamDevice : RxBindable
    {
		private static IAMVideoProcAmp		_Device = null;
		private List<CamDeviceProperty>		_Properties = null;

		public CamDevice(IBaseFilter device)
		{
			_Device     = device as IAMVideoProcAmp;
		}

		//--- _InitDeviceProperties ---------------------------------
        private List<CamDeviceProperty> _InitDeviceProperties()
        {
            List<CamDeviceProperty> list = new List<CamDeviceProperty>();
            foreach (VideoProcAmpProperty property in Enum.GetValues(typeof(VideoProcAmpProperty)))
            {
                CamDeviceProperty prop=new CamDeviceProperty(property);
                if (prop.Available) list.Add(prop);
            }
            return list;
        }

        //--- GetProperties ------------------------------------------------
        public List<CamDeviceProperty> GetProperties()
		{
			if (_Properties==null) _Properties = _InitDeviceProperties();
            return _Properties;
		}

        //--- GetRange ------------------------------------------------------------------------
        public int GetRange(VideoProcAmpProperty property, out int min, out int max, out int step, out int def, out VideoProcAmpFlags flag)
		{
            return _Device.GetRange(property, out min, out max, out step, out def, out flag);
		}

        //--- GetProperty -------------------------------------------------------------
        public int GetProperty(VideoProcAmpProperty property, out int value, out VideoProcAmpFlags flag)
		{
            return _Device.Get(property, out value, out flag);
		}

        //--- SetProperty --------------------------------------------------------
        public void SetProperty(VideoProcAmpProperty property, int value, VideoProcAmpFlags flag)
		{
            _Device.Set(property, value, flag);
		}
    }
}
