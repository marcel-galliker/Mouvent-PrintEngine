using RX_Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
	public class CTC_Param : RxBindable
	{
		public CTC_Param()
		{
		}

		public CTC_Param(string name, string step, string sensor, Int32 min, Int32 max)
		{
			Name	= name;
			Step	= step;
			Sensor	= sensor;
			Min		= min;
			Max		= max;
		}

		//--- ParamKey -------------------
		static public string ParamKey(string name, string step, string sensor)
		{
			return name+'.'+step+'.'+sensor;
		}

		//--- Property Key ---------------------------------------
		public string Key
		{
			get { return ParamKey(_Name, _Step, _Sensor); }
		}

		//--- Property Name ---------------------------------------
		private string _Name;
		public string Name
		{
			get { return _Name; }
			set { SetProperty(ref _Name, value); }
		}

		//--- Property Step ---------------------------------------
		private string _Step;
		public string Step
		{
			get { return _Step; }
			set { SetProperty(ref _Step, value); }
		}

		//--- Property Sensor ---------------------------------------
		private string _Sensor;
		public string Sensor
		{
			get { return _Sensor; }
			set { SetProperty(ref _Sensor, value); }
		}

		//--- Property Min ---------------------------------------
		private Int32 _Min;
		public Int32 Min
		{
			get { return _Min; }
			set { SetProperty(ref _Min, value); }
		}

		//--- Property Max ---------------------------------------
		private Int32 _Max;
		public Int32 Max
		{
			get { return _Max; }
			set { SetProperty(ref _Max, value); }
		}

	}
}
