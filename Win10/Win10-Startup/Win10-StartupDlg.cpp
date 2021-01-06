// ****************************************************************************
//
//	Win10-StartupDlg.cpp 
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "stdafx.h"
#include <io.h>
#include <Shlobj.h>
#include "Windows.h"
#include "Win10-Startup.h"
#include "Win10-StartupDlg.h"
#include "afxdialogex.h"

//--- DEFINES ----------------------------------------------

#define STARTUP_DELAY_LOCAL		2
#define STARTUP_DELAY_REMOTE	150
#define STARTUP_TIME_LOCAL		10
#define STARTUP_TIME_REMOTE		20


#define DEFUALT_HOST_NAME	"192.168.200.1"
#define USER				"radex"
#define PASSWORD			"radex"

#define LOCAL_GUI_DIR		"c:\\gui\\"
#define GUI_EXE				"RX_DigiPrint.exe"
#define REMOTE				false

int		_startup_time;
int		_delay_time;
static char _HostName[MAX_PATH]="";

//--- Statics ----------------------------------------------
bool CWin10StartupDlg::m_BackgroundThreadRunning = false;
HANDLE CWin10StartupDlg::m_BackgroundThreadHdl = NULL;
DWORD CWin10StartupDlg::m_BackgroundThreadId = 0;

//----------------------------------------------------------

CWin10StartupDlg::CWin10StartupDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWin10StartupDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_started = false;
	m_connected = false;
}

void CWin10StartupDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_LOADING, m_loading);
	DDX_Control(pDX, IDC_BUTTON1, IDB_Local);
}

BEGIN_MESSAGE_MAP(CWin10StartupDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON1, &CWin10StartupDlg::OnBnClicked_StartLocal)
END_MESSAGE_MAP()

//--- rx_process_start -----------------------------------------------------
int rx_process_start	(const char *process, const char *params)
{
	STARTUPINFOA		startuopInfo;
	PROCESS_INFORMATION	processInfo;
	char				*workingDir=NULL;

	memset(&startuopInfo, 0, sizeof(startuopInfo));
	startuopInfo.cb = sizeof(startuopInfo);
	

	if (params) return CreateProcessA((char*)process, (char*)params, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS | DETACHED_PROCESS, NULL, workingDir, &startuopInfo, &processInfo);
	else	    return CreateProcessA(NULL, (char*)process,          NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS | DETACHED_PROCESS, NULL, workingDir, &startuopInfo, &processInfo);
}

//--- rx_process_execute --------------------------------------------------------------------
int rx_process_execute(const char *process, const char *outPath, int timeout)
{
	SECURITY_ATTRIBUTES		sa;
	STARTUPINFOA			startuopInfo;
	PROCESS_INFORMATION		processInfo;

	int		ret;
	ULONG	code;
	HANDLE	hfile=NULL;
	 
	memset(&startuopInfo, 0, sizeof(startuopInfo));
	startuopInfo.cb			= sizeof(startuopInfo);

	if (outPath && *outPath)
	{
		memset(&sa, 0, sizeof(sa));
		sa.nLength			= sizeof(sa);
		sa.bInheritHandle	= TRUE;

		hfile = CreateFileA(outPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, CREATE_ALWAYS, FILE_FLAG_SEQUENTIAL_SCAN, 0);

		startuopInfo.dwFlags	= STARTF_USESTDHANDLES;
		startuopInfo.hStdOutput = hfile;
		startuopInfo.hStdError  = hfile;
	}

	ret = CreateProcessA(NULL, (char*)process, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &startuopInfo, &processInfo);
	if (!ret) 
	{
		if (hfile) CloseHandle(hfile);
		return GetLastError();
	}
	do
	{
		Sleep(10);
		if (timeout)
		{
			timeout-=10;
			if (timeout<=0) 
			{
				TerminateProcess(processInfo.hProcess, WAIT_TIMEOUT);
			}
		}
		GetExitCodeProcess(processInfo.hProcess, &code);
	} while (code==STILL_ACTIVE);
	if (hfile) CloseHandle(hfile);
	return code;
}

//--- rx_popen ---------------------------------------------------------
HANDLE rx_popen(const char *cmd)
{
	SECURITY_ATTRIBUTES		sa;
	STARTUPINFOA			startuopInfo;
	PROCESS_INFORMATION		processInfo;
	HANDLE					stream_rd = NULL;
	HANDLE					stream_wr = NULL;
	int		ret;
	 
	//--- create the pipe -------------------
	sa.nLength = sizeof(SECURITY_ATTRIBUTES); 
	sa.bInheritHandle = TRUE; 
	sa.lpSecurityDescriptor = NULL; 

	if ( !CreatePipe(&stream_rd, &stream_wr, &sa, 0) ) 
	{
		printf("StdoutRd CreatePipe"); 
		return NULL;
	}

	if ( !SetHandleInformation(stream_rd, HANDLE_FLAG_INHERIT, 0) )
	{
		printf("Stdout SetHandleInformation"); 
		return NULL;
	}

	//--- create the process ----------------------------------
	memset(&startuopInfo, 0, sizeof(startuopInfo));
	startuopInfo.cb			= sizeof(startuopInfo);
	startuopInfo.dwFlags	= STARTF_USESTDHANDLES;
	startuopInfo.hStdOutput = stream_wr;
	startuopInfo.hStdError  = stream_wr;

	ret = CreateProcessA(NULL, (char*)cmd, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &startuopInfo, &processInfo);

	CloseHandle(stream_wr);
	CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
	return stream_rd;
}

