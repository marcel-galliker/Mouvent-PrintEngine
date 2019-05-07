using DigiPrint.Common;
using DigiPrint.Models;
using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using Xamarin.Forms;

namespace RX_DigiPrint.Services
{
    public class RxBtMsgHandler
    {
        //--- constructor ---------------
        public RxBtMsgHandler()
        {
        }

        //--- handle_message ------------------
        public void handle_message(Byte[] msg)
        {
            Debug.WriteLine("handle_message ");
            RxStructConvert.ToStruct(out RxBtDef.SMsgHdr hdr, msg);
            Debug.WriteLine("handle_message id={0}", hdr.msgId);
            try
            {
                switch (hdr.msgId)
                {
                case RxBtDef.BT_REP_LOGIN:          AppGlobals.Bluetooth.OnLogin?.Invoke();
                                                    AppGlobals.Bluetooth.SendCommand(RxBtDef.BT_CMD_PROD_STATE);
                                                    break;
                case RxBtDef.BT_REP_STATE:          handle_state(msg); break;
                case RxBtDef.BT_REP_EVT_CONFIRM:    handle_confirm_events();break;
                case RxBtDef.BT_EVT_GET_EVT:        handle_event(msg); break;
                case RxBtDef.BT_EVT_PROD_STATE:     handle_prod_state(msg); break;
                case RxBtDef.BT_REP_PROD_PREVIEW:   handle_prod_preview(msg, hdr.msgLen); break;
                case RxBtDef.BT_REP_HEAD_STATE:     handle_head_state(msg); break;
                case RxBtDef.BT_REP_STEPPER_STATE:  handle_stepper_state(msg); break;
                default:
  //                      Error("RxMsgHandler: Received unknown MessageId=0x{0:X}", hdr.msgId); 
                        break;
                }
            }
            catch (Exception e)
            {
                string m=e.Message;
//                Console.WriteLine("RxMsgHandler.handle_message (id={0:X}): >>{1}<<", hdr.msgId, e.Message );
//                Console.WriteLine("Stack Trace:\n{0}\n", e.StackTrace);
            }
            Debug.WriteLine("handle_message Done");
        }

        //--- handle_state -------------------------------
        private void handle_state(byte[] buf)
        {
            Debug.WriteLine("handle_state");
            RxStructConvert.ToStruct(out RxBtDef.SReplyStateMsg msg, buf);
            Rx.Invoke(() =>AppGlobals.Printer.Update(msg));
        }

        //--- handle_confirm_events ------------------
        private void handle_confirm_events()
        {
            Debug.WriteLine("handle_confirm_events");
            Rx.Invoke(() => 
            {
                AppGlobals.Events.Clear();
                AppGlobals.Printer.EventLevel = RxBtDef.ELogType.eErrUndef;
            });
        }

        //--- handle_event -------------------------------
        private void handle_event(byte[] buf)
        {
            Debug.WriteLine("handle_event");
            RxStructConvert.ToStruct(out RxBtDef.SErrorMsg msg, buf);
            Rx.Invoke(() => AppGlobals.Events.Add(new RxEvents(msg)));
        }

        //--- handle_prod_state ---------------------------------------------
        private void handle_prod_state(byte[] buf)
        {
            Debug.WriteLine("handle_prod_state");
            RxStructConvert.ToStruct(out RxBtDef.SProdStateMsg msg, buf);
            Rx.Invoke(() => AppGlobals.ProdState.Update(msg));
        }

        //--- handle_head_state ---------------------------------------------
        private void handle_head_state(byte[] buf)
        {
            Debug.WriteLine("handle_head_state");
            RxStructConvert.ToStruct(out RxBtDef.SBtHeadStatMsg msg, buf);
            if (msg.no< AppGlobals.PrintHeads.Count)
                Rx.Invoke(() => AppGlobals.PrintHeads[msg.no].Update(msg.status));
        }

        //--- handle_stepper_state ---------------------------------------------
        private void handle_stepper_state(byte[] buf)
        {
            Debug.WriteLine("handle_stepper_state");
            RxStructConvert.ToStruct(out RxBtDef.SBtStepperStatMsg msg, buf);
            Rx.Invoke(() =>
                { 
                    while (msg.no >= AppGlobals.Steppers.Count)
                        AppGlobals.Steppers.Add(new StepperMotor( AppGlobals.Steppers.Count));
                    AppGlobals.Steppers[msg.no].Update(msg.status);
                }
            );
        }

        //--- handle_prod_preview -------------------------------
        private void handle_prod_preview(byte[] buf, int msgLen)
        {
            //--- copy the data as it is comsumed in the UI Thread asynchronously ---
            int size = Marshal.SizeOf(new RxBtDef.SMsgHdr());
            Rx.Invoke(() => AppGlobals.ProdState.Preview = ImageSource.FromStream(() => new MemoryStream(buf, size, msgLen - size)));
        }
    }
}
