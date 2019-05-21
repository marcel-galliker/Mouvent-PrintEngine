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
#include "string.h"
// #include "atlimage.h"
#include <locale.h>
#include "tinyxml.h"
#include "rx_common.h"
#include "rx_file.h"
#include "rx_xml.h"
#include "rx_counter.h"
#include "rx_data.h"

//--- defines ----------------------------------------------
#define MAX_RECORD_SIZE	0x10000
#define LF	0x0a
#define CR	0x0d

//--- globals -----------------------------------------------
SFileDef	_FileDef;
SFileDef	_FileDefOrg;

FILE		*_File;
INT64		_FilePos;
INT32		_RecNo;
// _locale_t	_Locale = NULL;
static char _DataFile[MAX_PATH];

//--- record buffer ---------------------------
char		*_BufferA=NULL;
UTF16		*_BufferW = NULL;
SFieldDef	_FieldPos[DATA_FIELD_CNT];

//--- help functions -----------------
static int	_read_record(wchar_t *buffer, int maxSize, int *plen);
static void _file_close();
static int  _file_open();
static int	_file_read_record(int *plen);
static void	_count_records(SFileDef *pFileDef, SFileDef *pFileDefOrg);
static int _split_fields(int len);

//--- dat_max_record_size ------------
int  dat_max_record_size()
{
	return MAX_RECORD_SIZE;
}

//--- dat_load_file_def ----------------------------------
int dat_load_file_def(void *doc, char *tempPath, SFileDef *pFileDef)
{
	if (pFileDef->size!=sizeof(SFileDef)) 
	{
		int i=0;
		i=1/i;
		return 1;
	}
	memset(pFileDef, 0, sizeof(SFileDef));
	pFileDef->size = sizeof(SFileDef);

	if (!doc) return 0;

	TiXmlDocument	*pDoc = (TiXmlDocument*)doc;

	CRX_XmlElement	*file;
	CRX_XmlElement	*field;
	char str[128];
	
	if (pDoc==NULL) return 2;
	file = (CRX_XmlElement*)pDoc->FirstChild("File");
	if (file==NULL) return 2;
	file->Attribute("DataFile",			pFileDef->dataFile, SIZEOF(pFileDef->dataFile), "DataFile");
	add_root(_DataFile, pFileDef->dataFile, tempPath);

	file->Attribute("Header",			&pFileDef->header,	0);
	file->Attribute("CodePage",			&pFileDef->codePage, 852);
	file->Attribute("RecordCnt",		&pFileDef->recordCnt, 0);
	file->Attribute("FileFormat",		(INT32*)&pFileDef->fileFormat, FF_Fixed);
	file->Attribute("RecLen",			&pFileDef->recLen, 100);
	file->Attribute("FieldSeparator",	(INT16*)&pFileDef->fieldSep, 0x09);
	file->Attribute("Filter",			(INT32*)&pFileDef->filter, 0);

	for (int i=0; i<SIZEOF(pFileDef->field); i++)
	{
		snprintf(str, SIZEOF(str), "Field%02d", i);
		field = (CRX_XmlElement*)file->FirstChild(str);
		if (field==NULL) break;
		field->Attribute("Name",	pFileDef->field[i].name, SIZEOF(pFileDef->field[i].name), str);
		field->Attribute("Pos",		&pFileDef->field[i].pos, 0);
		field->Attribute("Len",		&pFileDef->field[i].len, 0);
	}
	memcpy(&_FileDefOrg, pFileDef, sizeof(_FileDefOrg));
	return 0;
}

