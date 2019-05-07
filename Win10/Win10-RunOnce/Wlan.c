#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <wlanapi.h>
#include <objbase.h>
#include <wtypes.h>

#include <stdio.h>
#include <stdlib.h>

// Need to link with Wlanapi.lib and Ole32.lib
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "ole32.lib")

int wlan_enum_interfaces(void)
{

    // Declare and initialize variables.

    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2;   //    
    DWORD dwCurVersion = 0;
    DWORD dwResult = 0;
    int iRet = 0;
    
    WCHAR GuidString[40] = {0};
     
    int i;

    /* variables used for WlanEnumInterfaces  */
    
    PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
    PWLAN_INTERFACE_INFO pIfInfo = NULL;

    
    dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient); 
    if (dwResult != ERROR_SUCCESS)  {
        wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
        // FormatMessage can be used to find out why the function failed
        return 1;
    }
    
    dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList); 
    if (dwResult != ERROR_SUCCESS)  {
        wprintf(L"WlanEnumInterfaces failed with error: %u\n", dwResult);
        // FormatMessage can be used to find out why the function failed
        return 1;
    }
    else {
        wprintf(L"Num Entries: %lu\n", pIfList->dwNumberOfItems);
        wprintf(L"Current Index: %lu\n", pIfList->dwIndex);
        for (i = 0; i < (int) pIfList->dwNumberOfItems; i++) {
            pIfInfo = (WLAN_INTERFACE_INFO *) &pIfList->InterfaceInfo[i];
            wprintf(L"  Interface Index[%d]:\t %lu\n", i, i);
            iRet = StringFromGUID2(&pIfInfo->InterfaceGuid, (LPOLESTR) &GuidString, 39); 
            // For c rather than C++ source code, the above line needs to be
            // iRet = StringFromGUID2(&pIfInfo->InterfaceGuid, (LPOLESTR) &GuidString, 39); 
            if (iRet == 0)
                wprintf(L"StringFromGUID2 failed\n");
            else {
                wprintf(L"  InterfaceGUID[%d]: %ws\n",i, GuidString);
            }    
            wprintf(L"  Interface Description[%d]: %ws", i, 
                pIfInfo->strInterfaceDescription);
            wprintf(L"\n");
            wprintf(L"  Interface State[%d]:\t ", i);
            switch (pIfInfo->isState) {
            case wlan_interface_state_not_ready:
                wprintf(L"Not ready\n");
                break;
            case wlan_interface_state_connected:
                wprintf(L"Connected\n");
                break;
            case wlan_interface_state_ad_hoc_network_formed:
                wprintf(L"First node in a ad hoc network\n");
                break;
            case wlan_interface_state_disconnecting:
                wprintf(L"Disconnecting\n");
                break;
            case wlan_interface_state_disconnected:
                wprintf(L"Not connected\n");
                break;
            case wlan_interface_state_associating:
                wprintf(L"Attempting to associate with a network\n");
                break;
            case wlan_interface_state_discovering:
                wprintf(L"Auto configuration is discovering settings for the network\n");
                break;
            case wlan_interface_state_authenticating:
                wprintf(L"In process of authenticating\n");
                break;
            default:
                wprintf(L"Unknown state %ld\n", pIfInfo->isState);
                break;
            }
            wprintf(L"\n");
        }
    }

    if (pIfList != NULL) {
        WlanFreeMemory(pIfList);
        pIfList = NULL;
    }
    return 0;
}

