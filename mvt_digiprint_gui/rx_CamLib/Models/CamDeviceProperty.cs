using DirectShowLib;
using RX_Common;
using System;
using System.Collections.Generic;
using System.Text;

namespace rx_CamLib.Models
{
    public class CamDeviceProperty : RxBindable
    {
		private VideoProcAmpProperty	_Property;

		//--- Constructor ---------------------------------
		public CamDeviceProperty(VideoProcAmpProperty property)
		{
			_Name		= property.ToString();
			_Property	= property;
			_Available	= false;
			VideoProcAmpFlags flag;
			if (CamGlobals.CamDevice.GetRange(property, out int min, out int max, out int step, out int def, out flag) == 0)
			{
				_Minimum = min;
				_Maximum = max;
				_Step    = step;
				_Default = def;
				_AutoAvailable = (((VideoProcAmpFlags)flag & VideoProcAmpFlags.Auto)!=0);
				CamGlobals.CamDevice.GetProperty(property, out int value, out flag);
				_Value	   = value;
				_Auto	   = _AutoAvailable && (((VideoProcAmpFlags)flag & VideoProcAmpFlags.Auto)!=0);
				_Available = true;
				IsDefault = (_Value==_Default);
			}
		}

		//--- Flag ---------------------------------------------------
		private VideoProcAmpFlags Flag()
		{
			if (!_AutoAvailable) return VideoProcAmpFlags.None;
			if (_Auto) return VideoProcAmpFlags.Auto;
			return VideoProcAmpFlags.Manual;
		}

		//--- Property Name ---------------------------------------
		private string _Name;
		public string Name
		{
			get { return _Name; }
			set { SetProperty(ref _Name,value); }
		}

		//--- Property Available ---------------------------------------
		private bool _Available;
		public bool Available
		{
			get { return _Available; }
			set { SetProperty(ref _Available,value); }
		}

		//--- Property Minimum ---------------------------------------
		private int _Minimum;
		public int Minimum
		{
			get { return _Minimum; }
			set { SetProperty(ref _Minimum,value); }
		}

		//--- Property Maximum ---------------------------------------
		private int _Maximum;
		public int Maximum
		{
			get { return _Maximum; }
			set { SetProperty(ref _Maximum,value); }
		}

		//--- Property Step ---------------------------------------
		private int _Step;
		public int Step
		{
			get { return _Step; }
			set { SetProperty(ref _Step,value); }
		}

		//--- Property Default ---------------------------------------
		private int _Default;
		public int Default
		{
			get { return _Default; }
			set { SetProperty(ref _Default,value); }
		}

		//--- Property Value ---------------------------------------
		private int _Value;
		public int Value
		{
			get { return _Value; }
			set 
            { 
                if (SetProperty(ref _Value,value))
				{
					CamGlobals.CamDevice.SetProperty(_Property, _Value, Flag());
					IsDefault = (_Value==_Default);
				};
            }
		}

		//--- Property IsDefault ---------------------------------------
		private bool _IsDefault;
		public bool IsDefault
		{
			get { return _IsDefault; }
			private set { SetProperty(ref _IsDefault,value); }
		}

		//--- Property AutoAvailable ---------------------------------------
		private bool _AutoAvailable;
		public bool	AutoVailable
		{
			get { return _AutoAvailable; }
			set { SetProperty(ref _AutoAvailable,value); }
		}

		//--- Property Auto ---------------------------------------
		private bool _Auto;
		public bool	Auto
		{
			get { return _Auto; }
			set 
			{ 
				if (SetProperty(ref _Auto,value))
				{
					CamGlobals.CamDevice.SetProperty(_Property, _Value, Flag());
				};
			}
		}
    }
}
