// RX-Rexroth.cpp : Defines the exported functions for the DLL application.
//
#ifdef _WIN32
	#include "stdafx.h"
#endif

#include "rx_common.h"

#include <mlpiApiLib.h>
#include <mlpiLogicLib.h>
#include <mlpiSystemLib.h>
#include <mlpiParameterLib.h>
#include <util/timer.h>
#include <util/wchar16.h>
#include <util/mlpiLogicHelper.h>

#include "rx_error.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "rx_xml.h"
#include "mlpi.h"
#include "par.h"
#include "rx_rexroth.h"


//--- defines ---------------------------------------------
#define MAX_VARIABLES	256

#undef TEST

#define TRIM_SPACE(name) for (int len=strlen(name); len>0 && name[--len]==' ';) name[len]=0;

//--- types -----------------------------------------------
typedef struct
{
	char			name[128];
	MlpiLogicType	type;
	MlpiLogicType	subType;
	char			value[32];
} SVariable;


//--- prototypes ------------------------------------------
static void	_var_name(char *name, wchar_t *wname, int size);

//---  statics -------------------------------------------
static int			_SymbolCnt=0;
static SVariable	_Symbols[MAX_VARIABLES];
static UINT32		_Error=0;

//--- lc_reset_error -------------------------------
void lc_reset_error(void)
{
	_Error=0;
}

//--- lc_get_symbols -------------------------------------------------------
void lc_get_symbols(void)
{
	MLPIHANDLE node=0;
	wchar_t		wsymbols[80];
	wchar_t		wvalue[32];
	char		symbols[80];
	int         i, n, tio, end;
	MLPIRESULT result;
	SVariable *pvar;

	#ifdef TEST
	Error(WARN, 0, "TEST");
	#endif

	memset(_Symbols, 0, sizeof(_Symbols));
	memset(symbols, 0, sizeof(symbols));
	if (!*_WApplication) return;
	for (_SymbolCnt=0, pvar=&_Symbols[_SymbolCnt], end=FALSE; !end; _SymbolCnt++, pvar++)
	{
		if (_SymbolCnt==SIZEOF(_Symbols))
		{
			printf("ERROR: Too many symbols on PLC\n");
			break;
		}
		tio=0;
		while (!*symbols)
		{
			result = mlpiLogicGetSymbolsOfApplication(_Connection, _WApplication, &node, wsymbols, _countof(wsymbols));
			wchar_to_char(wsymbols, symbols, sizeof(symbols));
			if (MLPI_FAILED(result)) 
				return;
			if (*symbols) break;
			rx_sleep(100);
			if (++tio>10 && _SymbolCnt==0 && !_Error) 
			{
				_Error = 1;
				Error(ERR_ABORT, 0, "No symbols in PLC defined. Check Symbol Configuration!");				
			}
		};
		
	//	printf("                                           Symbols=>>%s<<\n", symbols);
		
		//--- copy the first symbol-name and shift the "symbols" string
		for (i=0; i<SIZEOF(symbols); i++)
		{
			if (symbols[i]==';' || symbols[i]==0x00)
			{
				end = (symbols[i]==0x00);
				pvar->name[i] = 0;
				i++;
				for (n=0; symbols[i];)
				{
					symbols[n++] = symbols[i++];
				}
				symbols[n]=0;
				break;
			}
			pvar->name[i] = symbols[i];
		}
				
		char_to_wchar(pvar->name, wsymbols, SIZEOF(wsymbols));
		result = mlpiLogicGetTypeOfSymbol (_Connection, wsymbols, &pvar->type, &pvar->subType);
		result = mlpiLogicReadVariableBySymbolString(_Connection, wsymbols, wvalue, SIZEOF(wvalue));
		wchar_to_char(wvalue, pvar->value, sizeof(pvar->value));

		TrPrintfL(TRUE, "PLC-Var[%d]:>>%s<< = >>%s<<", _SymbolCnt, pvar->name, pvar->value);
		
		if (node==-1) 
			break;
	}
}

//--- lc_get_var ------------------------------------------------------------------
int lc_get_var (int no, char *name, char *value)
{
	if (no<_SymbolCnt)
	{
		SVariable *pvar = &_Symbols[no];

		strcpy(name, pvar->name);
		strcpy(value, pvar->value);		
		return TRUE;
	}
	return FALSE;
}

