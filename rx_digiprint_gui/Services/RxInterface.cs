using RX_Common;
using RX_Common.Source;
using RX_DigiPrint.Helpers;
using RX_DigiPrint.Models;
using System;
using System.Linq;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Threading;

namespace RX_DigiPrint.Services
{
    public class RxInterface : RxBindable
    {
        //--- Creator -----------------------------------------
        #region Creator
        
        static TcpClient        _Client;
        private Boolean         _Running;

        private static int cnt = 0;
        public RxInterface()
        {

      //    System.Windows.MessageBox.Show("Start");    // REMOTE DEBUGGING

       //     Console.WriteLine("*** RxInterface {0}", DateTime.Now);

            if (_task == null)
            {
                Address = RxGlobals.PrinterProperties.IP_Address;
                _Running = true;
//                _task = new Task(TcpIpThread);
                _task = new Thread(TcpIpThread);
                _task.Start();
            }
            cnt++;
            if (cnt > 1) 
                throw new Exception("Must not be initialzed more than once");
        }
        #endregion

        //--- ShutDown ---------------------------------------------------
        public void ShutDown()
        {
            _Running = false;
            if (_Client!=null) _Client.Close();
        }

        //--- destructor ---
        ~RxInterface()
        {
        }

        #region Properties
        //--- Property Running ---------------------------------------
        public bool Running
        {
            get { return _Running; }
            set { SetProperty(ref _Running, value); }
        }
        
        //---Property Connected ---------------------------------
        private Boolean _Connected = false;
        public Boolean Connected
        {
            get { return _Connected; }
            private set
            {
                if (_Running) SetProperty(ref _Connected, value);
            }
        }

        //--- Property Address ----------------------------------------

        private string _Address;
        public string Address
        {
            get { return _Address; }
            set 
            { 
                if (SetProperty(ref _Address, value))
                {
                    if (_Client!=null && _Client.Client.Connected)
                    { 
                        _Client.Close();
                    }
                }
            }
        }

        //--- Property LocalAddress ---------------------------------------
        private string _LocalAddress;
        public string LocalAddress
        {
            get { return _LocalAddress; }
            private set 
            { 
                string str=value;
                int pos = str.IndexOf(":");
                if (pos>0) SetProperty(ref _LocalAddress, str.Substring(0,pos)); 
                else SetProperty(ref _LocalAddress, value);
            }
        }        

       #endregion

        //--- MAIN Worker Thread --------------------------------------------------
        static Thread _task = null;
        static LogList _log = new LogList();
        static NetworkStream _Stream;

        private void TcpIpThread()
        {
            byte[] buffer = new Byte[4096];            
            Int32 read, len;
            int size = 4;
            RxMsgHandler msgHandler = new RxMsgHandler();
            
            while (_Running)
            {
                //--- connecting -------------------------------------------
                _Client = new TcpClient();
    
             //   Console.WriteLine("*** TcpIpThread started {0}", DateTime.Now);

                while (_Running && !Connected)
                {
                    try 
                    {
                        _Client.Connect(Address, TcpIp.PORT_GUI); 
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine(e.Message);
                        continue;
                    };

            //        Console.WriteLine("*** Connected {0}", DateTime.Now);

                    Connected = true;
            //        log.AddItem(new LogItem() { Error = 1010, Message = "Connected to Server" });
                }
                if (Connected && _Client.Client!=null)
                {
                    LocalAddress = _Client.Client.LocalEndPoint.ToString();

                    _Stream = _Client.GetStream();
                    msgHandler.Restart();

                    SendCommand(TcpIp.CMD_GET_INK_DEF);
                    SendCommand(TcpIp.CMD_GET_PRINTER_CFG);
                    SendCommand(TcpIp.CMD_GET_NETWORK);
                    SendCommand(TcpIp.CMD_GET_PRINT_QUEUE);
                    SendCommand(TcpIp.CMD_GET_PRINT_ENV);
                    SendCommand(TcpIp.CMD_GET_STEPPER_CFG);
                    SendCommand(TcpIp.CMD_GET_PRN_STAT);
                    SendCommand(TcpIp.CMD_GET_EVT);
                    SendCommand(TcpIp.CMD_PLC_REQ_MATERIAL);

                    if (!RxNetUse.ShareConnected(RxGlobals.PrinterProperties.RippedDataPath))
                    {
                        RxNetUse.DeleteShare(RxGlobals.PrinterProperties.RippedDataPath);
                        RxNetUse.OpenShare(RxGlobals.PrinterProperties.RippedDataPath, "radex", "radex");
                    }

                    //--- handling messages --------------------------------------------
                    while (_Running && Connected)
                    {
                        try
                        {
                            read = _Stream.Read(buffer, 0, size);
                            if (read <= 0) break;
                            len = BitConverter.ToInt32(buffer, 0);
                            if (len>buffer.Length)
                            {
                                RxGlobals.Events.AddItem(new LogItem("Receive Buffer VAL_OVERFLOW"));
                                while (len>0)
                                {
                                    read=_Stream.Read(buffer, 0, buffer.Length);
                                    len -= buffer.Length;
                                }
                            }
                            else read = _Stream.Read(buffer, size, len - size);
                        }
                        catch (Exception e)
                        {
                            if (_Running) _log.AddItem(new LogItem() { Error = 1010, Message = e.Message });
                            break;
                        }
                        msgHandler.handle_message(buffer);  
                    }
                //  Console.WriteLine("Main Disconnected");
                }                 
                if (_Running) Connected = false;
                _Client.Close();
            };
         //   Marshal.FreeHGlobal(phdr);
        }

