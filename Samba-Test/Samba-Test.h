
// Samba-Test.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CSambaTestApp:
// See Samba-Test.cpp for the implementation of this class
//

class CSambaTestApp : public CWinApp
{
public:
	CSambaTestApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CSambaTestApp theApp;