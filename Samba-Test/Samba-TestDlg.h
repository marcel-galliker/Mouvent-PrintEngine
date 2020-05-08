
// Samba-TestDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CSambaTestDlg dialog
class CSambaTestDlg : public CDialogEx
{
// Construction
public:
	CSambaTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SAMBATEST_DIALOG };

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

public:
	afx_msg void OnBnClickedConvert1BPX();
	afx_msg void OnBnClickedConvertBmp();
private:
	CComboBox	cc_dotGereration;
	CComboBox	cc_bitsPerPixel;
	CEdit		ce_pixel0;
	CButton		cb_version2;
};
