
// Win10-InstallDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Win10-Install.h"
#include "Win10-InstallDlg.h"
#include "lib.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//--- defines -------------------------------
#define RX_COMPUTER_NAME	L"RX-Touch-"


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CWin10InstallDlg dialog



CWin10InstallDlg::CWin10InstallDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWin10InstallDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWin10InstallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDE_SERIAL_NO, ce_serial_no);
	DDX_Control(pDX, IDE_COMPUTER_NAME, ce_computer_name);
	DDX_Control(pDX, IDE_AUTO_LOGIN, ce_auto_login);
	DDX_Control(pDX, IDE_STARTUP, ce_startup);
	DDX_Control(pDX, IDE_EXPLORER_SETTINGS, ce_explorer_settings);
	DDX_Control(pDX, IDE_UWFMGR, ce_uwfmgr);
	DDX_Control(pDX, IDE_TCPIP, ce_tcpip);
}

BEGIN_MESSAGE_MAP(CWin10InstallDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CWin10InstallDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CWin10InstallDlg message handlers

BOOL CWin10InstallDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	//------------------------------------------------------------------------------
	{
		wchar_t name[64];
		DWORD  len = sizeof(name);

		GetComputerName(name, &len);
		if (!wcsncmp(name, RX_COMPUTER_NAME, 10))
		{
			ce_serial_no.SetWindowText(&name[10]);
		}
		else ce_serial_no.SetWindowText(L"0001");
	}
	//------------------------------------------------------------------------------

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWin10InstallDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWin10InstallDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWin10InstallDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int SetComputerName(int serialNo)
{
	wchar_t	str[100];
	HKEY	key;
	int		ret;

	if (serialNo<1) 
	{
		AfxMessageBox(L"SerialNo invalid");
		return 1;
	}
	swprintf(str, RX_COMPUTER_NAME L"%04d", serialNo);
	if (RegOpenKey( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ComputerName", &key)==ERROR_SUCCESS)
	{
		ret=RegSetValueEx  (key, L"ComputerName", 0, REG_SZ, (BYTE*)str, (int)wcslen(str)*sizeof(*str));
	}
	if (ret==ERROR_SUCCESS && RegOpenKey( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", &key)==ERROR_SUCCESS)
	{
		ret=RegSetValueEx  (key, L"NV Hostname", 0, REG_SZ, (BYTE*)str, (int)wcslen(str)*sizeof(*str));
	}
	if (ret==ERROR_SUCCESS && RegOpenKey( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\lanmanserver\\parameters", &key)==ERROR_SUCCESS)
	{
		swprintf(str, RX_COMPUTER_NAME L"%04d", serialNo);
		ret=RegSetValueEx  (key, L"srvcomment", 0, REG_SZ, (BYTE*)str, (int)wcslen(str)*sizeof(*str));
	}

	swprintf(str, L"Error %d", ret);
	if (ret==5)  AfxMessageBox(L"Access denied: Check that app RUNs AS ADMINISTRATOR");
	else if (ret!=ERROR_SUCCESS) AfxMessageBox(str);
	return ret;
}

//--- ExplorerSettings -------------------------------------------
int ExplorerSettings(wchar_t *userId)
{
	HKEY	key;
	int		ret;
	DWORD	value;
	wchar_t keyname[MAX_PATH];

	if (*userId)
	{
		wsprintf(keyname, L"%s\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", userId);
		ret=RegOpenKey( HKEY_USERS, keyname, &key);
	}
	else ret=RegOpenKey( HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", &key);
	
	if (ret==ERROR_SUCCESS)
	{
		value = 0;
		ret=RegSetValueEx  (key, L"HideFileExt", 0, REG_DWORD, (BYTE*)&value, sizeof(value));
		value = 0;
		ret=RegSetValueEx  (key, L"AutoCheckSelect", 0, REG_DWORD, (BYTE*)&value, sizeof(value));	// use check boxes to select items
	}
	return ERROR_SUCCESS;
}

//--- Startup -------------------------------------------
int Startup(wchar_t *userId)
{
	HKEY	key;
	int		ret;
	wchar_t keyname[MAX_PATH];

	if (RegOpenKey( HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\IniFileMapping\\system.ini\\boot", &key)==ERROR_SUCCESS)
	{
		wchar_t*	str=L"USR:Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon";
		ret=RegSetValueEx  (key, L"Shell", 0, REG_SZ, (BYTE*)str, (int)wcslen(str)*2);	
	}
	if (*userId)
	{
		wsprintf(keyname, L"%s\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", userId);
		ret=RegOpenKey( HKEY_USERS, keyname, &key);
	}
	else ret=RegOpenKey( HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon", &key);

	if (ret==ERROR_SUCCESS)
	{
		wchar_t*	str=L"d:\\radex\\bin\\win10-startup.exe";
		ret=RegSetValueEx  (key, L"Shell", 0, REG_SZ, (BYTE*)str, (int)wcslen(str)*2);	
	}
	return ERROR_SUCCESS;
}

//--- DisableDefender -------------------------------------------
int DisableDefender(void)
{
	HKEY	key;
	int		ret;
	DWORD	value;

	ret=RegOpenKey( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Policies\\Microsoft\\Windows Defender", &key);
	if (ret==ERROR_SUCCESS)
	{
		value = 1;
		ret=RegSetValueEx  (key, L"DisableAntiSpyware", 0, REG_DWORD, (BYTE*)&value, sizeof(value));	// use check boxes to select items
	}
	return ERROR_SUCCESS;
}

//--- DisableWriteFilter ---------------------------------
int DisableWriteFilter()
{
	int ret;
	char str[2*MAX_PATH];
	char str1[MAX_PATH];

	char *filename = "C:\\Users\\Public\\Documents\\uwfmgrout.txt";
	FILE *file;
	char *src, *dst;
	
	ret = rx_process_execute("uwfmgr filter disable", filename, 20000);

	if (ret==ERROR_SUCCESS)
	{
		ret = ERROR_SUCCESS_RESTART_REQUIRED;
		file=fopen(filename, "r");
		if (file!=NULL)
		{
			memset(str, 0, sizeof(str));
			while (fgets(str, sizeof(str)/2, file))
			{
				src=str;
				if (!*src && src[1]) src++;
				for (dst=str1; *src; src+=2, dst++)
					*dst=*src;
				*dst=0;
				if (strstr(str1, "Access is denied"))
				{
					ret = ERROR_ACCESS_DENIED;
					break;					
				}
				if (strstr(str1, "no action taken"))
				{
					ret = ERROR_SUCCESS;
					break;
				}
				memset(str, 0, sizeof(str));
			}
			fclose(file);
			remove(filename);
		}
	}

	return ret;
}

//--- TcpIpSettings ----------------------------------
int TcpIpSettings(wchar_t *interfaceName, wchar_t *ipaddr )
{
	HKEY	key;
	HKEY	key1;
	HKEY	key2, key3;
	int		ret, i;
	DWORD	len, val;
	wchar_t 	adapterGUID[128];
	wchar_t 	name1[128];
	wchar_t 	name2[128];
	wchar_t 	name3[128];
	wchar_t		keyname2[128];
	wchar_t		keyname3[128];
	wchar_t		str[128];

	ret=RegOpenKey( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces", &key);
	if (ret==ERROR_SUCCESS)
	{
		for (i=0; RegEnumKey(key, i, adapterGUID, sizeof(adapterGUID))==ERROR_SUCCESS; i++)
		{
			// check adapter name
			ret=RegOpenKey( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Network", &key1);
			{
				for (int n=0; RegEnumKey(key1, n, name1, sizeof(name1))==ERROR_SUCCESS; n++)
				{
					if (name1[0]=='{')
					{
						swprintf(keyname2, L"SYSTEM\\CurrentControlSet\\Control\\Network\\%s", name1);
						ret=RegOpenKey( HKEY_LOCAL_MACHINE, keyname2, &key2);
						{
							for (int m=0; RegEnumKey(key2, m, name2, sizeof(name2))==ERROR_SUCCESS; m++)
							{
								if (!_wcsicmp(name2, adapterGUID))
								{
									swprintf(keyname3, L"%s\\Connection", name2);
									len = sizeof(name3);
									ret = RegGetValue(key2, keyname3, L"Name", RRF_RT_REG_SZ, NULL, name3, &len);
									if (!_wcsicmp(name3, interfaceName))
									{
										swprintf(keyname3, L"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\%s", adapterGUID);									
										ret=RegOpenKey( HKEY_LOCAL_MACHINE, keyname3, &key3);
										// EnableDHCP = 0
										val = 0;
										ret = RegSetValueEx(key3, L"EnableDHCP", 0, REG_DWORD, (BYTE*)&val, sizeof(val));
										// RegistrationEnabled = 1
										val = 1;
										ret = RegSetValueEx(key3, L"RegistrationEnabled", 0, REG_DWORD, (BYTE*)&val, sizeof(val));
										// RegisterAdapterName = 0
										val = 0;
										ret = RegSetValueEx(key3, L"RegisterAdapterName", 0, REG_DWORD, (BYTE*)&val, sizeof(val));
										// DisableHccpOnConnect=1
										val = 1;
										ret = RegSetValueEx(key3, L"DisableHccpOnConnect", 0, REG_DWORD, (BYTE*)&val, sizeof(val));
										// IPAddress = ipaddr  (REG_MULTI_SZ)
										ret=RegSetValueEx(key3, L"IPAddress", 0, REG_SZ, (BYTE*)ipaddr, (DWORD)wcslen(ipaddr)*2);
										// SubnetMask = 255.255.255.0 (REG_MULTI_SZ)
										wsprintf(str, L"255.255.255.0");
										ret=RegSetValueEx(key3, L"SubnetMask", 0, REG_SZ, (BYTE*)str, (DWORD)wcslen(str)*2);
										return ERROR_SUCCESS;
									}
								}
							}							
						}								
					}
				}
			}

		}
	}
	return ret;
}

//--- GetUserID -------------------------------------------------
int GetUserID(wchar_t *username, wchar_t *id)
{
	HKEY	key;
	int		ret, i;
	DWORD   len;
	wchar_t userID[128];
	wchar_t keyname[MAX_PATH];
	wchar_t path[MAX_PATH];
	wchar_t start[MAX_PATH];

	// command: "wmic useraccount where name='username' get sid"

	wsprintf(start, L"C:\\Users\\%s", username);

	ret=RegOpenKey( HKEY_USERS, NULL, &key);
	if (ret==ERROR_SUCCESS)
	{
		for (i=0; RegEnumKey(key, i, userID, sizeof(userID))==ERROR_SUCCESS; i++)
		{
			swprintf(keyname, L"%s\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", userID);
			len = MAX_PATH; 
			ret = RegGetValue(HKEY_USERS, keyname, L"AppData", RRF_RT_REG_SZ, NULL, path, &len);
			if (ret==ERROR_SUCCESS)
			{
				printf("user: >>%s<< path=>>%s<<\n", userID, path);
				if (_wcsnicmp(path, start, wcslen(start))==0)
				{
					wcscpy(id, userID);
					return ERROR_SUCCESS;
				}
			}
		}
	}
	*id = 0;
	return ERROR_FILE_NOT_FOUND;
}

//--- OnBnClickedOk ---------------------------------------------
void CWin10InstallDlg::OnBnClickedOk()
{
	int ret;
	wchar_t str[32];
	wchar_t userId[MAX_PATH];

	ret = DisableWriteFilter();

//	wsprintf(str, L"Disable Write Filter: %d", ret);
//	AfxMessageBox(str);
	
	if (ret==ERROR_ACCESS_DENIED)
	{
		AfxMessageBox(L"Access is denied: Make sure the appplication is RUN AS ADMINISTRATOR!");
	}
	else if (ret==ERROR_SUCCESS_RESTART_REQUIRED)
	{
		AfxMessageBox(L"Disable Write Filter: Restart the system now.");
		ret = rx_process_execute("shutdown /r /t 0", NULL, 1000);			
	}
	else
	{
		ret = GetUserID(L"rx-user", userId);
		if (ret==ERROR_SUCCESS)
		{
			ce_serial_no.GetWindowText(str, sizeof(str));
			ret = SetComputerName(_wtoi(str));
			swprintf(str, L"%d", ret);
			ce_computer_name.SetWindowText(str); 
		}
		else 
		/*
		AfxMessageBox(L"Could not find rx-user.");

		if (TRUE || ret==ERROR_SUCCESS)
		*/ 
		{
			ret = rx_process_execute("SetAutoLogin rx-user radex", NULL, 1000);
			swprintf(str, L"%d", ret);
			ce_auto_login.SetWindowText(str); 
		}

		if (ret==ERROR_SUCCESS) 
		{
			ret = ExplorerSettings(userId);
			swprintf(str, L"%d", ret);
			ce_explorer_settings.SetWindowText(str); 
		}

		if (ret==ERROR_SUCCESS) 
		{
			ret = Startup(userId);
			swprintf(str, L"%d", ret);
			ce_startup.SetWindowText(str); 
		}

		if (ret==ERROR_SUCCESS) 
		{
			ret = TcpIpSettings(L"Ethernet", L"192.168.200.2");
			swprintf(str, L"%d", ret);
			ce_tcpip.SetWindowText(str); 
		}

		if (ret==ERROR_SUCCESS) 
		{
			ret = rx_process_execute("net localgroup \"Remote Desktop Users\" rx-user /ADD", NULL, 20000);
		}

		if (ret==ERROR_SUCCESS) 
		{
			ret = DisableDefender();
		}

		if (ret==ERROR_SUCCESS) 
		{
			ret = rx_process_execute("uwfmgr filter enable", NULL, 20000);
			swprintf(str, L"%d", ret);
			ce_uwfmgr.SetWindowText(str); 
		}

		if (ret==ERROR_SUCCESS) 
		{
			AfxMessageBox(L"All Settings done: Restart the system now.");
			ret = rx_process_execute("shutdown /r /t 0", NULL, 1000);
		}
		else 
		{
			AfxMessageBox(L"There was an ERROR!");
		}
	}

	CDialogEx::OnOK();
}