//--- dat_save_file_def ----------------------------------
int dat_save_file_def(void *doc, char *tempPath, SFileDef *pFileDef, char *dstPath)
{
	if (pFileDef->size!=sizeof(SFileDef)) return 1;

	int i;
	char str[MAX_PATH];

	CRX_XmlDocument		*pDoc = (CRX_XmlDocument*)doc;
	
	_count_records(pFileDef, &_FileDefOrg);

	//--- write file definition --------------------------
	CRX_XmlElement *file = pDoc->InsertBefore("Files", "File");
	split_path(pFileDef->dataFile, NULL, str, NULL);
	file->SetAttribute("DataFile",			str);
	file->SetAttribute("Header",			pFileDef->header);
	file->SetAttribute("CodePage",			pFileDef->codePage);
	file->SetAttribute("RecordCnt",			pFileDef->recordCnt);
	file->SetAttribute("FileFormat",		pFileDef->fileFormat);
	file->SetAttribute("RecLen",			pFileDef->recLen);
	file->SetAttribute("FieldSeparator",	pFileDef->fieldSep);
	file->SetAttribute("Filter",			pFileDef->filter);

	//--- write fields ------------------------------------
	for (i=0; i<SIZEOF(pFileDef->field); i++)
	{
		if (!*pFileDef->field[i].name) break;
		snprintf(str, SIZEOF(str), "Field%02d", i);
		TiXmlElement *field = new TiXmlElement(str);
		field->SetAttribute("Name", pFileDef->field[i].name);
		field->SetAttribute("Pos", pFileDef->field[i].pos);
		field->SetAttribute("Len", pFileDef->field[i].len);
		file->LinkEndChild(field);
	}
	//--- add file -------------------------------
	/*
	{	
		add_root(str, pFileDef->dataFile, tempPath);
		pDoc->GetEndChild("Files")->SetBinFileCompressed(str);
	}
	*/
	#ifdef WIN32
	{
		char path[MAX_PATH];
		split_path(dstPath, path, NULL, NULL);
		split_path(pFileDef->dataFile, NULL, &path[strlen(path)], NULL);
		if (strcmp(pFileDef->dataFile, path)) CopyFile(pFileDef->dataFile, path, FALSE);
	}
	#endif
	// all pointers are deleted in the DOC destructor!
	return 0;	
}

//--- _file_close ------------------------------------------
static void _file_close()
{
//	if (_Locale) _free_locale(_Locale);
	if (_File) fclose(_File);
//	_Locale = NULL;
	_File=NULL;
}

//--- _file_open -------------------------------------------
static int _file_open()
{
	_file_close();
	_File = fopen(_DataFile, "rb");
	_FilePos	= 0;
	_RecNo		= 0;
	
	if (_BufferW==NULL) _BufferW=(UTF16*)  malloc(2*MAX_RECORD_SIZE);
	if (_BufferA==NULL) _BufferA=(char*)   malloc(2*MAX_RECORD_SIZE);
	return 0;
}

//--- dat_set_file_def ---------------------------------------------------------
void dat_set_file_def (SFileDef *pFileDef)
{
	char root[MAX_PATH];
	if (strcmp(_FileDef.dataFile, pFileDef->dataFile))
	{
		_file_close();
		split_path(pFileDef->dataFile, root, NULL, NULL);
		if (*root) strcpy(_DataFile, pFileDef->dataFile);
	}
	memcpy(&_FileDef, pFileDef, sizeof(_FileDef));

	if (_File==NULL) _file_open();

//	if (_Locale) _free_locale(_Locale);
	if (_FileDef.codePage>0)
	{
		char str[32];
		snprintf(str, sizeof(str), ".%d", _FileDef.codePage);
		setlocale(LC_CTYPE, str);	//	_Locale = _create_locale(LC_CTYPE, str);
		_FileDef.unicode = (_FileDef.codePage==1200);
	}
	else setlocale(LC_CTYPE, ".ACP");	// _Locale = _create_locale(LC_CTYPE, ".ACP");
		
}

//--- dat_seek ------------------------------------------------------------------
int dat_seek (int recNo)
{
	int len;
	int charsize= _FileDef.unicode?2:1;

	if (_File==NULL) 
	{
		_RecNo = recNo;
		return 0;
	}
	if (recNo<0) return 1;
	_RecNo   = recNo;

	_FilePos = 0;
	if (_FileDef.fileFormat==FF_Fixed) _FilePos = _FileDef.header*charsize+recNo*_FileDef.recLen;
	else
	{
		_FilePos = (_FileDef.header+1)*charsize;
		while (recNo) 
		{
			_file_read_record(&len); // seeking variable record file
			recNo--;
		}
	}
	return 0;
}

