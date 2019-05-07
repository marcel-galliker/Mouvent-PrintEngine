using DigiPrint.Common;
using System;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace RX_DigiPrint.Services
{
    public class RxBluetooth : RxBindable
    {
        private Task    _Reveiver;

        public enum EBtResult : int
        {
            result_ok = 0,
            result_disabled,
            result_not_bound,
            result_error,
        };

        public delegate EBtResult DelegateConnect(string deviceName);
        public DelegateConnect Connect; // add platform dependent function here
        public Action          OnLogin;
        public Action          OnDisconnect;

        //--- constructor ------------------------
        public RxBluetooth()
        {
            Deconnect();
        }

        //--- Property StreamIn ---------------------------------------
        private Stream _StreamIn;
        public Stream StreamIn
        {
            set 
            { 
                if (SetProperty(ref _StreamIn, value))
                {
                    IsConnected = (_StreamIn!=null);
                    if (IsConnected)
                    {
                        _Reveiver = new Task(_ReceiverThread);
                        _Reveiver.Start();
                    }
                }
            }
        }

        //--- Property StreamOut ---------------------------------------
        private Stream _StreamOut;
        public Stream StreamOut
        {
            set { SetProperty(ref _StreamOut, value); }
        }

        //--- _IsConnected ---------------------------------------------
        private bool _IsConnected;
        public bool IsConnected
        {
            get { return _IsConnected; }
            set { SetProperty(ref _IsConnected, value);}
        }

        //--- Property ConnectionId ---------------------------------------
        private string _ConnectionId;
        public string ConnectionId
        {
            get { return _ConnectionId; }
            set { SetProperty(ref _ConnectionId, value); }
        }

        //--- Deconnect ------------------------
        public void Deconnect()
        {
            StreamIn = null;
            StreamOut = null;
            OnDisconnect?.Invoke();
        }

        //--- _ReceiverThread -----------------------------------------
        private void _ReceiverThread()
        {
            int size = Marshal.SizeOf<Int32>();
            byte[] lenbuf = new byte[size];
            Int32 read, len;
            RxBtMsgHandler msgHandler = new RxBtMsgHandler();
            
            while (_StreamIn!=null)
            {
                try
                {
                    read = _StreamIn.Read(lenbuf, 0, size);
                    if (read <= 0)
                    {
                        // RxGlobals.Events.AddItem(new LogItem("Bluetooth: >>"+client.RemoteMachineName+"<< Closed 1")); 
                        Deconnect();
                        break;
                    }
                    if (read!=size)
                    {
                        // RxGlobals.Events.AddItem(new LogItem("Bluetooth: >>"+client.RemoteMachineName+"<< Closed 1")); 
                        Deconnect();
                        break;
                    }
                    len = BitConverter.ToInt32(lenbuf, 0);
                    byte[] buffer = new byte[len];
                    lenbuf.CopyTo(buffer, 0);
                    while (read<len)
                    {
                        read += _StreamIn.Read(buffer, read, len - read);
                    }

                    //  string result = System.Text.Encoding.UTF8.GetString(buffer, size+4, len - size-4);
                    //  Console.WriteLine(client.RemoteMachineName + " : " + result);
                    //  RxGlobals.Events.AddItem(new LogItem("Bluetooth received "+len+ "Bytes >>" + result + "<<" ));
                    msgHandler.handle_message(buffer);
                }
                catch (Exception)
                {
                    Deconnect();
                    break;
                }
            }
        }
        //--- SendCommand --------------------------------
        public void SendCommand(UInt32 msgId)
        {
            RxBtDef.SMsgHdr hdr = new RxBtDef.SMsgHdr() { msgId = msgId, msgLen = 8};

            if (_StreamOut != null)
            {
                RxStructConvert.ToBuffer<RxBtDef.SMsgHdr>(out byte[] buffer, hdr);
                try
                {
                    _StreamOut.Write(buffer, 0, buffer.Count());
                }
                catch (Exception)
                {
                    Deconnect();
                }
            }
        }

        //--- SendMsg ----------------------------------------
        public void SendMsg<type>(UInt32 msgId, ref type msg)
        {
            if (_StreamOut != null)
            {
                RxStructConvert.ToBuffer<type>(out byte[] buffer, msg);
                RxStructConvert.OverwriteBuffer(buffer, new RxBtDef.SMsgHdr() { msgId = msgId, msgLen = buffer.Count() });
                try
                {
                    _StreamOut.Write(buffer, 0, buffer.Count());
                }
                catch (Exception)
                {
                    Deconnect();
                }
            }
        }
    }
}
