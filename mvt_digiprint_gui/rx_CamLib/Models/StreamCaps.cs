using System;
using System.Collections.Generic;
using System.Text;
using RX_Common;

namespace rx_CamLib.Models
{
	public class StreamCaps : RxBindable
	{
		//--- Property Mediasubtype ---------------------------------------
		private Guid _Mediasubtype;
		public Guid Mediasubtype
		{
			get { return _Mediasubtype; }
			set { SetProperty(ref _Mediasubtype,value); }
		}

		//--- Property FrameRate ---------------------------------------
		private int _FrameRate;
		public int FrameRate
		{
			get { return _FrameRate; }
			set { SetProperty(ref _FrameRate,value); }
		}

		//--- Property Resolution ---------------------------------------
		private System.Drawing.Point _Resolution;
		public System.Drawing.Point Resolution
		{
			get { return _Resolution; }
			set { SetProperty(ref _Resolution,value); }
		}

		//--- Equals --------------------------------------
		public bool Equals(StreamCaps item)
		{
			if (!item.Mediasubtype.Equals(Mediasubtype)) return false;
			if (item.FrameRate!=FrameRate) return false;
			if (item.Resolution.X != Resolution.X) return false;
			if (item.Resolution.Y != Resolution.Y) return false;
			return true;
		}
	}
}