//--- _file_read_record --------------------------------------
static int	_file_read_record(int *plen)
// reads one record into _BufferW
{
	UINT	len, i;
	UINT16 *chw;

	*plen=0;
	if (_File==NULL) return 0;

//	TRACE(L"Seek %d\n", m_filePos);
	rx_file_seek(_File, _FilePos, SEEK_SET);
	
	if (_FileDef.fileFormat==FF_Fixed)
	{
		if (_FileDef.recLen>MAX_RECORD_SIZE) len=MAX_RECORD_SIZE;
		else								 len=(UINT)_FileDef.recLen;
		if (_FileDef.unicode)
		{
			len = (int)fread(_BufferW, 2, len, _File);
			_FilePos += 2*len;
		}
		else
		{
			len = (int)fread(_BufferA, 1, len, _File);
//			_mbstowcs_s_l(&ret, _BufferW, MAX_RECORD_SIZE, (const char*)_BufferA, len, _Locale);
			rx_mbstowcs(_BufferW, (const char*)_BufferA, len);
			_FilePos += len;
		}
		*plen = len;
		return *plen;
	}
	else
	{
		if (_FileDef.unicode)
		{
			len = (int)fread(_BufferW, 2, MAX_RECORD_SIZE-1,_File);
		}
		else
		{
			len = (int)fread(_BufferA, 1, MAX_RECORD_SIZE-1, _File);
//			_mbstowcs_s_l(&ret, _BufferW, MAX_RECORD_SIZE, _BufferA, len, _Locale);
			rx_mbstowcs(_BufferW, _BufferA, len);
		}

		//--- search cr/lf ---
		for (i=0, chw=(UINT16*)_BufferW; i<len && *chw!=CR && *chw!=LF; i++, chw++)
		{
		}
		*plen = i;
		if (_FileDef.unicode)	_FilePos += 2*i;
		else					_FilePos += i;
		while (i<len && *chw<' ') 
		{
			i++;
			chw++;
			if (_FileDef.unicode)	_FilePos+=2;
			else                    _FilePos++;
		}

//		if (*plen > _FileDef->rec.lenMax)  m_pFileDef->rec.lenMax=(*plen);
//		if (m_pFileDef->rec.lenMin && *plen<m_pFileDef->rec.lenMin)	m_pFileDef->rec.lenMin=(*plen);
	}
	return *plen;
}

//--- dat_read_next_record --------------------------------------------------------
int  dat_read_next_record ()
{
	int len;

	memset(_FieldPos, 0, sizeof(_FieldPos));
	len=_file_read_record(&len);
	if (_File==NULL) 
	{
		_RecNo++;
		return 1;
	}
	if (len==0) 
		return 0;
	_RecNo++;
	_split_fields(len);
	return len;
}

//--- _split_fields -------------------------------------------
static int _split_fields(int len)
{
	int fld;
	int pos, p;
	UTF16	filter; 

	//--- split fields -----------------------
	memcpy(_FieldPos, _FileDef.field, sizeof(_FieldPos));
	if (_FileDef.fileFormat==FF_Fixed)
	{
		return len;
	}
	else
	{
		filter=0;
		for (p=pos=0, fld=0; p<=len; p++)
		{
			// ignore record-separators within quotations
			if (_FileDef.filter && p==pos)
			{
				filter =  _BufferW[p];
				if (filter!='\'' && filter!='"') filter=0;
			}
			if (p!=pos && filter && _BufferW[p]==filter) filter=0;

			if (!filter && (_BufferW[p]==_FileDef.fieldSep || p==len))
			{
				if (fld<SIZEOF(_FieldPos))
				{
					_FieldPos[fld].pos = pos;
					_FieldPos[fld].len = p-pos;
					if (_FileDef.filter && _FieldPos[fld].len>2)
					{
						if ((_BufferW[pos]=='\'' && _BufferW[p-1]=='\'') 
						||  (_BufferW[pos]=='"'  && _BufferW[p-1]=='"' ) 
						)
						{
							_FieldPos[fld].pos++;
							_FieldPos[fld].len-=2;
						}
					}
					fld++;
					pos = p+1;
				}
			}
		}
		
		//--- TEST -------------------------
		if (FALSE)
		{
			int i, len;
			wchar_t wstr[64];
			char str[64];
			printf("--- DATA -------------------------\n");
			for (i=0; i<fld; i++)
			{
				len = dat_get_field(i, (BYTE*)wstr, sizeof(wstr));
				wchar_to_char(wstr, str, sizeof(str));
				
				printf("Field[%d]: >>%s<<\n", i, str);
			}
		}
		return p;
	}
}


