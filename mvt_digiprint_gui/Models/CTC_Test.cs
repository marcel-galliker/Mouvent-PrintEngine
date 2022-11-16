using RX_Common;
using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace RX_DigiPrint.Models
{
	public class CTC_Test: RxBindable
	{
		public const int HEADS=48;
		public static CTC_Test Overall = new CTC_Test() { Name = "OVERALL" };

		//--- EN_State -----------------------
		public enum EN_State
		{
			undef,
			running,
			ok,
			failed
		}

		//--- constructor -----------------------------------------------
		public CTC_Test()
		{
			_State = new ObservableCollection<EN_State>();
			for (int head=0; head<HEADS; head++)
				_State.Add(EN_State.undef);
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

		//--- Property State ---------------------------------------
		private ObservableCollection<EN_State> _State;
		public ObservableCollection<EN_State> State
		{
			get { return _State; }
			set { SetProperty(ref _State, value); }
		}

		//--- Start ----------------------------------------
		public void Start()
		{
			for (int head = 0; head < HEADS; head++) 
			{
				if (Overall.State[head] != EN_State.failed) SetHeadState(head, EN_State.running);
			}
		}

		//--- Done -------------------------------------------
		public void Done(CTC_Test.EN_State state)
		{

			for( int headNo=0; headNo<CTC_Test.HEADS; headNo++)
			{
				if (State[headNo]==CTC_Test.EN_State.running) State[headNo]=state;
			}
		}
		//--- SetResult -------------------------------
		public void SetResult(int head, bool ok)
		{
			if (Overall.State[head]!=EN_State.failed) 
			{
				if (ok) SetHeadState(head, EN_State.ok);
				else    SetHeadState(head, EN_State.failed);
			}
		}

		//--- SetHeadState -----------------------------
		public void SetHeadState(int head, EN_State state)
		{
			RxBindable.Invoke(()=>
			{
				State[head]=state;
				if (state==EN_State.ok || state==EN_State.failed)
				{
					if (Overall.State[head]==EN_State.undef) Overall.State[head]=state;
					else if (state==EN_State.failed)		 Overall.State[head]=EN_State.failed;
				}
			});
		}

		//--- ResetState -------------------------------
		public void ResetState()
		{
			for (int head = 0; head < HEADS; head++) State[head]=EN_State.undef;
		}

		//--- SimuFailed ---------------------------------
		public void SimuFailed(List<int> list)
		{
			for (int head=0; head<HEADS; head++)
			{
				if (Overall.State[head] != EN_State.failed)
				{
					if (list.Contains(head)) SetHeadState(head, EN_State.failed);
					else					 SetHeadState(head, EN_State.ok);
				}
			}
		}
	}
}
