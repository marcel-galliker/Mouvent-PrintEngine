
// Win10-InstallDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CWin10InstallDlg dialog
class CWin10InstallDlg : public CDialogEx
{
// Construction
public:
	CWin10InstallDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WIN10INSTALL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CEdit ce_serial_no;
public:
	afx_msg void OnBnClickedOk();
private:
	CEdit ce_computer_name;
	CEdit ce_auto_login;
	CEdit ce_explorer_settings;
public:
	CEdit ce_uwfmgr;
private:
	CEdit ce_tcpip;
	CEdit ce_startup;
};
