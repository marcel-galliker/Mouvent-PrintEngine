using rx_CamLib;
using RX_Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
	public class SA_Action: RxBindable
	{
		//--- Property Name ---------------------------------------
		private string _Name;
		public string Name
		{
			get { return _Name; }
			set { SetProperty(ref _Name,value); }
		}

		//--- Property PrintbarNo ---------------------------------------
		private int _PrintbarNo;
		public int PrintbarNo
		{
			get { return _PrintbarNo; }
			set { SetProperty(ref _PrintbarNo,value); }
		}

		//--- Property HeadNo ---------------------------------------
		private int _HeadNo;
		public int HeadNo
		{
			get { return _HeadNo; }
			set { SetProperty(ref _HeadNo,value); }
		}

		//--- Property WebMoveDist ---------------------------------------
		private double _WebMoveDist;
		public double WebMoveDist
		{
			get { return _WebMoveDist; }
			set { SetProperty(ref _WebMoveDist,value); }
		}

		//--- Property ScanPos ---------------------------------------
		private double _ScanPos;
		public double ScanPos
		{
			get { return _ScanPos; }
			set { SetProperty(ref _ScanPos, value); }
		}

		//--- Property ScanMoveDone ---------------------------------------
		private bool _ScanMoveDone;
		public bool ScanMoveDone
		{
			get { return _ScanMoveDone; }
			set { SetProperty(ref _ScanMoveDone,value); }
		}

		//--- Property WebMoveDone ---------------------------------------
		private bool _WebMoveDone;
		public bool WebMoveDone
		{
			get { return _WebMoveDone; }
			set { SetProperty(ref _WebMoveDone,value); }
		}

		//--- Property Function ---------------------------------------
		private ECamFunction _Function=ECamFunction.CamNoFunction;
		public ECamFunction Function
		{
			get { return _Function; }
			set { SetProperty(ref _Function,value); }
		}

		//--- Property State ---------------------------------------
		private ECamFunctionState _State;
		public ECamFunctionState State
		{
			get { return _State; }
			set { SetProperty(ref _State,value); }
		}

		public int MeasureCnt;
		private List<double> _Angles=new List<double>();
		//--- AngleMeasured ---
		public void AngleMeasured(double value)
		{
			_Angles.Add(value);
			if (value.Equals(double.NaN)) 
				AngleStr += string.Format(" ---");
			else                   
				AngleStr += string.Format(" {0:0.0}", value);
			if (_Angles.Count()>3)
			{
				int cnt=0;
				string str1="", str2="";
				double avg=0;
				double min=1000;
				double max=-1000;
				foreach(double a in _Angles) 
				{ 
					if (!a.Equals(double.NaN))
					{
						if (a<min) min=a;
						if (a>max) max=a;
						avg+=a;
						str1 += string.Format("{0:0.0}  ", a);
						cnt++;
					}
				}
				if (cnt<3) return;
				cnt-=2;
				avg = (avg-min-max)/cnt;
				
				double diff=0;
				foreach(double a in _Angles) diff+=(a-avg)*(a-avg);
				diff /= cnt;
				diff = Math.Sqrt(diff);

				min=avg-diff;
				max=avg+diff;
				double avg2=0;
				cnt=0;
				foreach(double a in _Angles)
				{
					if (!a.Equals(double.NaN) && a>=min && a<=max) { cnt++; avg2+=a; str2 += string.Format("{0:0.0}  ", a);}
				}
				if (cnt<3) return;

				Angle=avg2/cnt;
				Console.WriteLine("ANGLE CORRECTION: all {0}", str1);
				Console.WriteLine("ANGLE CORRECTION: ok  {0}", str2);
				Console.WriteLine("ANGLE CORRECTION: Corr={0:0.000} avg={1:0.000}, diff={2:0.000}, avg2={3:0.000}, cnt={4}", Angle, avg, diff, avg2, cnt);
			}
		}

		//--- Property Angle ---------------------------------------
		private string _AngleStr="";
		public string AngleStr
		{
			get { return _AngleStr; }
			set { SetProperty(ref _AngleStr,value); }
		}

		//--- Property Angle ---------------------------------------
		private double _Angle;
		public double Angle
		{
			get { return _Angle; }
			set { SetProperty(ref _Angle,value); }
		}

	}
}