//--- dat_get_field -----------------------------
int  dat_get_field(int no, BYTE *data, int maxStrLen)
{
	int len;
	if (no<SIZEOF(_FieldPos))
	{
		if (_FieldPos[no].len<maxStrLen) len=_FieldPos[no].len*2;
		else len=maxStrLen-1;
		memcpy(data, &_BufferW[_FieldPos[no].pos], len);
		data[len]=0;
		return len;
	}
	return 0;
}

//--- dat_get_buffer -----------------------------
int  dat_get_buffer(int pos, int len, BYTE *data)
{
	len *=2;
	memcpy(data, &_BufferW[pos], len);
	return len;
}

//--- dat_set_buffer -----------------------------
int  dat_set_buffer(int pos, int len, BYTE *data)
{
	if (_BufferW==NULL) _BufferW=(UTF16*)malloc(2*MAX_RECORD_SIZE);

	memcpy(&_BufferW[pos], data, 2*len);
	_split_fields(len);
	return len;
}

//--- GetField ------------------------------------------------------
int GetField(char *fieldname, UTF16 *dst, int maxStrLen)
{
	UINT no, len;

	//--- counter ----------------------------------------
	if (!strcmp(fieldname, "Counter"))
	{
		char str[64];
		ctr_get_counter(CTR_Counter, str);
//		_mbstowcs_s_l(&len, dst, maxStrLen, (const char*)str, sizeof(str), _Locale);
		len = (int)rx_mbstowcs(dst, (const char*)str, min(strlen(str), sizeof(str)));
		return len;
	}

	//--- data field ---------------------------------------
	for (no=0; no<SIZEOF(_FieldPos); no++)
	{
		if (!strcmp(fieldname, _FieldPos[no].name))
		{
			if (_FieldPos[no].len<maxStrLen) len=_FieldPos[no].len;
			else len=maxStrLen-1;
			memcpy(dst, &_BufferW[_FieldPos[no].pos], 2*len);
			dst[len]=0;
			return len;
		}
	}
	
	//--- field not found ---
	return 0;
}

//--- dat_interprete_data_fields -------------------------------------------
int dat_interprete_data_fields(UTF16 *dst, UTF16 *srcStr, int size)
{
	int len, l;
	UTF16 *src;
	char fieldname[FIELD_NAME_LEN];

	dst[size-1]=0;
	for (src=srcStr, len=0; *src && len<size; src++)
	{
		if (len>=size) return 1;
		if (*src==(UTF16)'{') // replace field
		{
			for (l=0; *(++src);) 
			{
				if (*src==(UTF16)'}')
				{
					fieldname[l]=0;
					len += GetField(fieldname,	&dst[len], size-len);
					break;
				}
				if (l<FIELD_NAME_LEN-1) fieldname[l++]=(char)*src;
			}
		}
		else dst[len++]=*src;
	}
	dst[len]=0;
	return 0;
}

//--- dat_analyse_file ---------------------------------------------
static void _findchar(BYTE *buffer, int size, char ch, int *pos, int maxCnt, int *cnt);
static void _analyse_record(BYTE *buffer, int size, SFileDef *pFileDef);
static void _analyse_fixed_fields(BYTE *buffer, int size, SFileDef *pFileDef);
static void _analyse_variable_fields(BYTE *buffer, int size, SFileDef *pFileDef);