//--- wlan_networks ---------------------------------
int wlan_networks()
{

    // Declare and initialize variables.

    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2;      //    
    DWORD dwCurVersion = 0;
    DWORD dwResult = 0;
    DWORD dwRetVal = 0;
    int iRet = 0;
    
    WCHAR GuidString[39] = {0};

    unsigned int i, j, k;

    /* variables used for WlanEnumInterfaces  */

    PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
    PWLAN_INTERFACE_INFO pIfInfo = NULL;

    PWLAN_AVAILABLE_NETWORK_LIST pBssList = NULL;
    PWLAN_AVAILABLE_NETWORK pBssEntry = NULL;
    
    int iRSSI = 0;

    dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
        return 1;
        // You can use FormatMessage here to find out why the function failed
    }

    dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
    if (dwResult != ERROR_SUCCESS) {
        wprintf(L"WlanEnumInterfaces failed with error: %u\n", dwResult);
        return 1;
        // You can use FormatMessage here to find out why the function failed
    } else {
        wprintf(L"Num Entries: %lu\n", pIfList->dwNumberOfItems);
        wprintf(L"Current Index: %lu\n", pIfList->dwIndex);
        for (i = 0; i < (int) pIfList->dwNumberOfItems; i++) {
            pIfInfo = (WLAN_INTERFACE_INFO *) &pIfList->InterfaceInfo[i];
            wprintf(L"  Interface Index[%u]:\t %lu\n", i, i);
            iRet = StringFromGUID2(&pIfInfo->InterfaceGuid, (LPOLESTR) &GuidString, 
                sizeof(GuidString)/sizeof(*GuidString)); 
            // For c rather than C++ source code, the above line needs to be
            // iRet = StringFromGUID2(&pIfInfo->InterfaceGuid, (LPOLESTR) &GuidString, 
            //     sizeof(GuidString)/sizeof(*GuidString)); 
            if (iRet == 0)
                wprintf(L"StringFromGUID2 failed\n");
            else {
                wprintf(L"  InterfaceGUID[%d]: %ws\n",i, GuidString);
            }    
            wprintf(L"  Interface Description[%d]: %ws", i, 
                pIfInfo->strInterfaceDescription);
            wprintf(L"\n");
            wprintf(L"  Interface State[%d]:\t ", i);
            switch (pIfInfo->isState) {
            case wlan_interface_state_not_ready:
                wprintf(L"Not ready\n");
                break;
            case wlan_interface_state_connected:
                wprintf(L"Connected\n");
                break;
            case wlan_interface_state_ad_hoc_network_formed:
                wprintf(L"First node in a ad hoc network\n");
                break;
            case wlan_interface_state_disconnecting:
                wprintf(L"Disconnecting\n");
                break;
            case wlan_interface_state_disconnected:
                wprintf(L"Not connected\n");
                break;
            case wlan_interface_state_associating:
                wprintf(L"Attempting to associate with a network\n");
                break;
            case wlan_interface_state_discovering:
                wprintf(L"Auto configuration is discovering settings for the network\n");
                break;
            case wlan_interface_state_authenticating:
                wprintf(L"In process of authenticating\n");
                break;
            default:
                wprintf(L"Unknown state %ld\n", pIfInfo->isState);
                break;
            }
            wprintf(L"\n");

            dwResult = WlanGetAvailableNetworkList(hClient,
                                             &pIfInfo->InterfaceGuid,
                                             0, 
                                             NULL, 
                                             &pBssList);

            if (dwResult != ERROR_SUCCESS) {
                wprintf(L"WlanGetAvailableNetworkList failed with error: %u\n",
                        dwResult);
                dwRetVal = 1;
                // You can use FormatMessage to find out why the function failed
            } else {
                wprintf(L"WLAN_AVAILABLE_NETWORK_LIST for this interface\n");

                wprintf(L"  Num Entries: %lu\n\n", pBssList->dwNumberOfItems);

                for (j = 0; j < pBssList->dwNumberOfItems; j++) {
                    pBssEntry =
                        (WLAN_AVAILABLE_NETWORK *) & pBssList->Network[j];

                    wprintf(L"  Profile Name[%u]:  %ws\n", j, pBssEntry->strProfileName);
                    
                    wprintf(L"  SSID[%u]:\t\t ", j);
                    if (pBssEntry->dot11Ssid.uSSIDLength == 0)
                        wprintf(L"\n");
                    else {   
                        for (k = 0; k < pBssEntry->dot11Ssid.uSSIDLength; k++) {
                            wprintf(L"%c", (int) pBssEntry->dot11Ssid.ucSSID[k]);
                        }
                        wprintf(L"\n");
                    }
                        
                    wprintf(L"  BSS Network type[%u]:\t ", j);
                    switch (pBssEntry->dot11BssType) {
                    case dot11_BSS_type_infrastructure   :
                        wprintf(L"Infrastructure (%u)\n", pBssEntry->dot11BssType);
                        break;
                    case dot11_BSS_type_independent:
                        wprintf(L"Infrastructure (%u)\n", pBssEntry->dot11BssType);
                        break;
                    default:
                        wprintf(L"Other (%lu)\n", pBssEntry->dot11BssType);
                        break;
                    }
                                
                    wprintf(L"  Number of BSSIDs[%u]:\t %u\n", j, pBssEntry->uNumberOfBssids);

                    wprintf(L"  Connectable[%u]:\t ", j);
                    if (pBssEntry->bNetworkConnectable)
                        wprintf(L"Yes\n");
                    else {
                        wprintf(L"No\n");
                        wprintf(L"  Not connectable WLAN_REASON_CODE value[%u]:\t %u\n", j, 
                            pBssEntry->wlanNotConnectableReason);
                    }        

                    wprintf(L"  Number of PHY types supported[%u]:\t %u\n", j, pBssEntry->uNumberOfPhyTypes);

                    if (pBssEntry->wlanSignalQuality == 0)
                        iRSSI = -100;
                    else if (pBssEntry->wlanSignalQuality == 100)   
                        iRSSI = -50;
                    else
                        iRSSI = -100 + (pBssEntry->wlanSignalQuality/2);    
                        
                    wprintf(L"  Signal Quality[%u]:\t %u (RSSI: %i dBm)\n", j, 
                        pBssEntry->wlanSignalQuality, iRSSI);

                    wprintf(L"  Security Enabled[%u]:\t ", j);
                    if (pBssEntry->bSecurityEnabled)
                        wprintf(L"Yes\n");
                    else
                        wprintf(L"No\n");
                        
                    wprintf(L"  Default AuthAlgorithm[%u]: ", j);
                    switch (pBssEntry->dot11DefaultAuthAlgorithm) {
                    case DOT11_AUTH_ALGO_80211_OPEN:
                        wprintf(L"802.11 Open (%u)\n", pBssEntry->dot11DefaultAuthAlgorithm);
                        break;
                    case DOT11_AUTH_ALGO_80211_SHARED_KEY:
                        wprintf(L"802.11 Shared (%u)\n", pBssEntry->dot11DefaultAuthAlgorithm);
                        break;
                    case DOT11_AUTH_ALGO_WPA:
                        wprintf(L"WPA (%u)\n", pBssEntry->dot11DefaultAuthAlgorithm);
                        break;
                    case DOT11_AUTH_ALGO_WPA_PSK:
                        wprintf(L"WPA-PSK (%u)\n", pBssEntry->dot11DefaultAuthAlgorithm);
                        break;
                    case DOT11_AUTH_ALGO_WPA_NONE:
                        wprintf(L"WPA-None (%u)\n", pBssEntry->dot11DefaultAuthAlgorithm);
                        break;
                    case DOT11_AUTH_ALGO_RSNA:
                        wprintf(L"RSNA (%u)\n", pBssEntry->dot11DefaultAuthAlgorithm);
                        break;
                    case DOT11_AUTH_ALGO_RSNA_PSK:
                        wprintf(L"RSNA with PSK(%u)\n", pBssEntry->dot11DefaultAuthAlgorithm);
                        break;
                    default:
                        wprintf(L"Other (%lu)\n", pBssEntry->dot11DefaultAuthAlgorithm);
                        break;
                    }
                        
                    wprintf(L"  Default CipherAlgorithm[%u]: ", j);
                    switch (pBssEntry->dot11DefaultCipherAlgorithm) {
                    case DOT11_CIPHER_ALGO_NONE:
                        wprintf(L"None (0x%x)\n", pBssEntry->dot11DefaultCipherAlgorithm);
                        break;
                    case DOT11_CIPHER_ALGO_WEP40:
                        wprintf(L"WEP-40 (0x%x)\n", pBssEntry->dot11DefaultCipherAlgorithm);
                        break;
                    case DOT11_CIPHER_ALGO_TKIP:
                        wprintf(L"TKIP (0x%x)\n", pBssEntry->dot11DefaultCipherAlgorithm);
                        break;
                    case DOT11_CIPHER_ALGO_CCMP:
                        wprintf(L"CCMP (0x%x)\n", pBssEntry->dot11DefaultCipherAlgorithm);
                        break;
                    case DOT11_CIPHER_ALGO_WEP104:
                        wprintf(L"WEP-104 (0x%x)\n", pBssEntry->dot11DefaultCipherAlgorithm);
                        break;
                    case DOT11_CIPHER_ALGO_WEP:
                        wprintf(L"WEP (0x%x)\n", pBssEntry->dot11DefaultCipherAlgorithm);
                        break;
                    default:
                        wprintf(L"Other (0x%x)\n", pBssEntry->dot11DefaultCipherAlgorithm);
                        break;
                    }

                    wprintf(L"  Flags[%u]:\t 0x%x", j, pBssEntry->dwFlags);
                    if (pBssEntry->dwFlags) {
                        if (pBssEntry->dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED)
                            wprintf(L" - Currently connected");
                        if (pBssEntry->dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED)
                            wprintf(L" - Has profile");
                    }   
                    wprintf(L"\n");
                    
                    wprintf(L"\n");
                }
            }
        }

    }
    if (pBssList != NULL) {
        WlanFreeMemory(pBssList);
        pBssList = NULL;
    }

    if (pIfList != NULL) {
        WlanFreeMemory(pIfList);
        pIfList = NULL;
    }

    return dwRetVal;
}

