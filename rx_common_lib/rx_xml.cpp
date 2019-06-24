//#include "stdafx.h"
#include "rx_xml.h"
#include "fastlz.h"
#include "rx_common.h"
#include "rx_file.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
// #include <io.h>

//--- rx_xml_load ------------------------------------------
rx_doc  rx_xml_load(char *path)
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

//--- rx_xml_new -----------------------------------------
rx_doc  rx_xml_new ()
{
	TiXmlDocument *pDoc = new TiXmlDocument; 

	TiXmlDeclaration	*decl = new TiXmlDeclaration("1.0", "", "");
 	pDoc->LinkEndChild(decl);
	return pDoc;
}

//--- rx_xml_save --------------------------------------
int  rx_xml_save(char *path, rx_doc doc)
{
	TiXmlDocument *pDoc = (TiXmlDocument*) doc;

	return pDoc->SaveFile(path);
}

//--- rx_xml_free ------------------------------------
int  rx_xml_free(rx_doc doc)
{
	delete (TiXmlDocument*)doc;
	return 0;
}


//--- InsertBefore -------------------------------------------------
CRX_XmlElement* CRX_XmlDocument::InsertBefore(const char *name, const char *addThis)
{
	TiXmlNode		*node;
	node = FirstChild(name);
	if (node==NULL) return (CRX_XmlElement*)LinkEndChild(new CRX_XmlElement(addThis));
	else return (CRX_XmlElement*)InsertBeforeChild(node, (const TiXmlNode&)*(new CRX_XmlElement(addThis)));
}

//--- LinkEndChild -------------------------------------------------------------
CRX_XmlElement* CRX_XmlDocument::GetEndChild(const char *name)
{
	TiXmlNode	*node;
	node = FirstChild(name);
	if (node==NULL) return (CRX_XmlElement*) LinkEndChild(new CRX_XmlElement(name));
	else          	return (CRX_XmlElement*) node->ToElement();
}

//--- Constructor -----------------------------------
CRX_XmlElement::CRX_XmlElement(const char *name) : TiXmlElement(name)
{
}

//--- Attribute (string) ---------------------------------
int CRX_XmlElement::Attribute(const char *name, char *value, int size, const char *def)
{
	const char *str = TiXmlElement::Attribute(name);
	if (str) strncpy(value, str, size);
	else strncpy(value, def, size);
	return (str!=NULL);
}

//--- Attribute (INT16) ---------------------------------
int CRX_XmlElement::Attribute(const char *name, INT16 *value, INT16 def)
{
	int i=def;
	TiXmlElement::Attribute(name,	&i);
	*value=i;
	return 0;
}

//--- Attribute (INT32) ---------------------------------
int CRX_XmlElement::Attribute(const char *name, INT32 *value, INT32 def)
{
	int i=def;
	TiXmlElement::Attribute(name,	&i);
	*value=i;
	return 0;
}

//--- Attribute (UINT32) ---------------------------------
int CRX_XmlElement::Attribute(const char *name, UINT32 *value, UINT32 def)
{
	int i=def;
	TiXmlElement::Attribute(name,	&i);
	*value=i;
	return 0;
}

//--- Attribute (INT64) ---------------------------------
int CRX_XmlElement::Attribute(const char *name, INT64 *value, INT64 def)
{
	*value = def;
	const char *str = TiXmlElement::Attribute(name);
	char format[10];
	format[0] = '%';
	strcpy(&format[1], PRId64);
	if (str!=NULL) sscanf(str, format, value);
	return 0;
}

//--- SetAttribute (INT64) ---------------------------------
int CRX_XmlElement::SetAttribute64(const char *name, INT64 value)
{
	char str[32];
	char format[10];
	format[0] = '%';
	strcpy(&format[1], PRId64);
	sprintf(str, format,  value);
	TiXmlElement::SetAttribute(name, str);
	return 0;
}

//--- GetBinFile ---------------------------------
/*
int CRX_XmlElement::GetBinFile(const char *filepath, __time64_t time)
{
	const char		*data;
	BYTE			d;
	FILE			*file;

	data=TiXmlElement::GetText();
	if (data==NULL) return 1;
	
	if (get_file_mtime(filepath)>=time) return 0;

	file = fopen(filepath, "wb");
	if (file!=NULL)
	{
		while(*data)
		{
			if (*data>='0' && *data<='9') d=(BYTE)(*data-'0'); else d=(BYTE)(*data-'A'+10);
			data++;
			d*=0x10;
			if (*data>='0' && *data<='9') d|=(BYTE)(*data-'0'); else d|=(BYTE)(*data-'A'+10);
			data++;
			fwrite(&d, 1, 1, file);
		}
		fclose(file);
		set_file_mtime(filepath, time);
		return 0;
	}
	return 1;
}
*/

