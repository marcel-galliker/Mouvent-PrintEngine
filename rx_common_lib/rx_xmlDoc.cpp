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

#ifdef _WIN32
//	#include "stdafx.h"
#endif

#include "tinyxml.h"
#include "rx_xmlDoc.h"

//--- rx_doc_load ------------------------------------------
rx_doc  rx_doc_load(char *path)
{
	TiXmlDocument *pDoc = new TiXmlDocument; 
	
	//--- read file definition ----------------
	if (!pDoc->LoadFile(path)) 
	{
		const char *err=pDoc->ErrorDesc();
		return NULL;
	}
	return pDoc;
}

//--- rx_doc_new -----------------------------------------
rx_doc  rx_doc_new ()
{
	TiXmlDocument *pDoc = new TiXmlDocument; 

	TiXmlDeclaration	*decl = new TiXmlDeclaration("1.0", "", "");
 	pDoc->LinkEndChild(decl);
	return pDoc;
}

//--- rx_doc_save --------------------------------------
int  rx_doc_save(char *path, rx_doc doc)
{
	TiXmlDocument *pDoc = (TiXmlDocument*) doc;

	return pDoc->SaveFile(path);
}

//--- rx_doc_free ------------------------------------
int  rx_doc_free(rx_doc doc)
{
	delete (TiXmlDocument*)doc;
	return 0;
}
