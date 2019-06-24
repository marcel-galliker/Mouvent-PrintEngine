using RX_Common;
using RX_DigiPrint.Models;
using System;
using System.Collections.ObjectModel;
using System.Runtime.InteropServices;
using System.Text;

namespace RX_DigiPrint.External
{

    class RX_Wlan
    {
        public enum WLAN_INTERFACE_STATE : uint
        {
            wlan_interface_state_not_ready,
            wlan_interface_state_connected,
            wlan_interface_state_ad_hoc_network_formed,
            wlan_interface_state_disconnecting,
            wlan_interface_state_disconnected,
            wlan_interface_state_associating,
            wlan_interface_state_discovering,
            wlan_interface_state_authenticating
        };

        [StructLayout(LayoutKind.Sequential, Pack = 1, CharSet = CharSet.Unicode)]
        public struct WLAN_INTERFACE_INFO
        {
            public Guid InterfaceGuid;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string strInterfaceDescription;
            public WLAN_INTERFACE_STATE isState;
        };

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct WLAN_INTERFACE_INFO_LIST 
        {
            public UInt32 dwNumberOfItems;
            public UInt32 dwIndex;
            
            public WLAN_INTERFACE_INFO list;
        };

        public struct DOT11_SSID
        {
            public UInt32 uSSIDLength;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
            public byte[] ucSSID;
        }

        public enum DOT11_BSS_TYPE
        {
            dot11_BSS_type_infrastructure = 1,
            dot11_BSS_type_independent = 2,
            dot11_BSS_type_any = 3,
        }
        
        public enum DOT11_AUTH_ALGORITHM : uint
        { 
          dot11_auth_algo_80211_open        = 1,
          dot11_auth_algo_80211_shared_key  = 2,
          dot11_auth_algo_wpa               = 3,
          dot11_auth_algo_wpa_psk           = 4,
          dot11_auth_algo_wpa_none          = 5,
          dot11_auth_algo_rsna              = 6,
          dot11_auth_algo_rsna_psk          = 7,
          dot11_auth_algo_ihv_start         = 0x80000000,
          dot11_auth_algo_ihv_end           = 0xffffffff
        };

        public enum DOT11_CIPHER_ALGORITHM : uint
        { 
          dot11_cipher_algo_none           = 0x00,
          dot11_cipher_algo_wep40          = 0x01,
          dot11_cipher_algo_tkip           = 0x02,
          dot11_cipher_algo_ccmp           = 0x04,
          dot11_cipher_algo_wep104         = 0x05,
          dot11_cipher_algo_wpa_use_group  = 0x100,
          dot11_cipher_algo_rsn_use_group  = 0x100,
          dot11_cipher_algo_wep            = 0x101,
          dot11_cipher_algo_ihv_start      = 0x80000000,
          dot11_cipher_algo_ihv_end        = 0xffffffff
        };

