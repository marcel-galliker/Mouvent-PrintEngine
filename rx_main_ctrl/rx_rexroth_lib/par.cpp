// RX-Rexroth.cpp : Defines the exported functions for the DLL application.
//

#ifdef _WIN32
	#include "stdafx.h"
#endif

#include "rx_rexroth.h"
#include "rx_xmlDoc.h"
#include "rx_xml.h"
#include "mlpi.h"
#include "rx_common.h"
#include "rx_trace.h"
#include <mlpiParameterLib.h>
#include <mlpiSystemLib.h>

#define PARAM_CHAPTER	"Parameters"

//--- -------------------------------------------------

static const char *DeviceName[17] = 
{
	"CPU",			//	0
	"Unwinder",		//	1
	"Infeed",		//	2
	"Outfeed",		//	3
	"Rewinder",		//	4
	"device_5",			//	5
	"device_6",			//	6
	"device_7",			//	7
	"PressRoller_2010001",			//	8
	"PressRoller_2010002",			//	9
	"device_10",			//	10
	"device_11",			//	11
	"device_12",			//	12
	"device_13",			//	13
	"device_14",			//	14
	"device_15",			//	15
	"MasterAxes"	//	16
};
 
typedef struct
{
	Eplc_Device device;
	const char *sidn;
	int save;
} SParameter;


static SParameter _ParList[] = 
{
//	 device			parameter	save
	{CPU,			"C-0-0000"},
	{CPU,			"C-0-0001"},
	{CPU,			"C-0-0012"},
	{CPU,			"C-0-0023"},
	{CPU,			"C-0-0050"},
	{CPU,			"C-0-0051"}, 
	{CPU,			"C-0-0070"},
	{CPU,			"C-0-0080"},
	{CPU,			"C-0-0081",	1},

	{Unwinder,		"A-0-0001"},
	{Unwinder,		"A-0-0002"},

	{Infeed,		"A-0-0001"},
	{Infeed,		"A-0-0002"},

	{Outfeed,		"A-0-0001"},
	{Outfeed,		"A-0-0002"},

	{Rewinder,		"A-0-0001"},
	{Rewinder,		"A-0-0002"},

	{MasterAxes,	"A-0-0001"},
	{MasterAxes,	"A-0-0002"},
};

//--- Prototypes --------------------------------------------------------------
static void _str_to_sidn(const char *str, ULLONG *sidn, int *device);

//--- _str_to_sidn ---------------------
static void _str_to_sidn(const char *str, ULLONG *sidn, int *device)
{
	
	const char *deviceStr;
	char type;
	int set;
	int block;

	*device=0;
	if (deviceStr = strchr(str, '.'))
	{
		sscanf(deviceStr+1, "%c-%d-%d", &type, &set, &block);
		while(!str_start(str, DeviceName[*device]))
			(*device)++;
	}
	else sscanf(str, "%c-%d-%d", &type, &set, &block);

	switch (type)
	{
	case 'A': *sidn=MLPI_SIDN_A(block); break;
	case 'C': *sidn=MLPI_SIDN_C(block); break;
	case 'K': *sidn=MLPI_SIDN_K(block); break;
	case 'M': *sidn=MLPI_SIDN_M(block); break;
	case 'N': *sidn=MLPI_SIDN_N(block); break;
	case 'O': *sidn=MLPI_SIDN_O(block); break;
	case 'S': *sidn=MLPI_SIDN_S(block); break;
	case 'P': *sidn=MLPI_SIDN_P(block); break;
	default:  *sidn=0; break;
	}
}


//--- Test ---------------------------------------
static void Test(MLPIHANDLE connection)
{
	MLPIRESULT result;
	ULONG numElementsRet;
	ULLONG parameter;
	WCHAR16 unit[16] = L"";
	WCHAR16 min[512] = L"";
	WCHAR16 max[512] = L"";
	WCHAR16 value[512] = L"";
	WCHAR16 def[512] = L"";
	ULONG attribute = 0;
	WCHAR16 name[512] = L"";
	wchar_t trace[2014];
	wchar_t wstr[64];

	int i;

	int device;

	for (i=0; i<SIZEOF(_ParList); i++)
	{
		// Read the name of parameter 'C-0-0012'.
		_str_to_sidn(_ParList[i].sidn, &parameter, &device);
		result = mlpiParameterReadName(connection, device, parameter, name, _countof(name));
		if (!MLPI_FAILED(result))
		{
			result = mlpiParameterReadAttribute		(connection, device, parameter, &attribute);
			result = mlpiParameterReadUnit			(connection, device, parameter, unit, _countof(unit));
			result = mlpiParameterReadMinimumString	(connection, device, parameter, min, _countof(min), &numElementsRet);
			result = mlpiParameterReadMaximumString	(connection, device, parameter, max, _countof(max), &numElementsRet);
			result = mlpiParameterReadDataString	(connection, device, parameter, value, _countof(value), &numElementsRet);
			result = mlpiParameterReadDefaultString	(connection, device, parameter, def, _countof(def), &numElementsRet);
			#ifndef linux
				swprintf(trace, L"Param >>%s:%s<< value= >>%s<<, min=>>%s<<, max=>>%s<<, def=>>%s<<\n", char_to_wchar(_ParList[i].sidn, wstr, SIZEOF(wstr)), name, value, min, max, def);
				OutputDebugStringW(trace);
			#endif
		}
	}
}


