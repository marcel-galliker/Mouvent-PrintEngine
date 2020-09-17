using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections;
using RX_Common;

namespace RX_DigiPrint.Models.Enums
{
	public class EN_TestFluid : IEnumerable
	{
		private static List<RxEnum<int>> _List;
		public EN_TestFluid()
		{
			if (_List == null)
			{
				_List = new List<RxEnum<int>>();
				_List.Add(new RxEnum<int>(0, "OFF"));
				_List.Add(new RxEnum<int>(1, "IPA"));
				_List.Add(new RxEnum<int>(2, "XL"));
			}
		}


		IEnumerator IEnumerable.GetEnumerator()
		{
			return (IEnumerator)new RxListEnumerator<RxEnum<int>>(_List);
		}

	}
}
