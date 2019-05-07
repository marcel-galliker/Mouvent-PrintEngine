
// Win10-Startup.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CWin10StartupApp:
// See Win10-Startup.cpp for the implementation of this class
//

class CWin10StartupApp : public CWinApp
{
public:
	CWin10StartupApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CWin10StartupApp theApp;