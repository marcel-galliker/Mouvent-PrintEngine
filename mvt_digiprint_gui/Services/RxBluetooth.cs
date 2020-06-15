using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using InTheHand.Net.Sockets;
using InTheHand.Net.Bluetooth;
using InTheHand.Net;
using System.IO;
using RX_DigiPrint.Models;
using System.Runtime.InteropServices;
using RX_Common;
using System.Collections.ObjectModel;
using RX_DigiPrint.Helpers;
using System.Windows;

// https://github.com/inthehand/32feet/wiki
// https://github.com/inthehand/32feet/wiki/Creating-Records

namespace RX_DigiPrint.Services
{
    public class RxBluetooth : RxBindable
    {
        private Visibility _bluetoothVisibility;
        public Visibility BluetoothVisibility
        {
            get
            {
                return _bluetoothVisibility;
            }
            set
            {
                _bluetoothVisibility = value;
                OnPropertyChanged("BluetoothVisibility");
            }
        }

        Guid _GUID = new Guid("{"+RxBtDef.MouventBlueToothGuid+"}");

        private static Random   _rnd = new Random();

        private static int _cnt = 0;
        private static bool _Running;

        private static Thread     _Thread;
        private BluetoothListener _btListener; //represent this server bluetooth device
        private RxBtMsgHandler    _msgHandler = new RxBtMsgHandler();


        List<RxBtClient>          _Clients = new List<RxBtClient>();

        EventHandler<BluetoothWin32AuthenticationEventArgs> handler;
        BluetoothWin32Authentication                        authenticator;


        //--- constructor ---------------------------------
        public RxBluetooth()
        {
            _cnt++;
            if (_cnt > 1) throw new Exception("Must not be initialzed more than once");

            if (BluetoothRadio.IsSupported)
            {
                _Thread = new Thread(_BluetoothThread);
                _Thread.Start();
                new Thread(_TimerThread).Start();

                handler         = new EventHandler<BluetoothWin32AuthenticationEventArgs>(PairRequests);
                authenticator   = new BluetoothWin32Authentication(handler);

                RxGlobals.PrinterStatus.PropertyChanged += PrinterStatusChanged;    
            }

            BluetoothVisibility = Visibility.Collapsed;
        }

        //--- Property IsSupported ---------------------------------------
        public bool IsSupported
        {
            get { return BluetoothRadio.IsSupported; }
        }        

        //--- DeviceVisible ---------------------------------------
        public void DeviceVisible()
        {     
            if (BluetoothRadio.PrimaryRadio!=null) BluetoothRadio.PrimaryRadio.Mode = RadioMode.Discoverable;
        }

        //--- PrinterStatusChanged -----------------------------------------
        private void PrinterStatusChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            foreach (var client in _Clients)
            {
                try
                {
                    _msgHandler.handle_bt_state_req(client);
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
            }
        }

        //--- PairRequests ----------------------------
        private void PairRequests(object sender, BluetoothWin32AuthenticationEventArgs e)
        {
            if (BluetoothRadio.PrimaryRadio!=null) BluetoothRadio.PrimaryRadio.Mode = RadioMode.Connectable;
            e.Confirm = true;
        }

        //--- Property Address ---------------------------------------
        public string Address
        {
            get 
            { 
                int i;
                string str=BluetoothRadio.PrimaryRadio.LocalAddress.ToString();
                for (i=2; i<16; i+=3) str = str.Insert(i, ":");
                return str; 
            }
        }
        

        //--- ShutDown -------------------
        public void ShutDown()
        {
            _Running = false;
            if (_btListener!=null) _btListener.Stop();
            while (_Clients.Count()>0)
            {
                _Clients[0].close();
                _Clients.RemoveAt(0);
            }
        }

        //--- _BluetoothThread ----------------------------
        private void _BluetoothThread()
        {
            _Running = true;
            // Console.WriteLine("Listener Started!");
            while (_Running) 
            {
                Thread.Sleep(1000);
                try
                {
                    _btListener = new BluetoothListener(_GUID);
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.Message);
                }
                if (_btListener!=null)
                { 
                    _btListener.Start();
                    //TODO: if there is no connection
                    // accepting
                    while(_Running && _btListener!=null)
                    {
                        try
                        {
                            // Console.WriteLine("Accepting incoming connection….");
                            BluetoothClient client = new BluetoothClient();
                            client = _btListener.AcceptBluetoothClient();
                            if (client.Connected)
                            {
                                // Console.WriteLine("Bluetooth: >>"+client.RemoteMachineName+"<< Connected");
                                // RxGlobals.Events.AddItem(new LogItem("Bluetooth: >>"+client.RemoteMachineName+"<< Connected"));                    
                                Thread thread= new Thread(_ClientThread);
                                RxBtClient      rxClient = new RxBtClient(client);
                                _Clients.Add(rxClient);
                                thread.Start(rxClient);
                            }
                        }
                        catch (Exception e)
                        {
                            Console.WriteLine("Exception {0}", e.Message);
                        }                    
                    }
                }
            }
        }

        //--- _TimerThread -----------------------------------------
        private void _TimerThread()
        {
            while (_Running)
            {
                Thread.Sleep(1000);
                if (RxGlobals.MainWindow != null && !RxGlobals.MainWindow.IsVisible)
                {
                    ShutDown();
                }
            }
        }
    