//--- rx_process_running_cnt --------------------------------------
int rx_process_running_cnt(const char *process, const char *arg)
{
	int start, count=0;
	DWORD len;
	HANDLE file;
	char str[100];

	for (start = (int)strlen(process); start > 0; start--)
	{
		if (process[start] == '\\' || process[start] == '/')
		{
			start++;
			break;
		}
	}

	file = rx_popen("tasklist.exe");
	if (file)
	{
		while (ReadFile( file, str, sizeof(str), &len, NULL))
		{
			if (strstr(str, &process[start])) count++;
		}
		CloseHandle(file);
	}
	return count;
}

//--- _checkPower ------------------------------
void CWin10StartupDlg::CheckPower()
{
	static int _acPower = FALSE;

	SYSTEM_POWER_STATUS status;
	GetSystemPowerStatus( &status );
	if (_acPower && !status.ACLineStatus) 
		system("shutdown /s /t 0 /f");
	_acPower = status.ACLineStatus;
}

//--- SetHostName -----------------------------------
void CWin10StartupDlg::SetHostName(wchar_t *hostname)
{
	if (*hostname)  wcstombs(_HostName, hostname, sizeof(_HostName));
	else		    strcpy(_HostName, DEFUALT_HOST_NAME);
}

//--- OnInitDialog ------------------------------------------------
BOOL CWin10StartupDlg::OnInitDialog()
{
	int ret=m_background.LoadBitmap(IDB_WALLPAPER);

	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	HDC hdc = CreateCompatibleDC(NULL);
	int sx  = GetDeviceCaps(hdc, HORZRES);
	int sy  = GetDeviceCaps(hdc, VERTRES);
	int width=500;
	RECT rect;
	int w, h;
	DeleteDC(hdc);

	_delay_time		= STARTUP_DELAY_REMOTE;
	_startup_time	= STARTUP_TIME_REMOTE;
	m_progress.SetRange(0, _delay_time);
	m_loading.SetWindowText(L"Connecting");

	m_progress.SetWindowPos(NULL, 50, sy-50, width, 20, 0);
	m_loading.SetWindowPos (NULL, 50, sy-50-30, width, 30, 0);
	IDB_Local.GetWindowRect(&rect);
	w = rect.right-rect.left;
	h = rect.bottom-rect.top;
	w *=2;
	h *=2;
	IDB_Local.SetWindowPos  (NULL, 50, sy-100-h, w, h, 0);
	SetTimer (1, 1000, NULL);
	ShowWindow(SW_MAXIMIZE);
	IDB_Local.Invalidate();

	//--- start the background thread -----------
	{
		m_BackgroundThreadRunning = true;
		m_BackgroundThreadHdl = CreateThread(NULL,			// pThreadAttributes 
				(SIZE_T)0,									// dwStackSize
				(LPTHREAD_START_ROUTINE) BackgroundThread,	// lpStartAddress 
				this,										// lpParameter 
				0,											// dwCreationFlags 
				&m_BackgroundThreadId						// lpThreadId
				);
	}
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

//--- _BackgroundThread ----------------------------------------
void CWin10StartupDlg::BackgroundThread(void *par)
{
	// https://docs.microsoft.com/de-de/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output?redirectedfrom=MSDN

	CWin10StartupDlg	*pdlg = (CWin10StartupDlg*)par;

	HANDLE	stream_rd;
	char	cmd[100];
	char	buf[256];
	DWORD	len;
	int		ret;

	ret = rx_process_running_cnt(GUI_EXE, NULL);
	if (ret>0) pdlg->m_started=true;

	sprintf(cmd, "ping -t %s", _HostName);
	stream_rd = rx_popen(cmd);

	while (m_BackgroundThreadRunning)
	{
		memset(buf, 0, sizeof(buf));
		if (ReadFile( stream_rd, buf, sizeof(buf), &len, NULL))
		{
			CheckPower();
			OutputDebugStringA(buf);
			if (strstr(buf, "Request timed out"))
			{
				if (pdlg->m_connected)
				{
					pdlg->m_connected = false;
					sprintf(cmd, "net use \"\\\\%s\\gui\" /DELETE", _HostName);
					ret = rx_process_execute(cmd, NULL, 1000);
				}
			}
			else if (strstr(buf, "Reply from"))
			{
				if (!pdlg->m_connected)
				{
					sprintf(cmd, "net.exe use \\\\%s\\gui /USER:%s %s", _HostName, USER, PASSWORD);
					ret = rx_process_execute(cmd, NULL, 1000);
					pdlg->m_connected = (ret==0);
				}
			}
			if (pdlg->m_connected && !pdlg->m_started)
			{
				//--- copy directory ---
				char *dst_dir=LOCAL_GUI_DIR;
				sprintf(cmd, "md %s", dst_dir);
				rx_process_execute(cmd, NULL, 0);

				sprintf(cmd, "xcopy \"\\\\%s\\gui\" %s /I /R /Y", _HostName, dst_dir);
				rx_process_execute(cmd, NULL, 0);
				pdlg->StartGui();
			}
		}
	};
}

//--- OnEraseBkgnd ----------------------------------------------------
BOOL CWin10StartupDlg::OnEraseBkgnd(CDC* pDC) 
{
	CDialog::OnEraseBkgnd(pDC);	
	if(!m_background.m_hObject)
		return true;
	
	CRect rect;
	GetClientRect(&rect);
	
	COLORREF bgcolor=RGB(0xff, 0xff, 0xff);
	SetBackgroundColor(bgcolor);
	
	//--- set dialog background ---------------------------------
	CBrush myBrush(bgcolor);
	CBrush *pOld = pDC->SelectObject(&myBrush);
    BOOL bRes  = pDC->PatBlt(0, 0, rect.Width(), rect.Height(), PATCOPY);
    pDC->SelectObject(pOld); 

	//--- set the bitmap ----------------------------------------
	CDC dc;
	dc.CreateCompatibleDC(pDC);
	CBitmap* pOldBitmap = dc.SelectObject(&m_background);

	BITMAP bmap;
	m_background.GetBitmap(&bmap);
	double stretch = 1.0*rect.Height()/bmap.bmHeight;
	pDC->StretchBlt(rect.Width()-(int)(bmap.bmWidth*stretch), 0, (int)(bmap.bmWidth*stretch), (int)(bmap.bmHeight*stretch), &dc,0, 0, bmap.bmWidth, bmap.bmHeight, SRCCOPY);
	dc.SelectObject(pOldBitmap);

	//--- refresh the button ---------------------
	IDB_Local.Invalidate();
	m_progress.Invalidate();
	return true;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWin10StartupDlg::OnPaint()
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
		m_progress.Invalidate();
	}
}

