
// ****************************************************************************
//
//	
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "stdafx.h"
#include "afxdialogex.h"
#include "Samba-Test.h"
#include "Samba-TestDlg.h"
#include "DotShift.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define VERSION 2

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


// CSambaTestDlg dialog



CSambaTestDlg::CSambaTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSambaTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSambaTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DOT_GENERATION, cc_dotGereration);
	DDX_Control(pDX, IDC_BITS_PER_PIXEL, cc_bitsPerPixel);
	DDX_Control(pDX, IDE_PX_0,			 ce_pixel0);
	DDX_Control(pDX, IDC_VERSION_2,		 cb_version2);
}

BEGIN_MESSAGE_MAP(CSambaTestDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDB_CONVERT_1BPX, &CSambaTestDlg::OnBnClickedConvert1BPX)
	ON_BN_CLICKED(IDB_CONVERT_BMP, &CSambaTestDlg::OnBnClickedConvertBmp)
END_MESSAGE_MAP()


// CSambaTestDlg message handlers

BOOL CSambaTestDlg::OnInitDialog()
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
	cc_dotGereration.AddString(L"1");	
	cc_dotGereration.AddString(L"01");	
	cc_dotGereration.AddString(L"10");	
	cc_dotGereration.AddString(L"11");	
	cc_dotGereration.AddString(L"001");	
	cc_dotGereration.AddString(L"010");	
	cc_dotGereration.AddString(L"011"); 
	cc_dotGereration.AddString(L"100");	
	cc_dotGereration.AddString(L"101"); 
	cc_dotGereration.AddString(L"110"); 
	cc_dotGereration.AddString(L"111"); 
	cc_dotGereration.AddString(L"1000");
	cc_dotGereration.AddString(L"10000"); 
	cc_dotGereration.AddString(L"100000"); 
	cc_dotGereration.AddString(L"1000000"); 
	cc_dotGereration.AddString(L"10000000"); 

	cc_dotGereration.SetCurSel(4);

	cc_bitsPerPixel.AddString(L"1");
	cc_bitsPerPixel.AddString(L"01");
	cc_bitsPerPixel.AddString(L"10");
	cc_bitsPerPixel.AddString(L"11");
	cc_bitsPerPixel.SetCurSel(0);

	ce_pixel0.SetWindowTextW(L"0");
	cb_version2.SetCheck(TRUE);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSambaTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSambaTestDlg::OnPaint()
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
HCURSOR CSambaTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//--- OnBnClickedConvert1BPX -----------------------------
void CSambaTestDlg::OnBnClickedConvert1BPX()
{
	CFileDialog dlg(TRUE);

	if (dlg.DoModal()==IDOK)
	{
		wchar_t dst[256];
		wchar_t info[256];
		wchar_t dotGeneration[10];
		wchar_t bitsPerPixel[10];
		int		pixel0;
		int		version;

		ce_pixel0.GetWindowText(dotGeneration, sizeof(dotGeneration));
		pixel0 = _wtoi(dotGeneration);
		cc_dotGereration.GetWindowText(dotGeneration, sizeof(dotGeneration));
		cc_bitsPerPixel. GetWindowText(bitsPerPixel, sizeof(bitsPerPixel));
		version = cb_version2.GetCheck() ? 2:1;

		wcscpy_s(dst, dlg.GetPathName());
		for (int i=wcslen(dst)-1; i>=0 && dst[i]!='.'; i--)
			dst[i]=0;
		wcscpy(info, dst);
//		wcscpy(&dst[wcslen(dst)], L"udp");
//		wcscpy(&info[wcslen(info)], L"txt");
		udp_data(dlg.GetPathName(), dst, info, pixel0, bitsPerPixel);

		wcscpy_s(dst, dlg.GetPathName());
		for (int i=wcslen(dst)-1; i>=0 && dst[i]!='.'; i--)
			dst[i]=0;
//		wcscpy(&dst[wcslen(dst)], L"srd");

		//--- generate all 4 files ---
		{	
			shift_bmp(dlg.GetPathName(), dst, dotGeneration, bitsPerPixel, pixel0, false, false, version);
			shift_bmp(dlg.GetPathName(), dst, dotGeneration, bitsPerPixel, pixel0, true,  false, version);
			shift_bmp(dlg.GetPathName(), dst, dotGeneration, bitsPerPixel, pixel0, false, true,  version);
			shift_bmp(dlg.GetPathName(), dst, dotGeneration, bitsPerPixel, pixel0, true,  true,	 version);
		}
		AfxMessageBox(L"Files created", MB_OK);
	}
}

//--- OnBnClickedConvertBmp --------------------------------------
void CSambaTestDlg::OnBnClickedConvertBmp()
{
	CFileDialog dlg(TRUE);

	if (dlg.DoModal()==IDOK)
	{
		wchar_t dst[256];
		int version = cb_version2.GetCheck()? 2:1;
		wcscpy_s(dst, dlg.GetPathName());
		for (int i=wcslen(dst)-1; i>=0 && dst[i]!='.'; i--)
			dst[i]=0;
		wcscpy(&dst[wcslen(dst)], L"test.bmp");
		if (cb_version2.GetCheck())  shift_bmp_reverse(dlg.GetPathName(), dst, 2);
		else                         shift_bmp_reverse(dlg.GetPathName(), dst, 1);
	}
}