//--- lc_get_value --------------------------------------------------------------
int lc_get_value (int no, char *value)
{
	MLPIRESULT result;
	if (no<_SymbolCnt)
	{
		SVariable *pvar = &_Symbols[no];
		wchar_t   wvalue[32];
		wchar_t	  wname[128];
		
		char_to_wchar(pvar->name, wname, SIZEOF(wname));
		result = mlpiLogicReadVariableBySymbolString(_Connection, wname, wvalue, SIZEOF(wvalue));

		wchar_to_char(wvalue, pvar->value, SIZEOF(pvar->value));
		strcpy(value, pvar->value);
		
	//	TrPrintfL(!strcmp(pvar->name, "Application.GUI_00_001_Main.PAR_FLEXO_CONFIGURATION"), "lc_get_value >>%s=%s<<\n", pvar->name, value);				
		return TRUE;
	}
	return FALSE;
}

//--- lc_get_value_by_name --------------------------------------------------------------
int lc_get_value_by_name (char *name, char *value)
{
	int i;
	for (i=0; i<_SymbolCnt; i++)
	{
		if (!strcmp(name, _Symbols[i].name))
		{
			if (strstr(name, "XML_") || RX_Config.printer.type == printer_LH702) 
			{
				strcpy(value, _Symbols[i].value);
				return REPLY_OK;				
			}
			return rex_check_result(lc_get_value(i, value));				
		}
	}
	strcpy(value, "ERROR");
	return REPLY_ERROR;
}

//--- lc_get_value_all ----------------------------
int lc_get_value_all(char *valList, int size)
{
	int len=0;
	int i, appNameLen;
	char value[MAX_PATH];
	
	memset(valList, 0, size);
	strcpy(valList, _Symbols[0].name);
	for (appNameLen=(int)strlen(valList); appNameLen>0 && valList[appNameLen]!='.'; appNameLen--)
	{};
	valList[appNameLen]=0;
	len = appNameLen;
	len += sprintf(&valList[len], "\n");
	appNameLen++;

//	char_to_wchar(pvar->name, wsymbols, SIZEOF(wsymbols));
//		result = mlpiLogicGetTypeOfSymbol (_Connection, wsymbols, &pvar->type, &pvar->subType);
//		result = mlpiLogicReadVariableBySymbolString(_Connection, wsymbols, wvalue, SIZEOF(wvalue));
//		wchar_to_char(wvalue, pvar->value, sizeof(pvar->value));

	for (i=0; i<_SymbolCnt; i++)
	{
		if (_Symbols[i].name[0])
		{
			len+=snprintf(&valList[len], size-len, "%s=", &_Symbols[i].name[appNameLen]);
			if (strstr(_Symbols[i].name, "XML_")) strcpy(value, _Symbols[i].value);
			else lc_get_value(i, value);
			len+=snprintf(&valList[len], size-len, "%s\n", value);				
		}
	}
	return REPLY_OK;
}

//--- lc_get_value_by_name_UINT32 --------------------------------------------------------------
int lc_get_value_by_name_UINT32 (char *name, UINT32 *pvalue)
{
	MLPIRESULT result;
	char	 value[128];

	result = lc_get_value_by_name(name, value);
	if (!strcmp(value,"TRUE")) *pvalue = 1;
	else                       *pvalue = atoi(value);
	#ifndef TEST
		if ( MLPI_FAILED(result)) Error(ERR_CONT, 0, "Error accessing >>%s<<\n", name);		
	#endif
	return rex_check_result(result);
}

//--- lc_get_value_by_name_FLOAT --------------------------------------------------------------
int lc_get_value_by_name_FLOAT (char *name, FLOAT *pvalue)
{
	MLPIRESULT result;
	wchar_t wname[128];

	//TRIM_SPACE(name);
	
	char_to_wchar(name,  wname, SIZEOF(wname));

	result = mlpiLogicReadVariableBySymbolFloat(_Connection, wname, pvalue);
	if ( MLPI_FAILED(result)) 
//			Error(ERR_CONT, 0, "Error accessing >>%s<<\n", name);		
			Error(LOG, 0, "Error accessing >>%s<<\n", name);		
	return rex_check_result(result);
}

