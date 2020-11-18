using RX_Common;
using System;
using System.Collections.Generic;
using System.Text;

namespace rx_CamLib.Models
{
	public class CamSettings : RxBindable
	{
		//--- Property Name ---------------------------------------
		private string _Name;
		public string Name
		{
			get { return _Name; }
			set { SetProperty(ref _Name,value); }
		}

		//--- Property Threshold ---------------------------------------
		private int _Threshold;
		public int Threshold
		{
			get { return _Threshold; }
			set { SetProperty(ref _Threshold,value); }
		}

		//--- Property StreamCaps ---------------------------------------
		private StreamCaps _StreamCaps = new StreamCaps();
		public StreamCaps StreamCaps
		{
			get { return _StreamCaps; }
			set { SetProperty(ref _StreamCaps,value); }
		}
	}
}
