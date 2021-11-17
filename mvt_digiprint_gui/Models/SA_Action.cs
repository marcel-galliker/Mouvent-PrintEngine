using MahApps.Metro.IconPacks;
using rx_CamLib;
using RX_Common;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Windows.Forms;
using System.Windows.Media;

namespace RX_DigiPrint.Models
{
	public class SA_Value
	{
		public double Value { get; set; }
		public bool Used { get; set; }
	}

	public class SA_Action: RxBindable
	{
		public static readonly int  MeasurementPasses=3;

		public List<List<SA_Value>> _ValueList = new List<List<SA_Value>>();

		//--- Property Values ---------------------------------------
		private List<SA_Value> _Values;
		public List<SA_Value> Values
		{
			get { return _Values; }
			set { SetProperty(ref _Values, value); }
		}

		//--- Property Name ---------------------------------------
		private string _Name;
		public string Name
		{
			get { return _Name; }
			set { SetProperty(ref _Name,value); }
		}

		//--- Property Colorbrush ---------------------------------------
		private Brush _ColorBrush = Brushes.Transparent;
		public Brush ColorBrush
		{
			get { return _ColorBrush; }
			set { SetProperty(ref _ColorBrush,value); }
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

		//--- Property WebPos ---------------------------------------
		private double _WebPos;
		public double WebPos
		{
			get { return _WebPos; }
			set { SetProperty(ref _WebPos,value); }
		}

		//--- Property ScanPos ---------------------------------------
		private double _ScanPos;
		public double ScanPos
		{
			get { return _ScanPos; }
			set {
					if (_ScanPos!=0 && _ScanPos!=value && Function == ECamFunction.CamMeasureAngle)
						Console.WriteLine("ScanPos changed");
					SetProperty(ref _ScanPos, value);
				}
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
					case ECamFunction.CamMeasureAngle:		IconKind = PackIconMaterialKind.AngleAcute; break;
					case ECamFunction.CamMeasureStitch:		IconKind = PackIconMaterialKind.ArrowCollapseHorizontal; break;
					case ECamFunction.CamMeasureDist:		IconKind = PackIconMaterialKind.ArrowExpandDown; break;
					case ECamFunction.CamMeasureRegStitch:	IconKind = PackIconMaterialKind.ArrowCollapseHorizontal; break;
					case ECamFunction.CamMeasureRegAngle:	IconKind = PackIconMaterialKind.AngleAcute; break;
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

		private static readonly char CSV_SEPARATOR=';';

		//--- ReadCsv ----------------------------------------------
		public void ReadCsv(string filepath)
		{
			try
			{
				using (StreamReader sr = new StreamReader(filepath))
				{
					String fct = Function.ToString();
					while (true)
					{
						String line = sr.ReadLine();
						if (line==null) break;
						String[] field=line.Split(CSV_SEPARATOR);
						if (field[1].Equals(Name)
						&& field[2].Equals(Function.ToString()))
						{
							int pass = Rx.StrToInt32(field[3]);
							for (int n=4; n<field.Length; n++)
							{
								if (field[n].Contains("---")) Measured(pass, double.NaN);
								else
									Measured(pass, Rx.StrToDouble(field[n]));
							}
						}
					}
				}
			}
			catch (Exception e)
			{
				Console.WriteLine(e.Message);
			}
		}

		/// <summary>
		/// Writing the header line of the csv file
		/// </summary>
		/// <param name="csv">Stream Writer</param>
		public static void WriteCsvHeader(StreamWriter csv)
		{
			csv.Write("Color"); csv.Write(CSV_SEPARATOR);
			csv.Write("Head"); csv.Write(CSV_SEPARATOR);
			csv.Write("Function"); csv.Write(CSV_SEPARATOR);
			csv.Write("Pass"); csv.Write(CSV_SEPARATOR);
			for (int i=0; i<8; i++)
				csv.Write("Res"+(i+1).ToString()+ CSV_SEPARATOR);
			csv.WriteLine();
		}

		//--- WriteCsv ---------------------------------------------------------
		public void WriteCsv(StreamWriter csv)
		{
			for(int pass=0; pass<_ValueList.Count; pass++)
			{
				csv.Write(ColorBrush.ToString()); csv.Write(CSV_SEPARATOR);
				csv.Write(Name); csv.Write(CSV_SEPARATOR);
				csv.Write(Function.ToString()); csv.Write(CSV_SEPARATOR);
				csv.Write(pass+1); csv.Write(CSV_SEPARATOR);
				for(int i=0; i< _ValueList[pass].Count; i++)
				{
					if (double.IsNaN(_ValueList[pass][i].Value)) csv.Write("---");
					else                                         csv.Write(string.Format("{0:0.0}", _ValueList[pass][i].Value));
					csv.Write(CSV_SEPARATOR);
				}
//				csv.Write(_ValueList[pass].ToString().Replace(' ', CSV_SEPARATOR));
				csv.WriteLine();
			}
		}

		//--- FileOpen ---------------------------------------
		public static List<SA_Action> FileOpen()
		{
			List<SA_Action> actions = null;

			var fileContent = string.Empty;
			var filePath = string.Empty;

			using (OpenFileDialog openFileDialog = new OpenFileDialog())
			{
				openFileDialog.InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
				openFileDialog.Filter = "csv files (*.csv)|*.csv";
				openFileDialog.FilterIndex = 1;
				openFileDialog.RestoreDirectory = true;
				SA_Action action=null;

				if (openFileDialog.ShowDialog() == DialogResult.OK)
				{
					filePath = openFileDialog.FileName;

					try 
					{
						var fileStream = openFileDialog.OpenFile();
						using (StreamReader csv = new StreamReader(fileStream))
						{
							while (true)
							{
								String line = csv.ReadLine();
								if (line == null) break;
								String[] field = line.Split(CSV_SEPARATOR);
								ECamFunction function = ECamFunction.CamNoFunction;
								if (field[2].Equals(ECamFunction.CamMeasureAngle.ToString())) function= ECamFunction.CamMeasureAngle;
								else if (field[2].Equals(ECamFunction.CamMeasureStitch.ToString())) function = ECamFunction.CamMeasureStitch;
								
								if (function!= ECamFunction.CamNoFunction)
								{ 
									int pass= Rx.StrToInt32(field[3]);
									if (pass==1) 
									{ 
										action=new SA_Action();
										action.Function= function;
										action.ColorBrush = Rx.BrushFromStr(field[0]);
										action.Name = field[1];
										if (actions==null) actions=new List<SA_Action>();
										actions.Add(action);
									}
									for(int i=4; i<field.Length; i++)
										action.Measured(pass, Rx.StrToDouble(field[i]));
								}
							}
						}
					}
					catch (Exception ex)
					{
						Console.WriteLine("Exception {0}", ex.Message);
					}
				}
			}
			return actions;
		}

		//--- Property MeasureCnt ---------------------------------------
		public int MeasureCnt
		{
			get { 
					if (_Values==null) return 0;
					return _Values.Count(); 
				}
		}

		//--- Measured ---
		public void Measured(int pass, double value)
		{
			while (_ValueList.Count <= pass)
			{
				_ValueList.Add(new List<SA_Value>());
			}
		
			List<SA_Value> values = _ValueList[pass];

			values.Add(new SA_Value(){Value=value });

			if (values.Count()>3)
			{
				int cnt=0;
				string str1="", str2="";
				double avg=0;
				double min=1000;
				double max=-1000;
				foreach(SA_Value val in values) 
				{ 
					double a=val.Value;
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
				foreach(SA_Value val in values)
				{
					double a = val.Value;
					if (!a.Equals(double.NaN) && a>=min && a<=max) diff+=(a-avg)*(a-avg);
				}
				diff /= cnt;
				diff = Math.Sqrt(diff);

				min=avg-diff;
				max=avg+diff;
				double avg2=0;
				cnt=0;
				int i=0;
				foreach(SA_Value val in values)
				{
					double a = val.Value;
					if (!a.Equals(double.NaN) && a>=min && a<=max) { cnt++; avg2+=a; str2 += string.Format("{0:0.0}  ", a); val.Used =true;}
					i++;
				}
				if (cnt<3) return;

				Correction = avg2/cnt;

			//	Console.WriteLine("ANGLE CORRECTION: all {0}", str1);
			//	Console.WriteLine("ANGLE CORRECTION: ok  {0}", str2);
			//	Console.WriteLine("ANGLE CORRECTION: Corr={0:0.000} avg={1:0.000}, diff={2:0.000}, avg2={3:0.000}, cnt={4}", Correction, avg, diff, avg2, cnt);
			}

			//--- force a refresh ---
			Values = null;
			Values = values;
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
