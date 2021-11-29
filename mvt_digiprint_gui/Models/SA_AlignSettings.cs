using RX_Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
	public class SA_AlignSettings : RxSettingsBase
	{
		public int Passes { get; set; }
		public double ToleranceAngle { get; set; }
		public double ToleranceStitch { get; set; }
		public bool[] MeasureAngle { get; set; }
		public bool[] MeasureStitch { get; set; }

		private readonly string _Path = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData) + "\\" + System.Windows.Forms.Application.ProductName;
		//-----------------------------------------------------------
		public void Init()
		{
			Load(_Path);
			int cnt= RxGlobals.PrintSystem.ColorCnt;
			/*
			Passes=1;
			bool[] angle  = new bool[cnt];
			bool[] stitch = new bool[cnt];
			for (int i=0; i<cnt; i++)
			{
				angle[i]=true;
				stitch[i]=true;
			}
			MeasureAngle = angle;
			MeasureStitch = stitch;
			ToleranceAngle = 0.5;
			ToleranceStitch = 0.5;
			*/
		}

		public void Save()
		{
			Save(_Path);
		}
	}
}
