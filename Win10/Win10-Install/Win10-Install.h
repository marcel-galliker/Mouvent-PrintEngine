
// Win10-Install.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWin10InstallApp:
// See Win10-Install.cpp for the implementation of this class
//

class CWin10InstallApp : public CWinApp
{
public:
	CWin10InstallApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CWin10InstallApp theApp;