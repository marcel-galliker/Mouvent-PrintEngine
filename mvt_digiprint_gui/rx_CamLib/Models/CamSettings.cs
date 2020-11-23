using RX_Common;
using System;
using System.Collections.Generic;
using System.Text;

namespace rx_CamLib.Models
{

	[Flags]
	public enum AutoFlag
	{
		None = 0,
		Auto = 1,
		Manual = 2,
	}

	public class CamSettings : RxBindable
	{
		//--- Property Name ---------------------------------------
		private string _Name;
		public string Name
		{
			get { return _Name; }
			set { SetProperty(ref _Name,value); }
		}

		//--- Property DistToStop ---------------------------------------
		private double _DistToStop;
		public double DistToStop
		{
			get { return _DistToStop; }
			set { SetProperty(ref _DistToStop,value); }
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
