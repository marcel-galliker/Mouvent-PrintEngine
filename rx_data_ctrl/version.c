// ****************************************************************************
//
//	DIGITAL PRINTING - version.c
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include <stdio.h>
#include "version.h"

static char *revisionStr = "$Revision: 100 $";
static char *dateStr	 = "$Date: 2014-05-02 11:08:11 +0200 (Fr, 02 Mai 2014) $";

static int ver_major = 1;
static int ver_minor = 0;
static int ver_revision = 100;
static int ver_build = 99;

const char version[32]="1.0.100.99";