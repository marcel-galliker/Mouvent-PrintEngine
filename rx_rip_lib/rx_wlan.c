// ****************************************************************************
//
//	rx_wlan.c
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#ifdef linux
	int rx_wlan_get_networks(char *networks, int size)
	{
		*networks = 0;
		return 1;
	}
#else

#include <windows.h>
#include <wlanapi.h>

#include <stdio.h>
#include <stdlib.h>

#include "rx_def.h"
#include "rx_wlan.h"

//--- rx_wlan_get_interfaces -----------------------------------------
int rx_wlan_get_interfaces(WLAN_INTERFACE_INFO *interfaces, int size)
{
    // Declare and initialize variables.

    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2;   //    
    DWORD dwCurVersion = 0;
    DWORD dwResult = 0;
    int iRet = 0;
	int cnt=0;
    
    WCHAR GuidString[40] = {0};
     
    int i;

    /* variables used for WlanEnumInterfaces  */
    
    PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
    PWLAN_INTERFACE_INFO pIfInfo = NULL;
    
    dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient); 
    if (dwResult != ERROR_SUCCESS)  
	{
        wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
        // FormatMessage can be used to find out why the function failed
        goto END;
    }
    
    dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList); 
    if (dwResult != ERROR_SUCCESS)  
	{
        wprintf(L"WlanEnumInterfaces failed with error: %u\n", dwResult);
        // FormatMessage can be used to find out why the function failed
        goto END;
    }
    else 
	{
        wprintf(L"Num Entries: %lu\n", pIfList->dwNumberOfItems);
        wprintf(L"Current Index: %lu\n", pIfList->dwIndex);
		cnt = (int) pIfList->dwNumberOfItems;
		if (cnt>size) cnt=size;
        for (i = 0; i < cnt; i++) 
		{
            pIfInfo = (WLAN_INTERFACE_INFO *) &pIfList->InterfaceInfo[i];
			memcpy(&interfaces[i], pIfInfo, sizeof(interfaces[i]));
//			memcpy(&interfaces[i].InterfaceGuid, &pIfInfo->InterfaceGuid, sizeof(interfaces[i].InterfaceGuid));
			wchar_to_char((const wchar_t*) &pIfInfo->strInterfaceDescription, (char*)&interfaces[i].strInterfaceDescription, SIZEOF(interfaces[i].strInterfaceDescription)-1);
//			interfaces[i].isState = pIfInfo->isState;

            wprintf(L"  Interface Index[%d]:\t %lu\n", i, i);
            iRet = StringFromGUID2(&pIfInfo->InterfaceGuid, (LPOLESTR) &GuidString, 39);
			
			if (iRet == 0) wprintf(L"StringFromGUID2 failed\n");
            else		   wprintf(L"  InterfaceGUID[%d]: %ws\n",i, GuidString);    
            wprintf(L"  Interface Description[%d]: %ws", i,		pIfInfo->strInterfaceDescription);

            wprintf(L"\n");
            wprintf(L"  Interface State[%d]:\t ", i);
            switch (pIfInfo->isState) 
			{
            case wlan_interface_state_not_ready:				wprintf(L"Not ready\n");													break;
            case wlan_interface_state_connected:				wprintf(L"Connected\n");													break;
            case wlan_interface_state_ad_hoc_network_formed:	wprintf(L"First node in a ad hoc network\n");								break;
            case wlan_interface_state_disconnecting:			wprintf(L"Disconnecting\n");												break;
            case wlan_interface_state_disconnected:				wprintf(L"Not connected\n");												break;
            case wlan_interface_state_associating:				wprintf(L"Attempting to associate with a network\n");						break;
            case wlan_interface_state_discovering:				wprintf(L"Auto configuration is discovering settings for the network\n");	break;
            case wlan_interface_state_authenticating:			wprintf(L"In process of authenticating\n");									break;
            default:											wprintf(L"Unknown state %ld\n", pIfInfo->isState);							break;
            }
            wprintf(L"\n");
        }
    }
END:
    if (pIfList != NULL) 
	{
        WlanFreeMemory(pIfList);
        pIfList = NULL;
    }
	if (hClient)
	{
		WlanCloseHandle(hClient, NULL);
		hClient=NULL;
	}
    return cnt;
}