        //--- _ClientThread ------------------------------
        private void _ClientThread(object par)
        {
            RxBtClient      rxClient = par as RxBtClient;
            byte[] buffer = new Byte[2048];            
            Int32 read, len;
            int size = 4;
            NetworkStream stream = rxClient.Stream;

            // RxGlobals.Events.AddItem(new LogItem("Bluetooth: >>"+rxClient.RemoteMachineName+"<< Connected"));                    
            while (_Running && rxClient.Connected)
            {
                try
                {
                    read = stream.Read(buffer, 0, size);
                    if (read <= 0) 
                    {
                        // RxGlobals.Events.AddItem(new LogItem("Bluetooth: >>"+rxClient.RemoteMachineName+"<< Closed 1")); 
                        rxClient.close();
                        break;
                    }
                    len = BitConverter.ToInt32(buffer, 0);
                    read = stream.Read(buffer, size, len - size);

                  //  string result = System.Text.Encoding.UTF8.GetString(buffer, size+4, len - size-4);
                  //  Console.WriteLine(client.RemoteMachineName + " : " + result);
                  //  RxGlobals.Events.AddItem(new LogItem("Bluetooth received "+len+ "Bytes >>" + result + "<<" ));
                    _msgHandler.handle_message(buffer, rxClient);
                }
                catch (Exception ex)
                {
                    // RxGlobals.Events.AddItem(new LogItem("Bluetooth: >>"+client.RemoteMachineName+"<< Closed 2")); 
                    Console.WriteLine("There is an error while listening connection");
                    Console.WriteLine(ex.Message);
                }
            }
            _Clients.Remove(rxClient);
            try
            {
                RxGlobals.Events.AddItem(new LogItem("Bluetooth Logout: {0}", rxClient.UserName));
                RxBindable.Invoke(()=>Users.Remove(rxClient.UserName));
                _update_Users();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
            };
        }

        //--- GenerateConnectionId -------------
        public void GenerateConnectionId()
        {
            ConnectionId = _rnd.Next(1, 1000000).ToString();
        }

        //--- Property ConnectionId ---------------------------------------
        private string _ConnectionId;
        public string ConnectionId
        {
            get { return _ConnectionId; }
            private set { SetProperty(ref _ConnectionId, value); }
        }

        //--- Property IsConnected ---------------------------------------
        private bool _IsConnected;
        public bool IsConnected
        {
            get { return _IsConnected; }
            set { SetProperty(ref _IsConnected, value); }
        }

        //--- Property Users ---------------------------------------
        private ObservableCollection<string> _Users = new ObservableCollection<string>();
        public ObservableCollection<string> Users
        {
            get { return _Users; }
            set { SetProperty(ref _Users, value); }
        }

        //--- Property UserSte ---------------------------------------
        private string _UserStr;
        public string UserStr
        {
            get { return _UserStr; }
            private set { SetProperty(ref _UserStr, value); }
        }        

        //--- _update_Users ---------------------
        private void _update_Users()
        {
            switch (Users.Count)
            {
                case 0:  IsConnected = false; UserStr=""; break;
                case 1:  IsConnected = true;  UserStr=Users[0]; break;
                default: IsConnected = true;  UserStr=Users[0]+" ..."; break;
            }
        }

        //--- LogIn ----------------------------------------------
        public  bool LogIn(RxBtDef.SLogInMsg msg, RxBtClient client)
        {
            // RxGlobals.Events.AddItem(new LogItem("Bluetooth Login: LoginID="+msg.connectionId.ToString()+" ("+ConnectionId.ToString()+")"){LogType=ELogType.eErrCont}); 
            
            if (!msg.connectionId.Equals(ConnectionId)) 
            {
                RxGlobals.Events.AddItem(new LogItem("Bluetooth Login with invalid LoginID"){LogType=ELogType.eErrCont}); 
                return false;
            }
            if (msg.userName.Equals(""))
            {
                RxGlobals.Events.AddItem(new LogItem("Bluetooth Login without USER NAME"){LogType=ELogType.eErrCont}); 
                return false;
            }
            RxGlobals.License.Code = msg.license;
            if (true || RxGlobals.License.Valid)
            {
                RxGlobals.Events.AddItem(new LogItem("Bluetooth Login: {0}", msg.userName));
                Users.Insert(0, msg.userName);
                _update_Users();
                if (RxGlobals.BluetoothLoginWnd!=null) RxBindable.Invoke(()=>RxGlobals.BluetoothLoginWnd.Close());
                _msgHandler.handle_bt_state_req(client);
                {
                    int i;
                    for(i=0; i<RxGlobals.Events.List.Count; i++) RxGlobals.Events.List[i].SendToBluetooth();
                }
                return true;
            }
            return false;
        }

        //--- SendCommand ------------------
        public void SendCommand(UInt32 cmd)
        {
            foreach(RxBtClient client in _Clients) client.SendCommand(cmd);
        }

        //--- SendMsg ----------------------------------------
        public void SendMsg<type>(UInt32 msgId, ref type msg)
        {
      //      byte[] buffer;
      //      RxStructConvert.ToBuffer<type>(out buffer, msg);
      //      RxStructConvert.OverwriteBuffer(buffer, new TcpIp.SMsgHdr(){msgId=msgId, msgLen=buffer.Count()});
            foreach(RxBtClient client in _Clients) client.SendMsg(msgId, ref msg);
        }
        
        //--- SendMsgBuf ----------------------------------------
        public void SendMsgBuf(UInt32 msgId, byte[] data)
        {
            foreach(RxBtClient client in _Clients) client.SendMsgBuf(msgId, data);
        }

        public void SetVisibility()
        {
            if (BluetoothVisibility == Visibility.Collapsed)
            {
                BluetoothVisibility = Visibility.Visible;
            }
            else
            {
                BluetoothVisibility = Visibility.Collapsed;
            }
        }
    }
}

