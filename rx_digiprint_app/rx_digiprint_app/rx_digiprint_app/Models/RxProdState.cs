using DigiPrint.Common;
using RX_DigiPrint.Services;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Xamarin.Forms;

namespace DigiPrint.Models
{
    public class RxProdState : RxBindable
    {
        //--- Update -----------------------------------
        public void Update(RxBtDef.SProdStateMsg msg)
        {
            State            = msg.state;
            FilePath         = msg.filepath;
            Progress         = msg.progress/100.0;
            ProgressStr      = msg.progressStr;
            CopiesStr        = msg.copiesStr;
            StartBnState     = msg.startBnState;
            PauseBnState     = msg.pauseBnState;
            StopBnState      = msg.stopBnState;
            AbortBnState     = msg.abortBnState;
            AllInkSuppliesOn = msg.allInkSuppliesOn;
        }

        //--- Property State ---------------------------------------
        private RxBtDef.EPQState _State;
        public RxBtDef.EPQState State
        {
            get { return _State; }
            set { SetProperty(ref _State, value); }
        }

        //--- Property Progress ---------------------------------------
        private double _Progress;
        public double Progress
        {
            get { return _Progress; }
            set { SetProperty(ref _Progress, value); }
        }

        //--- Property ProgressStr ---------------------------------------
        private string _ProgressStr;
        public string ProgressStr
        {
            get { return _ProgressStr; }
            set { SetProperty(ref _ProgressStr, value); }
        }

        //--- Property CopiesStr ---------------------------------------
        private string _CopiesStr;
        public string CopiesStr
        {
            get { return _CopiesStr; }
            set { SetProperty(ref _CopiesStr, value); }
        }


        //--- Property FilePath ---------------------------------------
        private string _FilePath;
        public string FilePath
        {
            get { return _FilePath; }
            set
            {
                if (SetProperty(ref _FilePath, value))
                {
                    Preview=null;
                    AppGlobals.Bluetooth.SendCommand(RxBtDef.BT_CMD_PROD_PREVIEW);
                }
            }
        }

        //--- Property StartBnState ---------------------------------------
        private RxBtDef.EButtonState _StartBnState;
        public RxBtDef.EButtonState StartBnState
        {
            get { return _StartBnState; }
            set { SetProperty(ref _StartBnState, value); }
        }

        //--- Property PauseBnState ---------------------------------------
        private RxBtDef.EButtonState _PauseBnState;
        public RxBtDef.EButtonState PauseBnState
        {
            get { return _PauseBnState; }
            set { SetProperty(ref _PauseBnState, value); }
        }

        //--- Property StopBnState ---------------------------------------
        private RxBtDef.EButtonState _StopBnState;
        public RxBtDef.EButtonState StopBnState
        {
            get { return _StopBnState; }
            set { SetProperty(ref _StopBnState, value); }
        }

        //--- Property AbortBnState ---------------------------------------
        private RxBtDef.EButtonState _AbortBnState;
        public RxBtDef.EButtonState AbortBnState
        {
            get { return _AbortBnState; }
            set { SetProperty(ref _AbortBnState, value); }
        }

        //--- Property AllInkSuppliesOn ---------------------------------------
        private bool _AllInkSuppliesOn;
        public bool AllInkSuppliesOn
        {
            get { return _AllInkSuppliesOn; }
            set { SetProperty(ref _AllInkSuppliesOn, value); }
        }

        //--- Property PreviewImage ---------------------------------------
        private ImageSource _Preview;
        public ImageSource Preview
        {
            get { return _Preview; }
            set { SetProperty(ref _Preview, value); }
        }
    }
}