//--- rx_wlan_get_networks -----------------------------------------------------
int rx_wlan_get_networks(GUID guid, char *networks, int size, int *pconnected)
{
	int trace = FALSE;

    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2;
    DWORD dwCurVersion = 0;
    DWORD dwResult = 0;
    DWORD dwRetVal = 0;
    int iRet = 0;
	int len=0, l, j;
	int cnt=0;
    
    /* variables used for WlanEnumInterfaces  */

    PWLAN_AVAILABLE_NETWORK_LIST pBssList = NULL;
    PWLAN_AVAILABLE_NETWORK pBssEntry = NULL;
    
    int iRSSI = 0;
	*pconnected = -1;

    dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) 
	{
        wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
        return cnt;
        // You can use FormatMessage here to find out why the function failed
    }
    dwResult = WlanGetAvailableNetworkList(hClient,
										&guid,
                                        0, 
                                        NULL, 
                                        &pBssList);

    if (dwResult != ERROR_SUCCESS) 
	{
		wprintf(L"WlanGetAvailableNetworkList failed with error: %u\n", dwResult);
        dwRetVal = REPLY_ERROR;
        // You can use FormatMessage to find out why the function failed
    } 
	else
	{
		if (trace)
		{
			wprintf(L"WLAN_AVAILABLE_NETWORK_LIST for this interface\n");
			wprintf(L"  Num Entries: %lu\n\n", pBssList->dwNumberOfItems);
		}
		cnt = (int)pBssList->dwNumberOfItems;
		if (cnt>size) cnt=size;
        for (j = 0; j < cnt; j++) 
		{
            pBssEntry = (WLAN_AVAILABLE_NETWORK *) & pBssList->Network[j];

			l = pBssEntry->dot11Ssid.uSSIDLength;
			
			//--- save network info --------------------
			if (len+l+10>=size) goto END;
			if (len) networks[len++]='\n';
			len += sprintf(&networks[len], "%s\t%d\t%03d\t%03d", pBssEntry->dot11Ssid.ucSSID, pBssEntry->strProfileName[0]!=0, pBssEntry->dot11BssType, pBssEntry->wlanSignalQuality);
			printf("%s\n", &networks[len]);
			if (pBssEntry->dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED) *pconnected = j;

			if (trace)
			{
				wprintf(L"  Profile Name[%u]:  %ws\n", j, pBssEntry->strProfileName);
				wprintf(L"  SSID[%u]:\t\t ", j);

				wprintf(L"  BSS Network type[%u]:\t ", j);
				switch (pBssEntry->dot11BssType) 
				{
				case dot11_BSS_type_infrastructure:	wprintf(L"Infrastructure (%u)\n", pBssEntry->dot11BssType); break;
				case dot11_BSS_type_independent:	wprintf(L"Infrastructure (%u)\n", pBssEntry->dot11BssType);	break;
				default:							wprintf(L"Other (%lu)\n", pBssEntry->dot11BssType);			break;
				}
                                
				wprintf(L"  Number of BSSIDs[%u]:\t %u\n", j, pBssEntry->uNumberOfBssids);

				wprintf(L"  Connectable[%u]:\t ", j);
				if (pBssEntry->bNetworkConnectable) wprintf(L"Yes\n");
				else 
				{
					wprintf(L"No\n");
					wprintf(L"  Not connectable WLAN_REASON_CODE value[%u]:\t %u\n", j, pBssEntry->wlanNotConnectableReason);
				}        

				wprintf(L"  Number of PHY types supported[%u]:\t %u\n", j, pBssEntry->uNumberOfPhyTypes);
				if (pBssEntry->wlanSignalQuality == 0)			iRSSI = -100;
				else if (pBssEntry->wlanSignalQuality == 100)   iRSSI = -50;
				else											iRSSI = -100 + (pBssEntry->wlanSignalQuality/2);    
                        
				wprintf(L"  Signal Quality[%u]:\t %u (RSSI: %i dBm)\n", j, pBssEntry->wlanSignalQuality, iRSSI);

				wprintf(L"  Security Enabled[%u]:\t ", j);
				if (pBssEntry->bSecurityEnabled)	wprintf(L"Yes\n");
				else								wprintf(L"No\n");
                        
				wprintf(L"  Default AuthAlgorithm[%u]: ", j);
				switch (pBssEntry->dot11DefaultAuthAlgorithm) 
				{
				case DOT11_AUTH_ALGO_80211_OPEN:		wprintf(L"802.11 Open (%u)\n", pBssEntry->dot11DefaultAuthAlgorithm);	break;
				case DOT11_AUTH_ALGO_80211_SHARED_KEY:	wprintf(L"802.11 Shared (%u)\n", pBssEntry->dot11DefaultAuthAlgorithm); break;
				case DOT11_AUTH_ALGO_WPA:				wprintf(L"WPA (%u)\n", pBssEntry->dot11DefaultAuthAlgorithm);			break;
				case DOT11_AUTH_ALGO_WPA_PSK:			wprintf(L"WPA-PSK (%u)\n", pBssEntry->dot11DefaultAuthAlgorithm);		break;
				case DOT11_AUTH_ALGO_WPA_NONE:			wprintf(L"WPA-None (%u)\n", pBssEntry->dot11DefaultAuthAlgorithm);		break;
				case DOT11_AUTH_ALGO_RSNA:				wprintf(L"RSNA (%u)\n", pBssEntry->dot11DefaultAuthAlgorithm);			break;
				case DOT11_AUTH_ALGO_RSNA_PSK:			wprintf(L"RSNA with PSK(%u)\n", pBssEntry->dot11DefaultAuthAlgorithm);	break;
				default:								wprintf(L"Other (%lu)\n", pBssEntry->dot11DefaultAuthAlgorithm);		break;
				}
                        
				wprintf(L"  Default CipherAlgorithm[%u]: ", j);
				switch (pBssEntry->dot11DefaultCipherAlgorithm) 
				{
				case DOT11_CIPHER_ALGO_NONE:			wprintf(L"None (0x%x)\n", pBssEntry->dot11DefaultCipherAlgorithm);		break;
				case DOT11_CIPHER_ALGO_WEP40:			wprintf(L"WEP-40 (0x%x)\n", pBssEntry->dot11DefaultCipherAlgorithm);	break;
				case DOT11_CIPHER_ALGO_TKIP:			wprintf(L"TKIP (0x%x)\n", pBssEntry->dot11DefaultCipherAlgorithm);		break;
				case DOT11_CIPHER_ALGO_CCMP:			wprintf(L"CCMP (0x%x)\n", pBssEntry->dot11DefaultCipherAlgorithm);		break;
				case DOT11_CIPHER_ALGO_WEP104:			wprintf(L"WEP-104 (0x%x)\n", pBssEntry->dot11DefaultCipherAlgorithm);	break;
				case DOT11_CIPHER_ALGO_WEP:				wprintf(L"WEP (0x%x)\n", pBssEntry->dot11DefaultCipherAlgorithm);		break;
				default:								wprintf(L"Other (0x%x)\n", pBssEntry->dot11DefaultCipherAlgorithm);		break;
				}
			
				wprintf(L"  Flags[%u]:\t 0x%x", j, pBssEntry->dwFlags);
				if (pBssEntry->dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED) 	 wprintf(L" - Currently connected");
				if (pBssEntry->dwFlags & WLAN_AVAILABLE_NETWORK_HAS_PROFILE) wprintf(L" - Has profile");
				wprintf(L"\n");
                    
				wprintf(L"\n");
			}
        }
    }