//--- OnClose -------------------------------------------------------
void CWin10StartupDlg::OnClose()
{
    //--- start explorer if not running already ---
    system("explorer.exe");
	CDialogEx::OnClose();
}

//--- OnQueryDragIcon -------------------------------------------------------
HCURSOR CWin10StartupDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//--- StartGui -------------------------------------
void CWin10StartupDlg::StartGui()
{
	int pos = m_progress.GetPos();
	rx_process_start(LOCAL_GUI_DIR GUI_EXE, NULL);
	m_loading.SetWindowText(L"Starting");
	m_loading.Invalidate();
	m_progress.SetBarColor(CLR_DEFAULT);
	m_progress.SetRange(0, STARTUP_TIME_REMOTE);
	m_started = TRUE;
}

//--- OnTimer ---------------------------------------
void CWin10StartupDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CDialogEx::OnTimer(nIDEvent);

	if (nIDEvent == 1)
	{
		int pos = m_progress.GetPos();
		CheckPower();
		if (REMOTE)
		{
			char cmd[256];
			sprintf(cmd, "net.exe use \\\\%s\\gui /USER:%s %s", _HostName, USER, PASSWORD);
//			ret= rx_process_execute(cmd, "c:/radex/trace.txt", 1000);	
			int ret= rx_process_execute(cmd, NULL, 1000);
			if (ret==0 && !m_started) 
			{
				//--- start binary on main PC --------------------------
				//	sprintf(cmd, "\\\\%s\\gui\\RX_DigiPrint.exe", _HostName);
				//	ret = rx_process_start(cmd, NULL);

				//--- copy directory ---
				char *dst_dir=LOCAL_GUI_DIR;
				sprintf(cmd, "md %s", dst_dir);
				rx_process_execute(cmd, NULL, 0);

				sprintf(cmd, "xcopy \"\\\\%s\\gui\" %s /I /R /Y", _HostName, dst_dir);
				rx_process_execute(cmd, NULL, 0);
				StartGui();
			}
			else if (pos>_delay_time)
			{					
				m_progress.SetBarColor(RGB(255,0,0));
			}
			else
			{
				sprintf(cmd, "cmd /C net use \"\\\\%s\\gui\" /DELETE", _HostName);
				ret= rx_process_execute(cmd, NULL, 1000);
//				system(cmd);
			}
		}
		else
		{
			if (pos==_delay_time) rx_process_start("d:\\radex\\bin\\gui\\RX_DigiPrint.exe", NULL);
	//		if (pos==_delay_time+_startup_time)  EndDialog(0);
		}

		m_progress.SetPos(pos+1);
	}
}

//--- OnBnClicked_StartLocal -----------------------------
void CWin10StartupDlg::OnBnClicked_StartLocal()
{
	StartGui();
}