#pragma once

#include "rx_common.h"

typedef  void* rx_doc;

#ifdef __cplusplus
extern "C"{
#endif

DLL_EXPORT rx_doc rx_xml_load(char *path);
DLL_EXPORT rx_doc rx_xml_new ();
DLL_EXPORT int    rx_xml_save(char *path, rx_doc doc);
DLL_EXPORT int    rx_xml_free(rx_doc doc);

#ifdef __cplusplus
}

#include "tinyxml.h"

class CRX_XmlElement : public TiXmlElement
{
public:
	CRX_XmlElement(const char *name);

	int Attribute(const char *name, char *value, int size, const char *def);
//	int Attribute(const char *name, char *value, const char *def);
	int Attribute(const char *name, INT16 *value, INT16 def);
	int Attribute(const char *name, INT32 *value, INT32 def);
	int Attribute(const char *name, UINT32 *value, UINT32 def);
	int Attribute(const char *name, INT64  *value, INT64 def);
	int SetAttribute64(const char *name, INT64 valu);

//	int GetBinFile(const char *filepath, __time64_t time);
//	int SetBinFile(const char *filepath);

//	int SetBinFileCompressed(const char *name, const char *filepath);
//	int GetBinFileCompressed(const char *name, const char *filepath);
	int SetBinFileCompressed(const char *filepath);
	int GetBinFileCompressed(const char *folder);
};

class CRX_XmlDocument: public TiXmlDocument
{
public:
	CRX_XmlElement* InsertBefore(const char *name, const char *addThis);
	CRX_XmlElement* GetEndChild (const char *name);
};

#endif
