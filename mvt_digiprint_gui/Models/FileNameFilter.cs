using RX_Common;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;

namespace RX_DigiPrint.Models
{

	public class FileNameFilter : RxBindable
	{
		//--- Property Filter ---------------------------------------
		private string _Filter="";
		public string Filter
		{
			get { return _Filter; }
			set { SetProperty(ref _Filter,value); }
		}
	}
}
