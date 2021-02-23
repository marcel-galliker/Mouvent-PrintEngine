using MahApps.Metro.IconPacks;
using rx_CamLib;
using RX_Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;

namespace RX_DigiPrint.Models
{
	public class SA_Action: RxBindable
	{
		private List<double> _Values=new List<double>();

		//--- Property Name ---------------------------------------
		private string _Name;
		public string Name
		{
			get { return _Name; }
			set { SetProperty(ref _Name,value); }
		}

		//--- Property StepperNo ---------------------------------------
		private int _StepperNo;
		public int StepperNo
		{
			get { return _StepperNo; }
			set { SetProperty(ref _StepperNo,value); }
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

		//--- Property ConfirmVisibility ---------------------------------------
		private bool _ConfirmVisibile=false;
		public bool ConfirmVisibile
		{
			get { return _ConfirmVisibile; }
			set { SetProperty(ref _ConfirmVisibile,value); }
		}

		//--- Property Function ---------------------------------------
		private ECamFunction _Function=ECamFunction.CamNoFunction;
		public ECamFunction Function
		{
			get { return _Function; }
			set { 
					SetProperty(ref _Function,value);
					switch(value)
					{ 
					case ECamFunction.CamMeasureAngle:	IconKind=PackIconMaterialKind.AngleAcute; break;
					case ECamFunction.CamMeasureStitch: IconKind=PackIconMaterialKind.ArrowCollapseHorizontal; break;
					case ECamFunction.CamMeasureDist:	IconKind=PackIconMaterialKind.ArrowExpandDown; break;
					default: break;
					}
				}
		}

		//--- Property IconKind ---------------------------------------
		private PackIconMaterialKind _IconKind;
		public PackIconMaterialKind IconKind
		{
			get { return _IconKind; }
			set { SetProperty(ref _IconKind,value); }
		}

		//--- Property State ---------------------------------------
		private ECamFunctionState _State;
		public ECamFunctionState State
		{
			get { return _State; }
			set { SetProperty(ref _State,value); }
		}

		//--- Property MyProperty ---------------------------------------
		public int MeasureCnt
		{
			get { return _Values.Count(); }
		}

		//--- Property BaseIdx ---------------------------------------
		private int _BaseIdx=-1;
		public int BaseIdx
		{
			get { return _BaseIdx; }
			set { SetProperty(ref _BaseIdx,value); }
		}

		//--- Measured ---
		public void Measured(double value)
		{
			_Values.Add(value);
			if (value.Equals(double.NaN)) 
				ValueStr += string.Format(" ---");
			else                   
				ValueStr += string.Format(" {0:0.0}", value);
			if (_Values.Count()>3)
			{
				int cnt=0;
				string str1="", str2="";
				double avg=0;
				double min=1000;
				double max=-1000;
				foreach(double a in _Values) 
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
				foreach(double a in _Values)
				{
					if (!a.Equals(double.NaN) && a>=min && a<=max) diff+=(a-avg)*(a-avg);
				}
				diff /= cnt;
				diff = Math.Sqrt(diff);

				min=avg-diff;
				max=avg+diff;
				double avg2=0;
				cnt=0;
				foreach(double a in _Values)
				{
					if (!a.Equals(double.NaN) && a>=min && a<=max) { cnt++; avg2+=a; str2 += string.Format("{0:0.0}  ", a);}
				}
				if (cnt<3) return;

				Correction = avg2/cnt;
			//	Console.WriteLine("ANGLE CORRECTION: all {0}", str1);
			//	Console.WriteLine("ANGLE CORRECTION: ok  {0}", str2);
			//	Console.WriteLine("ANGLE CORRECTION: Corr={0:0.000} avg={1:0.000}, diff={2:0.000}, avg2={3:0.000}, cnt={4}", Correction, avg, diff, avg2, cnt);
			}
		}

		//--- Property ValueStr ---------------------------------------
		private string _ValueStr="";
		public string ValueStr
		{
			get { return _ValueStr; }
			set { SetProperty(ref _ValueStr,value); }
		}

		//--- Property Correction ---------------------------------------
		private double? _Correction;
		public double? Correction
		{
			get { return _Correction; }
			set { SetProperty(ref _Correction,value); }
		}

	}
}