END:
    if (pBssList != NULL) 
	{
        WlanFreeMemory(pBssList);
        pBssList = NULL;
    }

	if (hClient)
	{
		WlanCloseHandle(hClient, NULL);
		hClient=NULL;
	}

	return cnt;
}

//--- rx_wlan_get_profiles ---------------------------
int rx_wlan_get_profiles(GUID guid, char *profiles, int size)
{

// Declare and initialize variables.

    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2;      //    
    DWORD dwCurVersion = 0;
    DWORD dwResult = 0;
    DWORD dwRetVal = 0;
    int iRet = 0;
    
    WCHAR GuidString[39] = {0};

    unsigned int j;
	int len=0, l;

    /* variables used for WlanEnumInterfaces  */

    PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
    PWLAN_INTERFACE_INFO pIfInfo = NULL;

    PWLAN_PROFILE_INFO_LIST pProfileList = NULL;
    PWLAN_PROFILE_INFO pProfile = NULL;

	memset(profiles, 0, size);

    dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
        goto END;
        // You can use FormatMessage here to find out why the function failed
    }

    dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
    if (dwResult != ERROR_SUCCESS) 
	{
        wprintf(L"WlanEnumInterfaces failed with error: %u\n", dwResult);
        goto END;
        // You can use FormatMessage here to find out why the function failed
    } 
	else 
	{
        wprintf(L"WLAN_INTERFACE_INFO_LIST for this system\n");

        wprintf(L"Num Entries: %lu\n", pIfList->dwNumberOfItems);
        wprintf(L"Current Index: %lu\n", pIfList->dwIndex);

        dwResult = WlanGetProfileList(hClient, &guid, NULL, &pProfileList);

        if (dwResult != ERROR_SUCCESS) 
		{
            wprintf(L"WlanGetProfileList failed with error: %u\n",dwResult);
            dwRetVal = 1;
            // You can use FormatMessage to find out why the function failed
        } 
		else 
		{
            wprintf(L"WLAN_PROFILE_INFO_LIST for this interface\n");

            wprintf(L"  Num Entries: %lu\n\n", pProfileList->dwNumberOfItems);

            for (j = 0; j < pProfileList->dwNumberOfItems; j++) 
			{
                pProfile = (WLAN_PROFILE_INFO *) & pProfileList->ProfileInfo[j];

				//--- save name -----------------
				l=(int)wcslen(pProfile->strProfileName);
				if (len+l>=size) goto END;

				if (len) profiles[len++]= '\n';
				wchar_to_char(pProfile->strProfileName, &profiles[len], l+1);
				len +=l;
				// --------------------------

                wprintf(L"  Profile Name[%u]:  %ws\n", j, pProfile->strProfileName);
                    
                wprintf(L"  Flags[%u]:\t    0x%x", j, pProfile->dwFlags);
                if (pProfile->dwFlags & WLAN_PROFILE_GROUP_POLICY)	wprintf(L"   Group Policy");
                if (pProfile->dwFlags & WLAN_PROFILE_USER)			wprintf(L"   Per User Profile");
                wprintf(L"\n");    

                wprintf(L"\n");
            }
        }
    }