//--- lc_set_value_by_name --------------------------------------------------------------
int lc_set_value_by_name (const char *name, char *value)
{
	wchar_t wname[128];
	wchar_t wvalue[128];

	if (strstr(name, "CMD_")) 
		printf("SET >>%s=%s<<\n", name, value);
	else
	{
		int i;
		for (i=0; i<_SymbolCnt; i++)
		{
			if (!strcmp(_Symbols[i].name, name))
			{
				strcpy(_Symbols[i].value, value);
				if (strstr(name, "XML_") || RX_Config.printer.type == printer_LH702) 
					return REPLY_OK;
				break;
			}				
		}	
	}
	if (strstr(name, "XML_") || RX_Config.printer.type == printer_LH702)
	{
		if (_SymbolCnt<SIZEOF(_Symbols))
		{			
			strcpy(_Symbols[_SymbolCnt].name,  name);
			strcpy(_Symbols[_SymbolCnt].value, value);
			_SymbolCnt++;
			return REPLY_OK;
		}
		Error(ERR_CONT, 0, "Symbol Table overflow");
		return REPLY_ERROR;
	}
	
	char_to_wchar(name, wname, SIZEOF(wname));
	char_to_wchar(value, wvalue, SIZEOF(wvalue));
	return (rex_check_result(mlpiLogicWriteVariableBySymbolString(_Connection, wname, wvalue)));
}

//--- lc_set_value_by_name_UINT32 --------------------------------------------------------------
int lc_set_value_by_name_UINT32 (const char *name, UINT32 value)
{
	wchar_t wname[128];
	wchar_t wvalue[128];
	char	svalue[128];

	if (strstr(name, "CMD_")) 
		TrPrintfL(TRUE, "SET >>%s=%d<<\n",  name, value);

	char_to_wchar(name,  wname, SIZEOF(wname));

	sprintf(svalue, "%d", value);
	char_to_wchar(svalue, wvalue, SIZEOF(wvalue));
	if (rex_check_result(mlpiLogicWriteVariableBySymbolString(_Connection, wname, wvalue)))
	{
		#ifndef TEST
			Error(ERR_CONT, 0, "MLPI: setting >>%s<< Error: %s", name, mlpi_get_errmsg());
		#endif
		return REPLY_ERROR;
	}
	return REPLY_OK;
}

//--- lc_set_value_by_name_FLOAT --------------------------------------------------------------
int lc_set_value_by_name_FLOAT (const char *name, double value)
{
	wchar_t wname[128];

	if (strstr(name, "CMD_")) 
		printf("SET >>%s=%.3f<<\n",  name, value);

	char_to_wchar(name, wname, SIZEOF(wname));
	if (rex_check_result(mlpiLogicWriteVariableBySymbolFloat(_Connection, wname, (FLOAT)value)))
	{			
		Error(ERR_CONT, 0, "MLPI: setting >>%s<< Error: %s", name, mlpi_get_errmsg());
		return REPLY_ERROR;
	}
	return REPLY_OK;
}

//--- _filter_matching ----------------------------
static int _filter_matching(char *name, char *filter)
{
	char *read;
	char str[32];
	char *write=str;

	for (read=filter; TRUE;)
	{
		if (*read==0 || *read==';')
		{
			*write=0;
			if (strstr(name, str)) return TRUE;
			if (*read==0) return FALSE;
			read++;
			write=str;
		}
		*write++ = *read++; 
	}
		
	return FALSE;
}

//--- lc_save ------------------------------------------------------------
int lc_save(rx_doc doc, char *filter)
{
	int		i;
	
	TiXmlDocument  *pDoc = (TiXmlDocument*)doc;
	CRX_XmlElement *symbols = new CRX_XmlElement("Symbols"); 

//	printf("lc_save:\n");
	for (i=0; i<_SymbolCnt; i++)
	{
//		printf("var[%d]: >>%s<<\n", i, _Symbols[i].name);
		
		if (_filter_matching(_Symbols[i].name, filter))
		{
			TiXmlElement *par = new TiXmlElement(_Symbols[i].name);
			par->SetAttribute("Value", _Symbols[i].value);
			symbols->LinkEndChild(par);
		}
	}
	pDoc->LinkEndChild(symbols);
	return 0;
}

//--- lc_load --------------------------------------------
int lc_load(rx_doc doc)
{
	char	value[128];
	CRX_XmlElement *symbols;
	CRX_XmlElement *symbol;

	TiXmlDocument	*pDoc = (TiXmlDocument*)doc;
	if (pDoc==NULL) return 1;
	symbols = (CRX_XmlElement*)pDoc->FirstChild("Symbols");
	if (symbols==NULL) symbols = new CRX_XmlElement("Symbols");
	
	for( symbol = (CRX_XmlElement *)symbols->FirstChild(); symbol; symbol = (CRX_XmlElement *)symbol->NextSibling() )
	{
		symbol->Attribute("Value", value, SIZEOF(value), "");
		TrPrintf(TRUE, "lc_load >>%s<<=>>%s<<", symbol->Value(), value);
		lc_set_value_by_name(symbol->Value(), value);
	}

	return 0;
}