        public enum DOT11_PHY_TYPE : uint
        { 
            dot11_phy_type_unknown     = 0,
            dot11_phy_type_any         = 0,
            dot11_phy_type_fhss        = 1,
            dot11_phy_type_dsss        = 2,
            dot11_phy_type_irbaseband  = 3,
            dot11_phy_type_ofdm        = 4,
            dot11_phy_type_hrdsss      = 5,
            dot11_phy_type_erp         = 6,
            dot11_phy_type_ht          = 7,
            dot11_phy_type_vht         = 8,
            dot11_phy_type_IHV_start   = 0x80000000,
            dot11_phy_type_IHV_end     = 0xffffffff
        };

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct WLAN_AVAILABLE_NETWORK
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string strProfileName; // WCHAR[256]
            public DOT11_SSID dot11Ssid;
            public DOT11_BSS_TYPE dot11BssType;
            public uint uNumberOfBssids; // ULONG
            public bool bNetworkConnectable; // BOOL
            public uint wlanNotConnectableReason; // WLAN_REASON_CODE
            public uint uNumberOfPhyTypes; // ULONG
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
            public DOT11_PHY_TYPE[] dot11PhyTypes;
            public bool bMorePhyTypes; // BOOL
            public uint wlanSignalQuality; // WLAN_SIGNAL_QUALITY
            public bool bSecurityEnabled; // BOOL
            public DOT11_AUTH_ALGORITHM dot11DefaultAuthAlgorithm;
            public DOT11_CIPHER_ALGORITHM dot11DefaultCipherAlgorithm;
            public uint dwFlags; // DWORD
            public uint dwReserved; // DWORD
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        struct WLAN_AVAILABLE_NETWORK_LIST
        {
            internal   uint dwNumberOfItems;
            internal   uint dwIndex;
            internal   WLAN_AVAILABLE_NETWORK[] wlanAvailableNetwork;

            internal WLAN_AVAILABLE_NETWORK_LIST(IntPtr ppAvailableNetworkList)
            {
                if (ppAvailableNetworkList==IntPtr.Zero)
                {
                    dwNumberOfItems = 0;
                    dwIndex = 0;
                    wlanAvailableNetwork = new WLAN_AVAILABLE_NETWORK[dwNumberOfItems];
                }
                else
                { 
                    dwNumberOfItems = (uint)Marshal.ReadInt32(ppAvailableNetworkList);
                    dwIndex = (uint)Marshal.ReadInt32(ppAvailableNetworkList, 4);
                    wlanAvailableNetwork = new WLAN_AVAILABLE_NETWORK[dwNumberOfItems];

                    for (int i = 0; i < dwNumberOfItems; i++)
                    {
                       IntPtr pWlanAvailableNetwork = new IntPtr(ppAvailableNetworkList.ToInt64() + i * Marshal.SizeOf(typeof(WLAN_AVAILABLE_NETWORK)) + 8);
                        wlanAvailableNetwork[i] = (WLAN_AVAILABLE_NETWORK)Marshal.PtrToStructure(pWlanAvailableNetwork, typeof(WLAN_AVAILABLE_NETWORK));
                    }
                }
             }
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        public struct WLAN_PROFILE_INFO
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
            public string strProfileName;
            public uint dwFlags;
        }

        public struct WLAN_PROFILE_INFO_LIST
        {
            public uint dwNumberOfItems;
            public uint dwIndex;
            public WLAN_PROFILE_INFO[] ProfileInfo;

            public WLAN_PROFILE_INFO_LIST(IntPtr ppProfileList)
            {
                dwNumberOfItems = (uint)Marshal.ReadInt32(ppProfileList);
                dwIndex = (uint)Marshal.ReadInt32(ppProfileList, 4);
                ProfileInfo = new WLAN_PROFILE_INFO[dwNumberOfItems];
                IntPtr ppProfileListTemp = new IntPtr(ppProfileList.ToInt32() + 8);

                for (int i = 0; i < dwNumberOfItems; i++)
                {
                    ppProfileList = new IntPtr(ppProfileListTemp.ToInt32() + i * Marshal.SizeOf(typeof(WLAN_PROFILE_INFO)));
                    ProfileInfo[i] = (WLAN_PROFILE_INFO)Marshal.PtrToStructure(ppProfileList, typeof(WLAN_PROFILE_INFO));
                }
            }
        }
          
        [DllImport("Wlanapi", EntryPoint = "WlanFreeMemory")]
        private static extern void WlanFreeMemory([In] IntPtr pMemory);

        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct WLAN_NETWORK_INFO
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
	        public string ssid;
	        public Int32  bssType;
	        public UInt32 signalQuality;
        };
        
        public enum WLAN_CONNECTION_MODE
        {
            wlan_connection_mode_profile,
            wlan_connection_mode_temporary_profile,
            wlan_connection_mode_discovery_secure,
            wlan_connection_mode_discovery_unsecure,
            wlan_connection_mode_auto,
            wlan_connection_mode_invalid,
        };

        public struct NDIS_OBJECT_HEADER
        {
            byte Type;
            byte Revision;
            ushort Size;
        };

        public struct DOT11_BSSID_LIST
        {
            NDIS_OBJECT_HEADER Header;
            ulong uNumOfEntries;
            ulong uTotalNumOfEntries;
            IntPtr BSSIDs;
        };

        [StructLayout(LayoutKind.Sequential,CharSet=CharSet.Unicode)]
        public struct WLAN_CONNECTION_PARAMETERS
        {
            public WLAN_CONNECTION_MODE wlanConnectionMode;
            public string strProfile;
            public DOT11_SSID[] pDot11Ssid;
            public DOT11_BSSID_LIST[] pDesiredBssidList;
            public DOT11_BSS_TYPE dot11BssType;
            public uint dwFlags;
        };