END:
    if (pProfileList != NULL) {
        WlanFreeMemory(pProfileList);
        pProfileList = NULL;
    }

    if (pIfList != NULL) {
        WlanFreeMemory(pIfList);
        pIfList = NULL;
    }

	if (hClient)
	{
		WlanCloseHandle(hClient, NULL);
		hClient=NULL;
	}

    return dwRetVal;
}

//--- rx_wlan_add_profile -------------------------------
int rx_wlan_add_profile	 (GUID guid, char *name, char *password)
{
    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2;
    DWORD dwCurVersion = 0;
    DWORD dwResult = 0;
	DWORD dwReason = 0;    
    unsigned int size, len=0;

	wchar_t wname[128];
	wchar_t wpassword[128]; 
	wchar_t wxml[2048];
	size = SIZEOF(wxml);

	char_to_wchar(name, wname, SIZEOF(wname));
	char_to_wchar(password, wpassword, SIZEOF(wpassword));

    len += swprintf(&wxml[len], size-len, L"<?xml version=\"1.0\"?>\n");
    len += swprintf(&wxml[len], size-len, L"<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">\n");
	len += swprintf(&wxml[len], size-len, L"    <name>%ws</name>\n", wname);
	len += swprintf(&wxml[len], size-len, L"    <SSIDConfig>\n");
	len += swprintf(&wxml[len], size-len, L"        <SSID>\n");
	len += swprintf(&wxml[len], size-len, L"            <name>%ws</name>\n", wname);
	len += swprintf(&wxml[len], size-len, L"        </SSID>\n");
	len += swprintf(&wxml[len], size-len, L"        <nonBroadcast>false</nonBroadcast>\n");
	len += swprintf(&wxml[len], size-len, L"    </SSIDConfig>\n");
	len += swprintf(&wxml[len], size-len, L"    <connectionType>ESS</connectionType>\n");
	len += swprintf(&wxml[len], size-len, L"    <connectionMode>auto</connectionMode>\n");
	len += swprintf(&wxml[len], size-len, L"    <MSM>\n");
	len += swprintf(&wxml[len], size-len, L"        <security>\n");
	len += swprintf(&wxml[len], size-len, L"            <authEncryption>\n");
	len += swprintf(&wxml[len], size-len, L"	            <authentication>WPAPSK</authentication>\n");
	len += swprintf(&wxml[len], size-len, L"		        <encryption>AES</encryption>\n");
    len += swprintf(&wxml[len], size-len, L"	            <useOneX>false</useOneX>\n");
	len += swprintf(&wxml[len], size-len, L"            </authEncryption>\n");
	len += swprintf(&wxml[len], size-len, L"            <sharedKey>\n");
	len += swprintf(&wxml[len], size-len, L"	            <keyType>passPhrase</keyType>\n");
	len += swprintf(&wxml[len], size-len, L"	            <protected>false</protected>\n");
	len += swprintf(&wxml[len], size-len, L"	            <keyMaterial>%ws</keyMaterial>\n", wpassword);
	len += swprintf(&wxml[len], size-len, L"            </sharedKey>\n");
	len += swprintf(&wxml[len], size-len, L"        </security>\n");
	len += swprintf(&wxml[len], size-len, L"    </MSM>\n");
    len += swprintf(&wxml[len], size-len, L"</WLANProfile>\n");

	/* --- TEST --------------------------------------
		LPWSTR  wxml=NULL;
		DWORD dwFlags = WLAN_PROFILE_GET_PLAINTEXT_KEY;
		DWORD dwGrantedAccess;
		char_to_wchar(name, wname, SIZEOF(wname));
		dwReason=0;
		dwResult=WlanGetProfile(hClient, &guid, wname, NULL, &wxml, &dwFlags, &dwGrantedAccess);
		wprintf(L"  Profile XML string:\n");
		wprintf(L"%ws\n\n", wxml);

		wprintf(L"  dwFlags:\t    0x%x", dwFlags);
		//                    if (dwFlags & WLAN_PROFILE_GET_PLAINTEXT_KEY)
		//                        wprintf(L"   Get Plain Text Key");
			if (dwFlags & WLAN_PROFILE_GROUP_POLICY)
				wprintf(L"  Group Policy");
			if (dwFlags & WLAN_PROFILE_USER)
				wprintf(L"  Per User Profile");
			wprintf(L"\n");    

		wprintf(L"  dwGrantedAccess:  0x%x", dwGrantedAccess);
		if (dwGrantedAccess & WLAN_READ_ACCESS)
			wprintf(L"  Read access");
		if (dwGrantedAccess & WLAN_EXECUTE_ACCESS)
			wprintf(L"  Execute access");
		if (dwGrantedAccess & WLAN_WRITE_ACCESS)
			wprintf(L"  Write access");
		wprintf(L"\n");    
	*/
	dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
        return dwResult;
    }

	dwResult=WlanSetProfile(hClient, &guid, 0, wxml, NULL, TRUE, NULL, &dwReason);
	if (dwResult)
	{
		WlanReasonCodeToString(dwReason, SIZEOF(wname), wname, NULL);
		wprintf(L"ERROR: WlanSetProfile >>%ws<<\n", wname);
	}
	
	if (hClient)
	{
		WlanCloseHandle(hClient, NULL);
		hClient=NULL;
	}
	return dwResult;
}