        //--- SendCommand ------------------------------------------------------------
        public void SendCommand(UInt32 cmd)
        {
            TcpIp.SMsgHdr hdr=new TcpIp.SMsgHdr();

            if (Connected)
            {
                hdr.msgLen = Marshal.SizeOf(hdr);
                hdr.msgId = cmd;
                byte[] buffer;
                RxStructConvert.ToBuffer<TcpIp.SMsgHdr>(out buffer, hdr);
                try
                {
                    _Stream.Write(buffer, 0, hdr.msgLen);
                }
                catch (Exception e)
                {
                    Console.WriteLine("TCP/IP Send Exception: >>{0}<<", e.Message);
                    if (_Running) Connected = false;
                    _Client.Close();
                }
            }
        }

        //--- SendMsg ----------------------------------------
        public void SendMsg<type>(UInt32 msgId, ref type msg)
        {
            if (Connected)
            {
                byte[] buffer;
                RxStructConvert.ToBuffer<type>(out buffer, msg);
                RxStructConvert.OverwriteBuffer(buffer, new TcpIp.SMsgHdr(){msgId=msgId, msgLen=buffer.Count()});

                try
                {
                    _Stream.Write(buffer, 0, buffer.Count());            
                }
                catch (Exception e)
                {
                    Console.WriteLine("TCP/IP Send Exception: >>{0}<<", e.Message);
                    Connected = false;
                    _Client.Close();
                }
            }
        }

        //--- SendMsgBuf ----------------------------------------
        public void SendMsgBuf(UInt32 msgId, string str)
        {
            if (Connected)
            {
                char[] msg = new char[str.Length];
                str.CopyTo(0, msg, 0, str.Length);
            //    RxGlobals.RxInterface.SendMsgBuf(TcpIp.CMD_PLC_GET_VAR, buf);         
                int hdrsize  = Marshal.SizeOf(typeof(TcpIp.SMsgHdr));
                byte[] buffer = new byte[hdrsize+msg.Length+1]; // header + string + "/0"
                for (int i=0; i<msg.Length; i++)
                    buffer[hdrsize+i] = (byte)msg[i];
                RxStructConvert.OverwriteBuffer(buffer, new TcpIp.SMsgHdr(){msgId=msgId, msgLen=buffer.Count()});
                try
                {
                    _Stream.Write(buffer, 0, buffer.Count());            
                }
                catch (Exception e)
                {
                    Console.WriteLine("TCP/IP Send Exception: >>{0}<<", e.Message);
                    Connected = false;
                    _Client.Close();
                }
            }
        }
    }
}