//--- par_get_name -----------------------------
int par_get_name(int no, char *pdevice, char *sidn, char *name, char *value, int size)
{
	MLPIRESULT result;

	if (no<SIZEOF(_ParList))
	{
		int len;
		ULONG numElementsRet;
		wchar_t wstr[256]; 
		ULLONG sidn;
		int device;

		strcpy(pdevice, DeviceName[_ParList[no].device]);
		_str_to_sidn(_ParList[no].sidn, &sidn, &device);
		result = mlpiParameterReadName(_Connection, _ParList[no].device, sidn, wstr, SIZEOF(wstr));
		len=sprintf(name, "%s.%s: ", DeviceName[_ParList[no].device], _ParList[no].sidn);
		wchar_to_char(wstr, &name[len], size-len);
		result = mlpiParameterReadDataString(_Connection, _ParList[no].device, sidn, wstr, SIZEOF(wstr), &numElementsRet);
		wchar_to_char(wstr, value, size);
		return (rex_check_result(result))==0;
	}
	return FALSE;
}

//--- par_get_value -------------------------------------------------
int par_get_value (const char *psidn, char *value, int size)
{
	ULONG numElementsRet;
	wchar_t wstr[256];
	ULLONG sidn;
	int device;
	
	_str_to_sidn(psidn, &sidn, &device);
	MLPIRESULT result = mlpiParameterReadDataString(_Connection, device, sidn, wstr, SIZEOF(wstr), &numElementsRet);
	wchar_to_char(wstr, value, size);
	rex_check_result(result);
	return 0;
}

//--- par_set_value --------------------------------------------------
int par_set_value (const char *psidn, char *value)
{
	ULLONG sidn;
	wchar_t wvalue[128];
	int device;
	
	_str_to_sidn(psidn, &sidn, &device);
	MLPIRESULT result = mlpiParameterWriteDataString(_Connection, device, sidn, char_to_wchar(value, wvalue, SIZEOF(wvalue)));
	rex_check_result(result);
	return 0;
}

//--- par_save ------------------------------------------------------------
int par_save(rx_doc doc)
{
	int i;
	char	name[128];
	char	value[128];
	
	TiXmlDocument  *pDoc = (TiXmlDocument*)doc;
	CRX_XmlElement *params = new CRX_XmlElement(PARAM_CHAPTER); 

	for (i=0; i<SIZEOF(_ParList); i++)
	{
		if (_ParList[i].save)
		{
			sprintf(name, "%s.%s", DeviceName[_ParList[i].device], _ParList[i].sidn);
			TiXmlElement *par = new TiXmlElement(name);
			par_get_value(name, value, sizeof(value));
			par->SetAttribute("Value", value);
			params->LinkEndChild(par);
		}
	}
	pDoc->LinkEndChild(params);
	return 0;
}

//--- par_load ------------------------------------------------------------
int par_load(rx_doc doc)
{	
	char	value[128];
	CRX_XmlElement *params;
	CRX_XmlElement *param;

	return 0;
	
	TiXmlDocument	*pDoc = (TiXmlDocument*)doc;
	if (pDoc==NULL) return 1;
	params = (CRX_XmlElement*)pDoc->FirstChild(PARAM_CHAPTER);
	if (params==NULL) params = new CRX_XmlElement(PARAM_CHAPTER);
	
	MlpiSystemMode mode = MLPI_SYSTEMMODE_SWITCHING;
	
	for( param = (CRX_XmlElement *)params->FirstChild(); param; param = (CRX_XmlElement *)param->NextSibling() )
	{
		// some parameters can only be changed with the PLC in "P2" mode
		if (mode==MLPI_SYSTEMMODE_SWITCHING)
		{
			mlpiSystemGetCurrentMode(_Connection, &mode);
			sys_set_mode(MLPI_SYSTEMMODE_P2);			
		}
		param->Attribute("Value", value, SIZEOF(value), "");
		TrPrintf(TRUE, "par_load >>%s<<=>>%s<<", param->Value(), value);
		par_set_value(param->Value(), value);
	}
	if (mode != MLPI_SYSTEMMODE_SWITCHING) sys_set_mode(mode);

	return 0;
}