//--- wlan_profiles ------------------------------
int wlan_profiles()
{
// Declare and initialize variables.

    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2;      //    
    DWORD dwCurVersion = 0;
    DWORD dwResult = 0;
    DWORD dwRetVal = 0;
    int iRet = 0;
    
    WCHAR GuidString[39] = {0};

    unsigned int i, j;

    /* variables used for WlanEnumInterfaces  */

    PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
    PWLAN_INTERFACE_INFO pIfInfo = NULL;

    PWLAN_PROFILE_INFO_LIST pProfileList = NULL;
    PWLAN_PROFILE_INFO pProfile = NULL;

    dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
        return 1;
        // You can use FormatMessage here to find out why the function failed
    }

    dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
    if (dwResult != ERROR_SUCCESS) {
        wprintf(L"WlanEnumInterfaces failed with error: %u\n", dwResult);
        return 1;
        // You can use FormatMessage here to find out why the function failed
    } else {
        wprintf(L"WLAN_INTERFACE_INFO_LIST for this system\n");

        wprintf(L"Num Entries: %lu\n", pIfList->dwNumberOfItems);
        wprintf(L"Current Index: %lu\n", pIfList->dwIndex);
        for (i = 0; i < (int) pIfList->dwNumberOfItems; i++) {
            pIfInfo = (WLAN_INTERFACE_INFO *) &pIfList->InterfaceInfo[i];
            wprintf(L"  Interface Index[%u]:\t %lu\n", i, i);
            iRet = StringFromGUID2(&pIfInfo->InterfaceGuid, (LPOLESTR) &GuidString, 
                sizeof(GuidString)/sizeof(*GuidString)); 
            // For c rather than C++ source code, the above line needs to be
            // iRet = StringFromGUID2(&pIfInfo->InterfaceGuid, (LPOLESTR) &GuidString, 
            //     sizeof(GuidString)/sizeof(*GuidString)); 
            if (iRet == 0)
                wprintf(L"StringFromGUID2 failed\n");
            else {
                wprintf(L"  Interface GUID[%d]: %ws\n",i, GuidString);
            }    
            wprintf(L"  Interface Description[%d]: %ws", i, 
                pIfInfo->strInterfaceDescription);
            wprintf(L"\n");
            wprintf(L"  Interface State[%d]:\t ", i);
            switch (pIfInfo->isState) {
            case wlan_interface_state_not_ready:
                wprintf(L"Not ready\n");
                break;
            case wlan_interface_state_connected:
                wprintf(L"Connected\n");
                break;
            case wlan_interface_state_ad_hoc_network_formed:
                wprintf(L"First node in a ad hoc network\n");
                break;
            case wlan_interface_state_disconnecting:
                wprintf(L"Disconnecting\n");
                break;
            case wlan_interface_state_disconnected:
                wprintf(L"Not connected\n");
                break;
            case wlan_interface_state_associating:
                wprintf(L"Attempting to associate with a network\n");
                break;
            case wlan_interface_state_discovering:
                wprintf(L"Auto configuration is discovering settings for the network\n");
                break;
            case wlan_interface_state_authenticating:
                wprintf(L"In process of authenticating\n");
                break;
            default:
                wprintf(L"Unknown state %ld\n", pIfInfo->isState);
                break;
            }
            wprintf(L"\n");

            dwResult = WlanGetProfileList(hClient,
                                             &pIfInfo->InterfaceGuid,
                                             NULL, 
                                             &pProfileList);

            if (dwResult != ERROR_SUCCESS) {
                wprintf(L"WlanGetProfileList failed with error: %u\n",
                        dwResult);
                dwRetVal = 1;
                // You can use FormatMessage to find out why the function failed
            } else {
                wprintf(L"WLAN_PROFILE_INFO_LIST for this interface\n");

                wprintf(L"  Num Entries: %lu\n\n", pProfileList->dwNumberOfItems);

                for (j = 0; j < pProfileList->dwNumberOfItems; j++) {
                    pProfile =
                        (WLAN_PROFILE_INFO *) & pProfileList->ProfileInfo[j];

                    wprintf(L"  Profile Name[%u]:  %ws\n", j, pProfile->strProfileName);
                    
                    wprintf(L"  Flags[%u]:\t    0x%x", j, pProfile->dwFlags);
                    if (pProfile->dwFlags & WLAN_PROFILE_GROUP_POLICY)
                        wprintf(L"   Group Policy");
                    if (pProfile->dwFlags & WLAN_PROFILE_USER)
                        wprintf(L"   Per User Profile");
                    wprintf(L"\n");    

                    wprintf(L"\n");
                }
            }
        }

    }
    if (pProfileList != NULL) {
        WlanFreeMemory(pProfileList);
        pProfileList = NULL;
    }

    if (pIfList != NULL) {
        WlanFreeMemory(pIfList);
        pIfList = NULL;
    }

    return dwRetVal;
}