//---rx_wlan_del_profile -----------------------------------
int rx_wlan_del_profile	 (GUID guid, char *name)
{
    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2;
    DWORD dwCurVersion = 0;
    DWORD dwResult = 0;
    
    WCHAR wname[128];

    dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
        goto END;
    }

	char_to_wchar(name, wname, SIZEOF(wname));
	WlanDeleteProfile(hClient, &guid, wname, NULL);

END:
	if (hClient)
	{
		WlanCloseHandle(hClient, NULL);
		hClient=NULL;
	}
	return dwResult;
}

//--- rx_wlan_connect ------------------------------------
int rx_wlan_connect(GUID guid,	char *profile)
{
    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2;      //    
    DWORD dwCurVersion = 0;
	DWORD  dwResult;
	WLAN_CONNECTION_PARAMETERS par;
	WCHAR wprofile[64];

    dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
        goto END;
        // You can use FormatMessage here to find out why the function failed
    }

	char_to_wchar(profile, wprofile, SIZEOF(wprofile));

	memset(&par, 0, sizeof(WLAN_CONNECTION_PARAMETERS));
	par.wlanConnectionMode = wlan_connection_mode_profile;
	par.strProfile = wprofile;
	par.dwFlags = 0;
	par.pDot11Ssid = NULL;
	par.pDesiredBssidList = 0;
	par.dot11BssType = dot11_BSS_type_infrastructure; // pNetworkList->Network[i].dot11BssType;

	dwResult = WlanConnect(hClient, &guid, &par, NULL);

END:
	if (hClient)
	{
		WlanCloseHandle(hClient, NULL);
		hClient=NULL;
	}
	return dwResult;
}

//--- rx_wlan_disconnect --------------------
int rx_wlan_disconnect	 (GUID guid)
{
    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2;      //    
    DWORD dwCurVersion = 0;
	DWORD  dwResult;

    dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
        return dwResult;
        // You can use FormatMessage here to find out why the function failed
    }

	dwResult = WlanDisconnect(hClient, &guid, NULL);
	if (hClient)
	{
		WlanCloseHandle(hClient, NULL);
		hClient=NULL;
	}
	return dwResult;
}

#endif