        [Flags]
        public enum WlanProfileFlags
        {
            AllUser,
            GroupPolicy,
            User
        }

        [Flags]
        public enum WlanAccess
        {
            ExecuteAccess = 0x20021,
            ReadAccess = 0x20001,
            WriteAccess = 0x70023
        }

        [DllImport("wlanapi.dll")]
        public static extern int WlanGetProfile([In] IntPtr clientHandle, [In, MarshalAs(UnmanagedType.LPStruct)] Guid interfaceGuid, [In, MarshalAs(UnmanagedType.LPWStr)] string profileName, [In] IntPtr pReserved, out IntPtr profileXml, [Optional] out WlanProfileFlags flags, [Optional] out WlanAccess grantedAccess);

        [DllImport("wlanapi.dll")]
        public static extern int WlanReasonCodeToString([In] UInt32 reasonCode, [In] int bufferSize, [In, Out] StringBuilder stringBuffer, IntPtr pReserved);

        //--- _wlan_open ------------------------------------
        [DllImport("Wlanapi.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int WlanOpenHandle(int dwClientVersion, IntPtr pReserved, out uint pdwNegotiatedVersion, out IntPtr ClientHandle);

        private static IntPtr _wlan_open()
        {
            #if X64
                IntPtr handle;
                uint version;
                int ret;
                ret = WlanOpenHandle(2, IntPtr.Zero, out version, out handle);
                return handle;
            #else
                return IntPtr.Zero;
            #endif
        }

        //--- _wlan_close ------------------------------------
        [DllImport("Wlanapi.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int  WlanCloseHandle(IntPtr phClientHandle, IntPtr rPeserved);

        private static void _wlan_close(IntPtr handle)
        {

            if (handle!=IntPtr.Zero) WlanCloseHandle(handle, IntPtr.Zero);
        }

        //--- _wchar_to_char ------------------------------------
        private static string _wchar_to_char(IntPtr ptr, int offset, int size)
        {
            int i;
            char[] str = new char[size];
            for (i=0; i<size; i++)
            {
                str[i] = (char)Marshal.ReadByte(ptr, offset+2*i);
            }
            return new string(str).TrimEnd('\0');
        }

        //--- get_interfaces ----------------------------------------
        [DllImport("Wlanapi.dll", CallingConvention = CallingConvention.Cdecl)]
        private static extern int WlanEnumInterfaces(IntPtr hClientHandle, IntPtr pReserved, out IntPtr ppInterfaceList);

        public static ObservableCollection<WLAN_INTERFACE_INFO> get_interfaces()
        {
            IntPtr handle = _wlan_open();
            if (handle==IntPtr.Zero) return null;

            ObservableCollection<WLAN_INTERFACE_INFO> interfaces = new ObservableCollection<WLAN_INTERFACE_INFO>();
            IntPtr pList;
            int ret = WlanEnumInterfaces(handle, IntPtr.Zero, out pList);

            if (pList!=null)
            {
                Int64 read=0;
                //--- interprete data -------------------
                // The first 4 bytes are the number of WLAN_INTERFACE_INFO structures.
                Int32 dwNumberofItems = Marshal.ReadInt32(pList, 0);                
                read+=4;

                // The next 4 bytes are the index of the current item in the unmanaged API.
                Int32 dwIndex = Marshal.ReadInt32(pList, 4);
                read+=4;

                for (int i = 0; i < dwNumberofItems; i++)
                {
                    // The offset of the array of structures is 8 bytes past the beginning.
                    // Then, take the index and multiply it by the number of bytes in the
                    // structure.
                    // the length of the WLAN_INTERFACE_INFO structure is 532 bytes - this
                    // was determined by doing a sizeof(WLAN_INTERFACE_INFO) in an
                    // unmanaged C++ app.
                    IntPtr pItemList = new IntPtr(pList.ToInt64() + read);
 
                    // Construct the WLAN_INTERFACE_INFO structure, marshal the unmanaged
                    // structure into it, then copy it to the array of structures.

                    WLAN_INTERFACE_INFO info = new WLAN_INTERFACE_INFO();
                    info = (WLAN_INTERFACE_INFO)Marshal.PtrToStructure(pItemList, typeof(WLAN_INTERFACE_INFO));
                    interfaces.Add(info);
                }
                WlanFreeMemory(pList);
            }
            _wlan_close(handle);

            return interfaces;            
        }
        
        //--- get_networks ----------------------------------------
        [DllImport("Wlanapi.dll", SetLastError = true)]
        private static extern uint WlanGetAvailableNetworkList(IntPtr hClientHandle, ref Guid pInterfaceGuid, uint dwFlags, IntPtr pReserved, ref IntPtr ppAvailableNetworkList);
        private const uint WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES = 0x00000001;
        private const uint WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES = 0x00000002;

        public static ObservableCollection<WlanNetwork> get_networks(Guid ifGuid, out int connected)
        {
            ObservableCollection<WlanNetwork> networks = new ObservableCollection<WlanNetwork>();
            
            connected = -1;
            IntPtr handle = _wlan_open();
            if (handle!=IntPtr.Zero)
            {
                IntPtr ppAvailableNetworkList = new IntPtr();
                Guid pInterfaceGuid = ifGuid;
                WlanGetAvailableNetworkList(handle, ref pInterfaceGuid, WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_MANUAL_HIDDEN_PROFILES, IntPtr.Zero, ref ppAvailableNetworkList);
                WLAN_AVAILABLE_NETWORK_LIST wlanAvailableNetworkList = new WLAN_AVAILABLE_NETWORK_LIST(ppAvailableNetworkList);
                WlanFreeMemory(ppAvailableNetworkList);
                    for (int j = 0; j < wlanAvailableNetworkList .dwNumberOfItems; j++)
                    {
                        WLAN_AVAILABLE_NETWORK network = wlanAvailableNetworkList.wlanAvailableNetwork[j];
                        networks.Add(new WlanNetwork()
                        {
                            Name            = System.Text.Encoding.UTF8.GetString(network.dot11Ssid.ucSSID).TrimEnd('\0'),
                            HasProfile      = !network.strProfileName.Equals(""),
                            SignalQuality   = network.wlanSignalQuality,
                            Connected       = (network.dwFlags&1)!=0, 
                        });
                        if (networks[j].Connected) connected=j;
                        /*
                        Console.WriteLine("Available Network: ");
                        Console.WriteLine("SSID: " + network.dot11Ssid.ucSSID);
                        Console.WriteLine("Encrypted: " + network.bSecurityEnabled);
                        Console.WriteLine("Signal Strength: " + network.wlanSignalQuality);
                        Console.WriteLine("Default Authentication: " + network.dot11DefaultAuthAlgorithm.ToString());
                        Console.WriteLine("Default Cipher: " + network.dot11DefaultCipherAlgorithm.ToString());
                        Console.WriteLine();
                        */
                    }
            }
            _wlan_close(handle);
            return networks;            
        }

        //--- get_profiles ----------------------------------------
        [DllImport("Wlanapi.dll", SetLastError = true)]
        private static extern uint WlanGetProfileList(IntPtr hClientHandle,ref Guid pInterfaceGuid,IntPtr pReserved, ref IntPtr ppProfileList);

        public static ObservableCollection<string> get_profiles(Guid ifGuid)
        {
            ObservableCollection<string> profiles = new ObservableCollection<string>();

            IntPtr handle = _wlan_open();
            if (handle!=IntPtr.Zero)
            {
                IntPtr ppProfileList = new IntPtr();
                WlanGetProfileList(handle, ref ifGuid, new IntPtr(), ref ppProfileList);
                WLAN_PROFILE_INFO_LIST wlanProfileInfoList = new WLAN_PROFILE_INFO_LIST(ppProfileList);
            }
            _wlan_close(handle);
            return profiles;            
        }

        //--- set_profile --------------------------------
        
        [DllImport("wlanapi.dll")]
        private static extern int WlanSetProfile([In] IntPtr clientHandle, [In, MarshalAs(UnmanagedType.LPStruct)] Guid interfaceGuid, [In] WlanProfileFlags flags, [In, MarshalAs(UnmanagedType.LPWStr)] string profileXml, [In, Optional, MarshalAs(UnmanagedType.LPWStr)] string allUserProfileSecurity, [In] bool overwrite, [In] IntPtr pReserved, out UInt32 reasonCode);

        static public void set_profile(Guid guid, string name, string password)
        {
            IntPtr handle = _wlan_open();
            if (handle!=IntPtr.Zero)
            {
                UInt32 reasonCode=0;
                int ret;

                string xml = "<?xml version=\"1.0\"?>\n"
                           + "<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">\n"
                           + string.Format("    <name>{0}</name>\n", name)
	                       + "    <SSIDConfig>\n"
	                       + "        <SSID>\n"
	                       + string.Format("            <name>{0}</name>\n", name)
	                       + "        </SSID>\n"
	                       + "        <nonBroadcast>false</nonBroadcast>\n"
	                       + "    </SSIDConfig>\n"
	                       + "    <connectionType>ESS</connectionType>\n"
	                       + "    <connectionMode>auto</connectionMode>\n"
	                       + "    <MSM>\n"
	                       + "        <security>\n"
	                       + "            <authEncryption>\n"
	                       + "	            <authentication>WPAPSK</authentication>\n"
	                       + "		        <encryption>AES</encryption>\n"
                           + "	            <useOneX>false</useOneX>\n"
	                       + "            </authEncryption>\n"
	                       + "            <sharedKey>\n"
	                       + "	            <keyType>passPhrase</keyType>\n"
	                       + "	            <protected>false</protected>\n"
	                       + string.Format("	            <keyMaterial>{0}</keyMaterial>\n", password)
	                       + "            </sharedKey>\n"
	                       + "        </security>\n"
	                       + "    </MSM>\n"
 	                       + "</WLANProfile>\n";

                ret = WlanSetProfile(handle, guid, WlanProfileFlags.AllUser, xml, null, true, IntPtr.Zero, out reasonCode);

                if (ret!=0)
                {
                    StringBuilder reasonStr = new StringBuilder(0x400);
                    WlanReasonCodeToString(reasonCode, reasonStr.Capacity, reasonStr, IntPtr.Zero);
                    Console.WriteLine("WlanSetProfile Error >>{0}<<", reasonStr.Length);
                }
            }
            _wlan_close(handle);
        }

        //--- del_profile --------------------------------
        [DllImport("Wlanapi.dll", SetLastError = true)]
        private static extern uint WlanDeleteProfile(IntPtr hClientHandle,ref Guid pInterfaceGuid, string strProfileName, IntPtr pReserved);

        static public void del_profile(Guid guid, string name)
        {
            IntPtr handle = _wlan_open();
            if (handle!=IntPtr.Zero) WlanDeleteProfile(handle, ref guid, name, IntPtr.Zero);
            _wlan_close(handle);
        }

        //--- Connect --------------------------------
        [DllImport("Wlanapi.dll", SetLastError = true)]
        private static extern uint WlanConnect(IntPtr hClientHandle,ref Guid pInterfaceGuid, ref WLAN_CONNECTION_PARAMETERS pConnectionParameters, IntPtr pReserved);

        static public void Connect(Guid guid, string network)
        {
            IntPtr handle = _wlan_open();
            if (handle!=IntPtr.Zero)
            {
                WLAN_CONNECTION_PARAMETERS wlanConnectionParameters = new WLAN_CONNECTION_PARAMETERS();
                wlanConnectionParameters.dot11BssType = DOT11_BSS_TYPE.dot11_BSS_type_any;
                wlanConnectionParameters.dwFlags = 0;
                wlanConnectionParameters.strProfile = network;
                wlanConnectionParameters.wlanConnectionMode = WLAN_CONNECTION_MODE.wlan_connection_mode_profile;
                WlanConnect(handle,ref guid,ref wlanConnectionParameters, IntPtr.Zero);
            }
            _wlan_close(handle);
        }

        //--- Disconnect --------------------------------
        [DllImport("Wlanapi.dll", SetLastError = true)]
        private static extern uint  WlanDisconnect(IntPtr hClientHandle,ref Guid pInterfaceGuid,IntPtr pReserved);

        static public void Disconnect(Guid guid)
        {
            IntPtr handle = _wlan_open();
            if (handle!=IntPtr.Zero) WlanDisconnect(handle, ref guid, IntPtr.Zero);
            _wlan_close(handle);
        }
    }
}