int dat_analyse_file(SFileDef *pFileDef)
{
	FILE	*file;
	int		 bufSize, len;
	BYTE	*bufferA;

	file = fopen(pFileDef->dataFile, "rb");
	memset(pFileDef->field, 0, sizeof(pFileDef->field));
	if (file!=NULL)
	{
		bufSize = 10*MAX_RECORD_SIZE;
		bufferA = (BYTE*)malloc(bufSize);
		if (bufferA!=NULL)
		{
			len = (int)fread(bufferA, 1, bufSize, file);
			_analyse_record(bufferA, len, pFileDef);
			free(bufferA);
		}
		fclose(file);
	}
	return 0;
}

//--- isUnicode ------------------------------------------------------------
static int isUnicode(BYTE *buffer, int size)
{
	int i;
	int cnt;
	
	if (buffer[0]==0xff && buffer[1]==0xfe) return TRUE;
	for (i=0, cnt=0; i<size; i++)
	{
		if (!buffer[i]) cnt++;
	}

	return (cnt > size/3);
}

//--- _analyse_record ---------------------------------------------------------------
static void _analyse_record(BYTE *buffer, int size, SFileDef *pFileDef)
{
	int		pos[100];
	int		cnt, i, r, recLen, n;

	pFileDef->unicode = isUnicode(buffer, size);
	//--- check for CR or LF ---
	_findchar(buffer, size, CR, pos, SIZEOF(pos), &cnt);
	if (cnt==0) _findchar(buffer, size, LF, pos, SIZEOF(pos), &cnt);
	recLen = 0;
	n=0;
	for (i=cnt-2; i>0; i--)
	{
		r = pos[i] - pos[i-1];
		if (recLen==0) recLen=r;
		if (recLen==r) n++; 
	}
	if (10*n>9*cnt) //--- fixed format file ! ---
	{
		// record end can be CR, LF, CR/LF
		if (buffer[pos[n]]==CR && buffer[pos[n]+1]==LF)
		{
			
		}
		pFileDef->fileFormat = FF_Fixed;
		pFileDef->recLen	 = recLen;
		
		//--- check for header -----
		i = pos[0];
		while (buffer[i]==CR || buffer[i]==LF) i++;
		if (i==recLen) pFileDef->header=0;
		else pFileDef->header=pos[1]-recLen+1;

		_analyse_fixed_fields(buffer, size, pFileDef);
	}
	else //--- variable format ---
	{
		pFileDef->fileFormat = FF_Variable;
		pFileDef->recLen	 = recLen;
		_analyse_variable_fields(buffer, size, pFileDef);
		if (pFileDef->fieldSep==0) pFileDef->fileFormat = FF_Fixed;
	}
}

