
// Win10-StartupDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CWin10StartupDlg dialog
class CWin10StartupDlg : public CDialogEx
{
// Construction
public:
	CWin10StartupDlg(CWnd* pParent = NULL);	// standard constructor

	void SetHostName(wchar_t *hostname);

// Dialog Data
	enum { IDD = IDD_WIN10STARTUP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	BOOL OnEraseBkgnd(CDC* pDC);
	CBitmap m_background;


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void StartGui();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CProgressCtrl m_progress;
	int			  m_started;
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
private:
	CStatic m_loading;
	CButton IDB_Local;
public:
	afx_msg void OnBnClicked_StartLocal();
};