int	wlan_profile(LPCWSTR pProfileName)
{
    // Declare and initialize variables.

    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2;      //    
    DWORD dwCurVersion = 0;
    DWORD dwResult = 0;
    DWORD dwRetVal = 0;
    int iRet = 0;
    
    WCHAR GuidString[39] = {0};

    unsigned int i;

    /* variables used for WlanEnumInterfaces  */

    PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
    PWLAN_INTERFACE_INFO pIfInfo = NULL;

 //   LPCWSTR pProfileName = NULL;
    LPWSTR pProfileXml = NULL;
    DWORD dwFlags = 0;
    DWORD dwGrantedAccess = 0;
   
        // Validate the parameters
     
    wprintf("Information for profile: %ws\n\n", pProfileName);
    
    dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
        return 1;
        // You can use FormatMessage here to find out why the function failed
    }

    dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
    if (dwResult != ERROR_SUCCESS) {
        wprintf(L"WlanEnumInterfaces failed with error: %u\n", dwResult);
        return 1;
        // You can use FormatMessage here to find out why the function failed
    } else {
        wprintf(L"WLAN_INTERFACE_INFO_LIST for this system\n");

        wprintf(L"Num Entries: %lu\n", pIfList->dwNumberOfItems);
        wprintf(L"Current Index: %lu\n\n", pIfList->dwIndex);
        for (i = 0; i < (int) pIfList->dwNumberOfItems; i++) {
            pIfInfo = (WLAN_INTERFACE_INFO *) &pIfList->InterfaceInfo[i];
            wprintf(L"  Interface Index[%u]:\t %lu\n", i, i);
            iRet = StringFromGUID2(&pIfInfo->InterfaceGuid, (LPOLESTR) &GuidString, 
                sizeof(GuidString)/sizeof(*GuidString)); 
            // For c rather than C++ source code, the above line needs to be
            // iRet = StringFromGUID2(&pIfInfo->InterfaceGuid, (LPOLESTR) &GuidString, 
            //     sizeof(GuidString)/sizeof(*GuidString)); 
            if (iRet == 0)
                wprintf(L"StringFromGUID2 failed\n");
            else {
                wprintf(L"  InterfaceGUID[%d]: %ws\n",i, GuidString);
            }    
            wprintf(L"  Interface Description[%d]: %ws", i, 
                pIfInfo->strInterfaceDescription);
            wprintf(L"\n");
            wprintf(L"  Interface State[%d]:\t ", i);
            switch (pIfInfo->isState) {
            case wlan_interface_state_not_ready:
                wprintf(L"Not ready\n");
                break;
            case wlan_interface_state_connected:
                wprintf(L"Connected\n");
                break;
            case wlan_interface_state_ad_hoc_network_formed:
                wprintf(L"First node in a ad hoc network\n");
                break;
            case wlan_interface_state_disconnecting:
                wprintf(L"Disconnecting\n");
                break;
            case wlan_interface_state_disconnected:
                wprintf(L"Not connected\n");
                break;
            case wlan_interface_state_associating:
                wprintf(L"Attempting to associate with a network\n");
                break;
            case wlan_interface_state_discovering:
                wprintf(L"Auto configuration is discovering settings for the network\n");
                break;
            case wlan_interface_state_authenticating:
                wprintf(L"In process of authenticating\n");
                break;
            default:
                wprintf(L"Unknown state %ld\n", pIfInfo->isState);
                break;
            }
            wprintf(L"\n\n");
			dwFlags = WLAN_PROFILE_GET_PLAINTEXT_KEY;
            dwResult = WlanGetProfile(hClient,
                                             &pIfInfo->InterfaceGuid,
                                             pProfileName,
                                             NULL, 
                                             &pProfileXml,
                                             &dwFlags,
                                             &dwGrantedAccess);

            if (dwResult != ERROR_SUCCESS) {
                wprintf(L"WlanGetProfile failed with error: %u\n",
                        dwResult);
                // You can use FormatMessage to find out why the function failed
            } else {
                wprintf(L"  Profile Name:  %ws\n", pProfileName);

                wprintf(L"  Profile XML string:\n");
                wprintf(L"%ws\n\n", pProfileXml);

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

                wprintf(L"\n");
            }
        }

    }
    if (pProfileXml != NULL) {
        WlanFreeMemory(pProfileXml);
        pProfileXml = NULL;
    }

    if (pIfList != NULL) {
        WlanFreeMemory(pIfList);
        pIfList = NULL;
    }

    return dwRetVal;
}
