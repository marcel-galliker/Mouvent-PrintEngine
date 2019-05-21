using RX_Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace RX_DigiPrint.Models
{
    public class RxBtProdState : RxBindable
    {
        private RxBtDef.SProdStateMsg _msg = new RxBtDef.SProdStateMsg();
        private RxBtDef.SProdStateMsg _msg_sent = new RxBtDef.SProdStateMsg();

        //--- creator -----------------------------------------------
        public RxBtProdState()
        {
            _msg.hdr.msgId  = RxBtDef.BT_EVT_PROD_STATE;
            _msg.hdr.msgLen = Marshal.SizeOf(_msg);
        }

        //--- Property State ---------------------------------------
	    public RxBtDef.EPQState State
	    {
		    get { return _msg.state;}
		    set { SetProperty(ref _msg.state, value);}
	    }

        //--- Property FilePath ---------------------------------------
	    public string FilePath
	    {
		    get { return _msg.filepath;}
		    set { SetProperty(ref _msg.filepath, value);}
	    }

	    //--- Property Progress ---------------------------------------
	    public int Progress
	    {
		    get { return _msg.progress;}
		    set { SetProperty(ref _msg.progress, value);}
	    }

	    //--- Property ProgressStr ---------------------------------------
	    public string ProgressStr
	    {
		    get { return _msg.progressStr;}
		    set { SetProperty(ref _msg.progressStr, value);}
	    }

        //--- Property CopiesStr ---------------------------------------
        public string CopiesStr
        {
            get { return _msg.copiesStr; }
            set { SetProperty(ref _msg.copiesStr, value); }
        }
        
        //--- SetBnState -----------------------------------------------
	    private void SetBnState(ref RxBtDef.EButtonState state, bool isEnabled, bool isChecked, bool isLarge)
        {
            state=0;
            if (isEnabled)  state |= RxBtDef.EButtonState.isEnabled;
            if (isChecked)  state |= RxBtDef.EButtonState.isChecked;
            if (isLarge)    state |= RxBtDef.EButtonState.isLarge;
        }

        //--- Property StartBnState ---------------------------------------
	    public void SetStartBnState(bool isEnabled, bool isChecked, bool isLarge)
        {
            SetBnState(ref _msg.startBnState, isEnabled, isChecked, isLarge);
        }
	
        //--- Property PauseBnState ---------------------------------------
	    public void SetPauseBnState(bool isEnabled, bool isChecked, bool isLarge)
	    {
            SetBnState(ref _msg.pauseBnState, isEnabled, isChecked, isLarge);
	    }

        //--- Property StopBnState ---------------------------------------
	    public void SetStopBnState(bool isEnabled, bool isChecked, bool isLarge)
	    {
            SetBnState(ref _msg.stopBnState, isEnabled, isChecked, isLarge);
	    }

        //--- Property AbortBnState ---------------------------------------
	    public void SetAbortBnState(bool isEnabled, bool isChecked, bool isLarge)
	    {
            SetBnState(ref _msg.abortBnState, isEnabled, isChecked, isLarge);
	    }

        //--- send -----------------------------------------
        public void send(bool force)
        {
            _msg.allInkSuppliesOn = RxGlobals.PrinterStatus.AllInkSupliesOn;
            
            if (force || !_msg.Equals(_msg_sent))
            {
                RxGlobals.Bluetooth.SendMsg(RxBtDef.BT_EVT_PROD_STATE, ref _msg);
                _msg_sent = _msg;
            }
        }
    }
}
