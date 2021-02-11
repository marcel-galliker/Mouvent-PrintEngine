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

#include "stdafx.h"
#include "rx_common.h"
#include "tinyxml.h"
#include "rx_xml.h"
#include "rx_counter.h"

static SCounterDef	_CtrDef;
int					CTR_Counter=0;

static int _convert_to_int    (SCounterDef *pCtrDef, INT32 val, char *str);
static int _convert_to_counter(SCounterDef *pCtrDef, INT32 val, char *str);

//--- ctr_load_def ---------------------------------------------------------------
int  ctr_load_def(void *doc, SCounterDef *pCtrDef)
{	
	if (pCtrDef->size!=sizeof(SCounterDef)) return 1;
	memset(pCtrDef, 0, sizeof(SCounterDef));
	pCtrDef->size = sizeof(SCounterDef);

	TiXmlDocument		*pDoc = (TiXmlDocument*)doc;

	CRX_XmlElement	*ctr=NULL;
	
	if (doc) ctr = (CRX_XmlElement*)pDoc->FirstChild("Counter");
	if (ctr==NULL) ctr = new CRX_XmlElement("Counter");
	ctr->Attribute("Elements-N",	pCtrDef->elements_n, SIZEOF(pCtrDef->elements_n),	"0123456789");
	ctr->Attribute("Elements-A",	pCtrDef->elements_a, SIZEOF(pCtrDef->elements_a),	"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	ctr->Attribute("Elements-X",	pCtrDef->elements_x, SIZEOF(pCtrDef->elements_x),	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	ctr->Attribute("Elements",		pCtrDef->elements,	 SIZEOF(pCtrDef->elements),		"NNNNNN");
	ctr->Attribute("Start",		    pCtrDef->start,		 SIZEOF(pCtrDef->start),		"000001");
	ctr->Attribute("End",		    pCtrDef->end,		 SIZEOF(pCtrDef->end),			"999999");
	ctr->Attribute("LeadingZeros",	&pCtrDef->leadingZeros,								1);		
	ctr->Attribute("Increment",		(int*)&pCtrDef->increment,							INC_perLabel);		
	return 0;
}

//--- ctr_save_def ---------------------------------------------------------------
int  ctr_save_def(void *doc, SCounterDef *pCtrDef)
{
	if (pCtrDef->size!=sizeof(SCounterDef)) return 1;

	CRX_XmlDocument		*pDoc = (CRX_XmlDocument*)doc;
	
	//--- write file definition --------------------------
	CRX_XmlElement *ctr = pDoc->InsertBefore("Files", "Counter");
	ctr->SetAttribute("Elements-N",		pCtrDef->elements_n);
	ctr->SetAttribute("Elements-A",		pCtrDef->elements_a);
	ctr->SetAttribute("Elements-X",		pCtrDef->elements_x);
	ctr->SetAttribute("Elements",		pCtrDef->elements);
	ctr->SetAttribute("Start",		    pCtrDef->start);
	ctr->SetAttribute("End",		    pCtrDef->end);
	ctr->SetAttribute("LeadingZeros",	pCtrDef->leadingZeros);		
	ctr->SetAttribute("Increment",		pCtrDef->increment);		

	// all pointers are deleted in the DOC destructor!
	return 0;	
}

//--- _convert_to_int -------------------------------------
static INT32 _convert_to_int(SCounterDef *pCtrDef, char *str)
{
	INT32	val=0, v;
	int		i, weight;
	char	*elements;

	weight=1;
	for (i=(int)strlen(pCtrDef->elements); i; )
	{
		i--;
		switch(pCtrDef->elements[i])
		{
		case 'N':	elements = pCtrDef->elements_n; break;
		case 'A':	elements = pCtrDef->elements_a; break;
		case 'X':	elements = pCtrDef->elements_x; break;
		}
		for (v=0; elements[v] && str[i]!=elements[v]; v++)
		{};
		if (elements[v]==0) return -1;
		val += v*weight;
		weight *= (int)strlen(elements);
	}
	return val;
}

//--- _convert_to_counter ------------------------------------
static int _convert_to_counter(SCounterDef *pCtrDef, INT32 val, char *str)
{
	int		i, len;
	char	*elements;

	str[strlen(pCtrDef->elements)]=0;
	for (i=(int)strlen(pCtrDef->elements); i; )
	{
		i--;
		switch(pCtrDef->elements[i])
		{
		case 'N':	elements = pCtrDef->elements_n; break;
		case 'A':	elements = pCtrDef->elements_a; break;
		case 'X':	elements = pCtrDef->elements_x; break;
		default:	return 1;
		}
		len = (int)strlen(elements);
		if (!len)  return 1;
		str[i] = elements[val%len];
		val /= len;
	}
	if (!pCtrDef->leadingZeros)
	{
		for (i=0; str[i]=='0'; i++)
		{};
		memmove(str, &str[i], strlen(&str[i])+1);
	}
	return 0;
}

//--- ctr_def_is_equal --------------------------------------------
int  ctr_def_is_equal  (SCounterDef *pCtrDef1, SCounterDef *pCtrDef2)
{
	int res=memcmp(pCtrDef1, pCtrDef2, sizeof(SCounterDef));
	return (res==0);
}

//--- ctr_set_def --------------------------------------------
void  ctr_set_def(SCounterDef *pCtrDef)
{
	memcpy(&_CtrDef, pCtrDef, sizeof(_CtrDef));
}

//--- ctr_increment_mode --------------------------------------------
EIncrement ctr_increment_mode()
{
	return _CtrDef.increment;
}

//--- ctr_set_counter ------------------------------------------------
void  ctr_set_counter(INT32 recNo)
{
	CTR_Counter = recNo;
}

//--- ctr_inc_counter -----------------------------------------------
void  ctr_inc_counter		(void)
{
	CTR_Counter++;
}

//--- ctr_get_counter --------------------------------------------
int  ctr_get_counter(INT32 no, char *str)
{
	INT32 start = _convert_to_int(&_CtrDef,  _CtrDef.start);
	if (start<0 || _convert_to_counter(&_CtrDef, start+no, str)) sprintf(str, "INVALID");	 
	return 0;
}