//--- GetBinFileCompressed ----------------------------------------------------
/*
int CRX_XmlElement::GetBinFileCompressed(const char *name, const char *filepath)
{
	const char		*src;
	int				size, len;
	BYTE			*compr, *dst;
	BYTE			*buffer;
	FILE			*file;
	int				filesize;
	__time64_t		time;
	char			str[256];

	CRX_XmlElement	*data = (CRX_XmlElement*)this->FirstChild(name);
	if (data==NULL) return 1;
	src=data->GetText();
	if (src==NULL) return 1;
	data->Attribute("Name", str, SIZEOF(str), "not found");
	data->Attribute("Time",	&time, 0);
	data->Attribute("Size",	&filesize, 0);

	if (rx_file_get_size(filepath)==filesize && rx_file_get_mtime(filepath)>=time) return 0;
	file = fopen(filepath, "wb");
	if (file!=NULL)
	{
		size   = strlen(src);
		compr  = (BYTE*)malloc(size/2);
		buffer = (BYTE*)malloc(filesize+10);
		for(dst=compr; *src; dst++)
		{
			if (*src>='0' && *src<='9') *dst=0x10*(BYTE)(*src-'0'); else *dst=0x10*(BYTE)(*src-'A'+10);
			src++;
			if (*src>='0' && *src<='9') *dst|=(BYTE)(*src-'0'); else *dst|=(BYTE)(*src-'A'+10);
			src++;
		}
		len=fastlz_decompress(compr, size/2, buffer, filesize);
		fwrite(buffer, 1, filesize, file);
		fclose(file);
		free(compr);
		free(buffer);
		return 0;
	}
	return 1;
}
*/

int CRX_XmlElement::GetBinFileCompressed(const char *folder)
{
	const char		*src;
	int				size, len;
	BYTE			*compr, *dst;
	BYTE			*buffer;
	FILE			*file;
	INT32			filesize;
	INT64			time;
	char			filepath[256];

	CRX_XmlElement	*data = this;
	if (data==NULL) return 1;
	strcpy(filepath, folder);
	len = (int)strlen(filepath);
	data->Attribute("Name", &filepath[len], SIZEOF(filepath)-len, "not found");
	data->Attribute("Time",	&time, 0);
	data->Attribute("Size",	&filesize, 0);

	src=data->GetText();
	if (src==NULL) return 1;

	if (rx_file_get_size(filepath)==filesize && rx_file_get_mtime(filepath)>=time) return 0;
	file = fopen(filepath, "wb");
	if (file!=NULL)
	{
		size   = (int)strlen(src);
		compr  = (BYTE*)malloc(size/2);
		buffer = (BYTE*)malloc(filesize+10);
		for(dst=compr; *src; dst++)
		{
			if (*src>='0' && *src<='9') *dst=0x10*(BYTE)(*src-'0'); else *dst=0x10*(BYTE)(*src-'A'+10);
			src++;
			if (*src>='0' && *src<='9') *dst|=(BYTE)(*src-'0'); else *dst|=(BYTE)(*src-'A'+10);
			src++;
		}
		len=fastlz_decompress(compr, size/2, buffer, filesize);
		fwrite(buffer, 1, filesize, file);
		fclose(file);
		free(compr);
		free(buffer);
		return 0;
	}
	return 1;
}

//--- SetBinFile ------------------------------------------------------
/*
int CRX_XmlElement::SetBinFile(const char *filepath)
{
	FILE			*file;
	int				size;
	char			*buffer;
	char			*buf;
	UCHAR			data;

	file = fopen(filepath, "rb");
	if (file!=NULL)
	{
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		fseek(file, 0, SEEK_SET);
		buffer = (char*)malloc(2*size+2);
		for(buf=buffer; size; buf+=2, size--)
		{
			fread(&data, 1, 1, file);
			sprintf(buf, "%02X", data);
		}
		TiXmlText *text = new TiXmlText(buffer);
		LinkEndChild(text);
		free(buffer);
	}
	return NULL;
}
*/

//--- SetBinFileCompressed ---------------------------------------------
int CRX_XmlElement::SetBinFileCompressed(const char *filepath)
{
	FILE			*file;
	int				size, compressed;
	char			filename[128];
	char			*buffer;
	char			*compr;
	char			*dst;
	UCHAR			*src;


	if (*filepath==0) return 0;

	file = fopen(filepath, "rb");
	if (file!=NULL)
	{
		CRX_XmlElement *data = new CRX_XmlElement("File");

		split_path(filepath, NULL, filename, NULL);
		data->SetAttribute("Name", filename);
		data->SetAttribute64("Time", rx_file_get_mtime(filepath));

		fseek(file, 0, SEEK_END);
		size = ftell(file);

		data->SetAttribute("Size",	size);

		fseek(file, 0, SEEK_SET);
		buffer = (char*)malloc(2*size);
		compr  = (char*)malloc(6*size/5+2048);
		fread(buffer, 1, size, file);
		fclose(file);
		compressed = fastlz_compress(buffer, size, compr);
		for(src=(UCHAR*)compr, dst=buffer; compressed; dst+=2, compressed--)
		{
			sprintf((char*)dst, "%02X", *src++);
		}
		TiXmlText *text = new TiXmlText((char*)buffer);
		data->LinkEndChild(text);
		LinkEndChild(data);

		free(buffer);
		free(compr);
		return 0;
	}
	return 1;
}
