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

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//--- DEFINES ----------------------------------------------

#define REMOTE			TRUE

#define STARTUP_DELAY_LOCAL		2
#define STARTUP_DELAY_REMOTE	150
#define STARTUP_TIME_LOCAL		10
#define STARTUP_TIME_REMOTE		20


#define DEFUALT_HOST_NAME	"192.168.200.1"
#define USER				"radex"
#define PASSWORD			"radex"

#define LOCAL_GUI_DIR		"c:\\gui\\"

int		_startup_time;
int		_delay_time;
static char _HostName[MAX_PATH]="";

//----------------------------------------------------------


CWin10StartupDlg::CWin10StartupDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWin10StartupDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_started = FALSE;
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

//--- TrPrintf -------------
void TrPrintf ( const char * format, ... )
{
	char buffer[256];
	int len;
	va_list args;
	va_start (args, format);
	len=vsprintf (buffer, format, args);
	va_end (args);
	sprintf(&buffer[len], "\n");
	OutputDebugStringA(buffer);
}

// CWin10StartupDlg message handlers

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
	
	TrPrintf("rx_process_execute");

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

	TrPrintf("CreateProcess");
	ret = CreateProcessA(NULL, (char*)process, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL, NULL, &startuopInfo, &processInfo);
	TrPrintf("ret=%d", ret);
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

	if (REMOTE)
	{
		_delay_time		= STARTUP_DELAY_REMOTE;
		_startup_time	= STARTUP_TIME_REMOTE;
		m_progress.SetRange(0, _delay_time);
		m_loading.SetWindowText(L"Connecting");
	}
	else
	{
		_delay_time		= STARTUP_DELAY_LOCAL;
		_startup_time	= STARTUP_TIME_LOCAL;
		m_progress.SetRange(0, _delay_time+_startup_time);
	}

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
	return TRUE;  // return TRUE  unless you set the focus to a control
}

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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWin10StartupDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//--- _checkPower ------------------------------
static void _checkPower(void)
{
	static int _acPower = FALSE;

	SYSTEM_POWER_STATUS status;
	GetSystemPowerStatus( &status );
	if (_acPower && !status.ACLineStatus) 
		system("shutdown /s /t 0 /f");
	_acPower = status.ACLineStatus;
}

//--- StartGui -------------------------------------
void CWin10StartupDlg::StartGui()
{
	int pos = m_progress.GetPos();
	rx_process_start(LOCAL_GUI_DIR "RX_DigiPrint.exe", NULL);
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

	int ret;
	static int time;
	char cmd[MAX_PATH];

	if (nIDEvent == 1)
	{
		int pos = m_progress.GetPos();
		_checkPower();
		if (REMOTE)
		{
			sprintf(cmd, "net.exe use \\\\%s\\gui /USER:%s %s", _HostName, USER, PASSWORD);
			ret= rx_process_execute(cmd, NULL, 1000);
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
//				ret= rx_process_execute(cmd, NULL, 1000);
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
