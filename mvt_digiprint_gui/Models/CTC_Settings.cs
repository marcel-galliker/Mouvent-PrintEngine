using RX_Common;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
	public class CTC_Settings
	{
		Dictionary<string, CTC_Param> Settings = new Dictionary<string, CTC_Param>();

		private string path;

		//--- Constructor ------------------------------------------------
		public CTC_Settings()
		{
			path = RxGlobals.PrinterProperties.GetUserPath()+"\\CTC_Settings.csv";
			if (File.Exists(path))
			{
				try
				{
					using(StreamReader file = new StreamReader(path))
					{
						while (!file.EndOfStream)
						{
							string line=file.ReadLine();
							string[] fields = line.Split(';');
							CTC_Param param= new CTC_Param(fields[0], fields[1], fields[2], Rx.StrToInt32(fields[3]), Rx.StrToInt32(fields[4]));
							Settings.Add(param.Key, param);
						}
						file.Close();
					}
				}
				catch(Exception ex)
				{
					Console.WriteLine("Excaption {0}", ex.Message);
				}
			}
		}


		//--- WriteSettings -----------------------------------------
		private void WriteSettings()
		{
			path = RxGlobals.PrinterProperties.GetUserPath()+"\\CTC_Settings.csv";
			try
			{
				using(StreamWriter file = new StreamWriter(path))
				{
					foreach(KeyValuePair<string, CTC_Param> par in Settings)
					{
						file.WriteLine(String.Format("{0};{1};{2};{3};{4}", par.Value.Name, par.Value.Step, par.Value.Sensor, par.Value.Min, par.Value.Max));
					}
					file.Close();
				}
			}
			catch(Exception ex)
			{ 
				Console.WriteLine("Excaption {0}", ex.Message);
			}
		}

		//--- GetParam ----------------------------------------------------------------
		public CTC_Param GetParam(string name, string step, string sensor, Int32 min, Int32 max)
		{
			string key=CTC_Param.ParamKey(name, step, sensor);
			if (!Settings.ContainsKey(key)) 
			{
				CTC_Param param= new CTC_Param(name, step, sensor, min, max);
				Settings.Add(param.Key, param);
				WriteSettings();
			}
			return Settings[key];
		}
	}
}