//--- _analyse_fixed_fields -------------------------------------------------------------
static void _analyse_fixed_fields(BYTE *buffer, int size, SFileDef *pFileDef)
{
	int test=TRUE;

	int	  pos, i, cnt, n;
	int  recLen;
	BYTE *rec;
	UCHAR *counter;
	char separators[256];

	//--- ignore CR/LF at record end -----------
	recLen = pFileDef->recLen;
	while (recLen && buffer[pFileDef->header + recLen - 1] == CR || buffer[pFileDef->header + recLen - 1] == LF) recLen--;

	//--- init counters --------------
	memset(separators, 0, sizeof(separators));
	separators[',']  = TRUE;
	separators[';']  = TRUE;
	separators[0x09] = TRUE;

	counter = (UCHAR*)malloc(pFileDef->recLen);
	memset(counter, 0, pFileDef->recLen);

	//--- count possible field starts per coloumn
	for (pos=pFileDef->header, cnt=0; pos<size && cnt<5; pos+=pFileDef->recLen, cnt++)
	{
		rec = &buffer[pos];

		/*
		#ifdef WIN32
		if (test)
		{
			rec[pFileDef->recLen-2]=0;
			OutputDebugString("rec: ");
			OutputDebugString((char*)rec);
			OutputDebugString("\n");
		}
		#endif
		*/
		for (i=0; i<recLen; i++)
		{
			if (i==0) counter[i]++;
			else if (separators[rec[i-1]]) counter[i]++;
			else if (rec[i-1]==' ' && (rec[i]>' ') && !separators[rec[i]]) counter[i]++;  
		}
	}
	/*
	#ifdef WIN32
	if (test)
	{
		OutputDebugString("ctr: "); 
		for (i=0; i<pFileDef->recLen; i++)
		{
			str[0]='0'+counter[i];
			str[1]=0;
			OutputDebugString(str);
		}
		OutputDebugString("\n");
	}
	#endif
	*/

	//--- calculate fields, where several possible fieldstarts are at same coloumn
	for (i=0, n=0; i<recLen && n<SIZEOF(pFileDef->field); i++)
	{
		if (10*counter[i]>9*cnt) // start of field
		{
			if (n>0)
			{
				pFileDef->field[n-1].len = i-pFileDef->field[n-1].pos;
				if (separators[rec[i-1]]) pFileDef->field[n-1].len--;
			}
			snprintf(pFileDef->field[n].name, SIZEOF(pFileDef->field[n].name), "Field %d", n+1);
			pFileDef->field[n].pos = i;
			n++;
		}
	}
	if (n>0)pFileDef->field[n-1].len = i-pFileDef->field[n-1].pos;
	free(counter);
}

//--- _analyse_variable_fields -------------------------------------------------------------
static void _analyse_variable_fields(BYTE *buffer, int size, SFileDef *pFileDef)
{
	char *ch;
	int n;

	n=0;
	for (ch=(char*)&buffer[pFileDef->header]; *ch!=CR && *ch!=LF; ch++)
	{
		if (n==0 && (*ch==';' || *ch==0x09 || *ch==',')) pFileDef->fieldSep=*ch;
		if (*ch==pFileDef->fieldSep) snprintf(pFileDef->field[n++].name, SIZEOF(pFileDef->field[n++].name), "Field %d", n+1);
	}
	snprintf(pFileDef->field[n].name, SIZEOF(pFileDef->field[n].name), "Field %d", n+1);
}

//--- _findchar ---------------------------------------------------------------
static void _findchar(BYTE *buffer, int size, char ch, int *pos, int maxCnt, int *cnt)
{
	int i;
	*cnt=0;
	for (i=0; i<size && *cnt<maxCnt-1; i++)
	{
		if (buffer[i]==ch) pos[(*cnt)++]=i;
	}
}

//--- _count_records ---------------------------------------------------
static void _count_records(SFileDef *pFileDef, SFileDef *pFileDefOrg)
{
	INT64 size;
	FILE *file;
	char buffer[2048], *ch;
	int len, i;

	if (!strcmp(pFileDef->dataFile, pFileDefOrg->dataFile)
	&&   pFileDef->fileFormat == pFileDefOrg->fileFormat
	&&   pFileDef->header     == pFileDefOrg->header
	&&	 pFileDef->recLen	  == pFileDefOrg->recLen
	&&	 pFileDef->recordCnt>0)
		return;	

	size = rx_file_get_size(_DataFile);
	file = fopen(_DataFile, "rb");
	if (file!=NULL)
	{
		if (pFileDef->fileFormat==FF_Fixed) 
		{
			pFileDef->recordCnt = (int)((size-pFileDef->header)/pFileDef->recLen);
		}
		else
		{
			pFileDef->recordCnt=0;
			while (TRUE)
			{
				len = (int)fread(buffer, 1, sizeof(buffer), file);
				if (len<=0) break;
				for (i=len, ch=buffer; i; i--)
				{
					if (*ch==CR || *ch==LF) 
					{
						pFileDef->recordCnt++;
						ch++;
						if (*ch==CR || *ch==LF)  ch++;
					}
					else ch++;
				}
			}
		}
		fclose(file);
	}
}