using InTheHand.Net;
using InTheHand.Net.Sockets;
using RX_Common;
using RX_DigiPrint.Models;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace RX_DigiPrint.Services
{
    public class RxBtClient
    {
        private BluetoothClient _Client;
        private Stream          _ClientStream;

        Guid _GUID = new Guid("{"+RxBtDef.MouventBlueToothGuid+"}");

        //--- constructor -------------------------------
        public RxBtClient()
        {
        }

        //--- constructor -------------------------------
        public RxBtClient(BluetoothClient client)
        {
            _Client       = client;
            _ClientStream = _Client.GetStream();
        }
        
        //--- close -----------------------------------
        public void close()
        {
            if (_Client!= null && _Client.Connected) _Client.Close(); 
        }

        //--- RemoteMachineName -----------------
        public string RemoteMachineName
        {
            get { return _Client.RemoteMachineName; }
        }

        //--- RemoteMachineName -----------------
        public NetworkStream Stream
        {
            get { return _Client.GetStream(); }
        }
        
        //--- Connected -----------------
        public bool Connected
        {
            get { return _Client!=null && _Client.Connected; }
        }
        //--- Property UserName ---------------------------------------
        private string _UserName;
        public string UserName
        {
            get { return _UserName; }
            set { _UserName = value; }
        }
        
        //--- LogIn -----------------------------------------
        public void LogIn(RxBtDef.SLogInMsg msg)
        {
            RxBtDef.SReplyMsg reply = new RxBtDef.SReplyMsg();
            if (RxGlobals.Bluetooth.LogIn(msg, this))
            {
                RxBindable.Invoke(()=>UserName = msg.userName);
                reply.reply = 0;
            }
            else
            {
                close();
                reply.reply = 1;
            }
            SendMsg(RxBtDef.BT_REP_LOGIN, ref reply);          
        }

        //--- SendCommand ------------------------------------------------------------
        public void SendCommand(UInt32 cmd)
        {
            TcpIp.SMsgHdr hdr=new TcpIp.SMsgHdr();

            if (_ClientStream!=null)
            {
                hdr.msgLen = Marshal.SizeOf(hdr);
                hdr.msgId = cmd;
                byte[] buffer;
                RxStructConvert.ToBuffer<TcpIp.SMsgHdr>(out buffer, hdr);
                try
                {
                    _ClientStream.Write(buffer, 0, hdr.msgLen);
                }
                catch (Exception e)
                {
                    Console.WriteLine("TCP/IP Send Exception: >>{0}<<", e.Message);
                    _Client.Close();
                    _ClientStream = null;
                }
            }
        }

        //--- SendMsg ----------------------------------------
        public void SendMsg<type>(UInt32 msgId, ref type msg)
        {
            if (_ClientStream!=null)
            {
                byte[] buffer;
                RxStructConvert.ToBuffer<type>(out buffer, msg);
                RxStructConvert.OverwriteBuffer(buffer, new TcpIp.SMsgHdr(){msgId=msgId, msgLen=buffer.Count()});

                try
                {
                    _ClientStream.Write(buffer, 0, buffer.Count());  
                }
                catch (Exception e)
                {
                    Console.WriteLine("TCP/IP Send Exception: >>{0}<<", e.Message);
                    _Client.Close();
                    _ClientStream = null;
                }
            }
        }

        //--- SendMsgBuf ----------------------------------------
        public void SendMsgBuf(UInt32 msgId, byte[] data)
        {
            if (_ClientStream!=null)
            {
                int hdrsize  = Marshal.SizeOf(typeof(TcpIp.SMsgHdr));
                byte[] msg = new byte[hdrsize+data.Length];
                RxStructConvert.OverwriteBuffer(msg, new TcpIp.SMsgHdr(){msgId=msgId, msgLen=msg.Length});
                data.CopyTo(msg, hdrsize);
                try
                {
                    _ClientStream.Write(msg, 0, msg.Count());            
                }
                catch (Exception e)
                {
                    Console.WriteLine("TBluetooth Send Exception: >>{0}<<", e.Message);
                    _Client.Close();
                    _ClientStream = null;
                }
            }
        }
    }
}
