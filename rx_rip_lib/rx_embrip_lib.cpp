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
#include "rx_common.h"
#include "rx_file.h"
#include "rx_setup_file.h"
#include "rx_xml.h"
#include "rx_embrip_def.h"
#include "rx_embrip_lib.h"

//--- embrip_open_xml ------------------------------------------
HANDLE embrip_open_xml(const char *path)
{
	HANDLE file = setup_create();
	setup_load(file, path);

	return file;
}

//--- embrip_close_xml ---------------------------------
int embrip_close_xml(HANDLE file)
{
	setup_destroy(file);
	return REPLY_OK;
}

//--- embrip_screening_cfg ---------------------------------
int embrip_screening_cfg(HANDLE file, int write, SScreeningCfg *cfg)
{
	EN_setup_Action action = (EN_setup_Action)write;

//	if (setup_chapter_c(file, "PrintEnvironment", -1, action)!=REPLY_OK) 
//		return REPLY_ERROR;

	if (setup_chapter (file, "Screening", -1, action)!=REPLY_OK) 
		return REPLY_ERROR;

	setup_enum8 (file, "Mode", action, &cfg->mode, (enumToStr)EScreeningModeStr);
	setup_str (file, "ProfileName", action, cfg->profileName, sizeof(cfg->profileName), "Profile");

	if (setup_chapter(file, "..", -1, action)!=REPLY_OK) 
		return REPLY_ERROR;

//	if (setup_chapter(file, "..", -1, action)!=REPLY_OK) 
//		return REPLY_ERROR;

	return REPLY_OK;
}

//--- embrip_ripping_cfg -----------------------------------
int embrip_ripping_cfg  (HANDLE file, int write, SRippingCfg *cfg)
{
	EN_setup_Action action = (EN_setup_Action)write;

//	if (setup_chapter(file, "PrintEnvironment", -1, action)!=REPLY_OK) 
//		return REPLY_ERROR;

	if (setup_chapter(file, "Ripping", -1, action)!=REPLY_OK) 
		return REPLY_ERROR;

	setup_str  (file, "OutputProfileName", action, cfg->outputProfileName, sizeof(cfg->outputProfileName), "Profile");
	setup_enum8(file, "RenderIntent",      action, &cfg->renderIntent,	(enumToStr)ERenderIntentStr);
	setup_enum8(file, "BlackPreserve",     action, &cfg->blackPreserve, (enumToStr)EBlackPreserveStr);
	setup_enum8(file, "BlackPtComp",	   action, &cfg->blackPtComp,	(enumToStr)EBoolStr);
	setup_enum8(file, "SimuOverprint",	   action, &cfg->simuOverprint, (enumToStr)EBoolStr);

	if (setup_chapter(file, "..", -1, action)!=REPLY_OK) 
		return REPLY_ERROR;

	if (setup_chapter(file, "..", -1, action)!=REPLY_OK) 
		return REPLY_ERROR;

	return REPLY_OK;
}

//--- embrip_create_xml -----------------------
HANDLE embrip_create_xml(void)
{
	return setup_create();
}

//--- embrip_save_xml -------------------------------
int embrip_save_xml(HANDLE doc, const char *path)
{
	setup_save(doc, path);
	setup_destroy(doc);
	return REPLY_OK;
}

//--- embrip_save_ripping_cfg ----------------------------
int embrip_save_ripping_cfg  (void *doc, int save)
{
	setup_destroy(doc);
	return REPLY_OK;
}

//--- embrip_ScreeningModeStr ---------------------------------
int	  embrip_ScreeningModeStr(EScreeningMode mode, char *str)
{
	strcpy(str, EScreeningModeStr(mode));
	if (mode<end) return REPLY_OK;
	else          return REPLY_ERROR;
}



