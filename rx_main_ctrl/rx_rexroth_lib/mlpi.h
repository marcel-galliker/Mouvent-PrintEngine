// ****************************************************************************
//
//	
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

// #include "RX_Rexroth.h"

#include <mlpiParameterLib.h>

//--- defines ----------------------------------------
#define MLPI_TIMEOUT	1000

//--- external variables -------------------------------
extern MLPIHANDLE	_Connection;
extern char			_IpAddr[64];
extern wchar_t		_WIpAddr[64];
extern wchar_t		_WApplication[64];
extern char			_UsrMsg[128];
// extern char			_ErrMsg[128];


//--- functions -------------------------------------------------

void rex_message(const char *format, ...);
int	 rex_check_result(MLPIRESULT result);
