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
// #include "atlimage.h"
#include <locale.h>
#include "rx_common.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_xml.h"
#include "rip.h"
// #include "ErrorRip.h"
#include "rx_rip_lib.h"
#include "rx_threads.h"
#include "tinyxml.h"
#include "rx_data.h"
#include "TCP_IP_OEM.h"
#include "rx_barcodes.h"
#include "rx_free_type.h"
#include "tbarcode.h"

//--- Bitmap ----------------------------
static RX_Bitmap	_Bmp;

//--- Prototypes ---------------------------------
static void _set_box_hdr(SBoxHeader *hdr, SBoxHeaderDef *pHdr, int type, double factX, double factY);

//--- rip_init -------------------------
int rip_init()
{
	ft_init(1200, 1200, 1);
	return 0;
}

//--- rip_add_fonts ----------------------------
int rip_add_fonts(char *path)
{
	ft_enum_fonts(path);
	return 0;
}

//--- rip_open_xml_local ---------------------------------------------
rx_doc rip_open_xml_local(const char *path, const char *tempPath)
{
	char localPath[MAX_PATH];
	int len, reload;
	rx_doc pDoc=NULL;
	TiXmlNode *root;

	//--- set local path ---
	strcpy(localPath, tempPath);
	len =(int)strlen(localPath);
	if (localPath[len-1]!='/') localPath[len++]='/';
	split_path(path, NULL, &localPath[len], NULL);

	if (!rx_file_del_if_older(localPath, path))
		pDoc = rx_xml_load(localPath);
	
	//--- found local file: check files --- 
	if (pDoc)
	{
		TiXmlDocument *doc = (TiXmlDocument*)pDoc;

		root = doc->FirstChild("RXDFile");
		if (!root) 	root = doc;

		CRX_XmlElement *files = (CRX_XmlElement*)root->FirstChild("Files");
		if (files!=NULL)
		{
			CRX_XmlElement *file;
			char name[MAX_PATH];
			INT64		time, time1;
		//	int				size;
		
			strcpy(name, tempPath);
			reload = FALSE;
			for (file = (CRX_XmlElement *)files->FirstChild(); file; file = (CRX_XmlElement *)file->NextSibling())
			{
				file->Attribute("Name", &name[len], SIZEOF(name) - len, "not found");
				file->Attribute("Time", &time, 0);
		//	file->Attribute("Size",	&size, 0);
				time1 = rx_file_get_mtime(name);
				if (time1 < time)
				{
					remove(name);
					reload = TRUE;
				}
			}
			if (reload) 
			{
				rx_xml_free(pDoc);
				pDoc = NULL;
			}
		}
	}
	
	if (pDoc==NULL)
	{
		//--- no local file ---
		pDoc = rx_xml_load((char*)path);
		if (pDoc)
		{
			TiXmlDocument *doc = (TiXmlDocument*)pDoc;
			root = doc->FirstChild("RXDFile");
			if (!root) root = doc;

			CRX_XmlElement *files = (CRX_XmlElement*)root->FirstChild("Files");
			if (files)
			{
				CRX_XmlElement *file, *filesOrg;
				TiXmlNode *data;
				filesOrg = (CRX_XmlElement*)files->Clone();
				for( file = (CRX_XmlElement *)files->FirstChild(); file; file = (CRX_XmlElement *)file->NextSibling() )
				{
					data=file->LastChild();
					file->RemoveChild(data);
				}
				doc->SaveFile(localPath);
				doc->RemoveChild(files);
				doc->LinkEndChild(filesOrg);	
			}
		}
	}
	return (rx_doc) root;
}

//--- rip_enum_fonts -------------------------------
void   rip_enum_fonts(rx_doc *pDoc, font_callback callback)
{
	TiXmlDocument *doc = (TiXmlDocument*)pDoc;
	CRX_XmlElement *files = (CRX_XmlElement*)doc->FirstChild("Files");
	CRX_XmlElement *file;
	char name[MAX_PATH];

	for( file = (CRX_XmlElement *)files->FirstChild(); file; file = (CRX_XmlElement *)file->NextSibling() )
	{
		file->Attribute("Name", name, SIZEOF(name), "");
		if (!_stricmp(&name[strlen(name)-4], ".ttf")) callback(name);
	}	
}

//--- rip_load_layout ----------------------------------
int rip_load_layout(void *doc, char *tempPath, SLayoutDef *pLayout)
{
	if (pLayout->size!=sizeof(SLayoutDef))
	{
		int error=0;
		error=1/error;
		return 1;
	}
	memset(pLayout, 0, sizeof(SLayoutDef));
	pLayout->size = sizeof(SLayoutDef);

	TiXmlDocument	*pDoc = (TiXmlDocument*)doc;

	CRX_XmlElement	*layout=NULL;
	CRX_XmlElement	*box;
	CRX_XmlElement	*prop;

	char str[MAX_PATH];
	int  i;

	if (pDoc!=NULL) layout = (CRX_XmlElement*)pDoc->FirstChild("Layout");
	if (layout==NULL) layout = new CRX_XmlElement("Layout");

	layout->Attribute("Label",		 pLayout->label,	  sizeof(pLayout->label),      "");
	layout->Attribute("ColorLayer",	 pLayout->colorLayer, sizeof(pLayout->colorLayer), "");

	layout->Attribute("Width",		&pLayout->width,		300000);
	layout->Attribute("Height",		&pLayout->height,		100000);
	layout->Attribute("WebWidth",	&pLayout->webWidth,		300000);
	layout->Attribute("Columns",	&pLayout->columns,		1);
	layout->Attribute("ColumnDist",	&pLayout->columnDist,   0);

	for (i=0; i<SIZEOF(pLayout->box); i++)
	{
		snprintf(str, SIZEOF(str), "Box%02d", i);
		box = (CRX_XmlElement*)layout->FirstChild(str);
		if (box==NULL) break;
		box->Attribute("Type",			&pLayout->box[i].hdr.boxType, BOX_TYPE_TEXT);
		box->Attribute("X",				&pLayout->box[i].hdr.x, 0);
		box->Attribute("Y",				&pLayout->box[i].hdr.y, 0);
		box->Attribute("Width",			&pLayout->box[i].hdr.width, 500);
		box->Attribute("Height",		&pLayout->box[i].hdr.height, 100);
		box->Attribute("Orientation",	&pLayout->box[i].hdr.orientation, 0);
		box->Attribute("Mirror",		&pLayout->box[i].hdr.mirror, false);
		box->Attribute("Content",		pLayout->box[i].hdr.content, SIZEOF(pLayout->box[i].hdr.content), "");
		switch(pLayout->box[i].hdr.boxType)
		{
		case BOX_TYPE_TEXT:
			prop = (CRX_XmlElement*)box->FirstChild("Text");
			if (prop==NULL) break;
			prop->Attribute("Font",			pLayout->box[i].text.font, SIZEOF(pLayout->box[i].text.font), "Arial");
			prop->Attribute("Size",			&pLayout->box[i].text.size, 16);
			prop->Attribute("LineSpace",	&pLayout->box[i].text.lineSpace, 100);
			prop->Attribute("Alignment",	&pLayout->box[i].text.alignment, 0);
			break;

		case BOX_TYPE_BARCODE:
			prop = (CRX_XmlElement*)box->FirstChild("Barcode");
			if (prop==NULL) break;
			prop->Attribute("BcType",		&pLayout->box[i].bc.bcType, 0);
			for (int n=0; n<8; n++)
			{
				snprintf(str, SIZEOF(str), "Bar%02d", n);
				prop->Attribute(str,		&pLayout->box[i].bc.bar[n], 4*(n+1));
			}
			for (int n=0; n<8; n++)
			{
				snprintf(str, SIZEOF(str), "Space%02d", n);
				prop->Attribute(str,		&pLayout->box[i].bc.space[n], 4*(n+1));
			}
			prop->Attribute("Stretch",		&pLayout->box[i].bc.stretch, 1);
			prop->Attribute("Filter",		&pLayout->box[i].bc.filter, 0);
			prop->Attribute("CodePage",		&pLayout->box[i].bc.codePage, 0);
			prop->Attribute("Check",		&pLayout->box[i].bc.check, false);
			prop->Attribute("Size",			&pLayout->box[i].bc.size, 0);
			prop->Attribute("Format",		&pLayout->box[i].bc.format, 0);
			prop->Attribute("Mask",			&pLayout->box[i].bc.mask, 0);
			prop->Attribute("Rows",			&pLayout->box[i].bc.rows, 0);
			prop->Attribute("Cols",			&pLayout->box[i].bc.cols, 0);
			prop->Attribute("CcType",		&pLayout->box[i].bc.ccType, 0);
			prop->Attribute("SegPerRow",	&pLayout->box[i].bc.segPerRow, 0);
			prop->Attribute("Hex",			&pLayout->box[i].bc.hex, false);

			prop->Attribute("Font",			pLayout->box[i].bc.font, SIZEOF(pLayout->box[i].bc.font), "");
			prop->Attribute("FntSize",		&pLayout->box[i].bc.fontSize, 10);
			prop->Attribute("FntAbove",		&pLayout->box[i].bc.fontAbove, false);
			prop->Attribute("FntDist",		&pLayout->box[i].bc.fontDist, 50);
			break;
		}
	}
	pLayout->boxCnt = i;
	return 0;
}

//--- rip_load_files -------------------------------
int   rip_load_files(void* doc, char *dstFolder, char *filename)
{
	TiXmlDocument *pDoc = (TiXmlDocument*) doc;
	if (pDoc)
	{
		CRX_XmlElement *files = (CRX_XmlElement *)pDoc->FirstChild("Files");
		CRX_XmlElement *file;

		if (files!=NULL)
		{
			char name[MAX_PATH];
			for( file = (CRX_XmlElement *)files->FirstChild(); file; file = (CRX_XmlElement *)file->NextSibling() )
			{
				file->Attribute("Name", name, SIZEOF(name), "not found");
				if (filename==NULL || !strcmp(name, filename))
					file->GetBinFileCompressed(dstFolder);
			}
		}
	}

	return 0;
}

#ifdef WIN32 // CopyFile not defined in linux
//				linux implementation: http://stackoverflow.com/questions/7463689/most-efficient-way-to-copy-a-file-in-linux
//--- _copy_files --------------------------------------------
static void _copy_files(char *srcDir, char *filter, char *dstDir, char *srcName, char *dstName)
{
	char fname[MAX_PATH];
	char src[MAX_PATH];
	char dst[MAX_PATH];
	int pos;
	SEARCH_Handle hsearch;
	UINT32 isDir;

	hsearch = rx_search_open(srcDir, filter);
	while (rx_search_next(hsearch, fname, sizeof(fname), NULL, NULL, &isDir))
	{
		pos=str_start(fname, srcName);
		if (pos>0 && (fname[pos]=='.' || fname[pos]=='_'))
		{			
			sprintf(src, "%s%s", srcDir, fname);
			sprintf(dst, "%s%s%s", dstDir, dstName, &fname[pos]);

			if (stricmp(src, dst)) CopyFile(src, dst, FALSE); // only for WIN32!
		}
	}
	rx_search_close(hsearch);
}

//--- _copy_label_files ---------------------------
static void _copy_label_files(char *srcPath, char *dstDir, char *name)
{
	if (*srcPath)
	{
		char srcDir[MAX_PATH];
		char srcName[MAX_PATH];
		char ext[MAX_PATH];
		split_path(srcPath, srcDir, srcName, ext);
		_copy_files(srcDir, "*.bmp", dstDir, srcName, name);
		_copy_files(srcDir, "*.tif", dstDir, srcName, name);
		sprintf(srcPath, "%s%s", name, ext);
	}
}

//--- rip_copy_files ------------------------------------------
int rip_copy_files (char *path, SLayoutDef *pLayout)
{
	char dir[MAX_PATH];
	char name[MAX_PATH];
	char ext[MAX_PATH];

	//--- create directory --------------------------------
	split_path(path, dir, name, ext);
	if (strlen(dir)<strlen(name) || _strnicmp(&dir[strlen(dir)-strlen(name)-1], name, strlen(name)))
	{
		sprintf(&dir[strlen(dir)], name);
		rx_mkdir(dir);
		strcpy(&dir[strlen(dir)], "\\");
		sprintf(path, "%s%s%s", dir, name, ext);
	}

	//--- copy image files ---------------------
	_copy_label_files(pLayout->label, dir, name);
	strcat(name, "-Color");
	_copy_label_files(pLayout->colorLayer, dir, name);

	//--- copy font files ----
	{
		int i, n;
		char *font;
		char  fontlist[MAX_BOXES][32];
		char  fontPath[MAX_PATH];
		char  dstPath[MAX_PATH];
		memset(fontlist, 0, sizeof(fontlist));
		strcpy(dstPath, dir);
		for (i=0; i<pLayout->boxCnt; i++)
		{
			if (pLayout->box[i].hdr.boxType==BOX_TYPE_BARCODE) 	 font = pLayout->box[i].bc.font;
			else if (pLayout->box[i].hdr.boxType==BOX_TYPE_TEXT) font = pLayout->box[i].text.font;
			else *font=0;
			if (*font)
			{
				for (n=0; *fontlist[n] && strcmp(font, fontlist[n]); n++)
				{
				}
				if (*fontlist[n]==0)
				{	
					strcpy(fontlist[n], font);
					strcpy(fontPath, ft_get_font_file_path(font));
					split_path(fontPath, NULL, &dstPath[strlen(dir)], NULL);

					CopyFile(fontPath, dstPath, FALSE);
				}
			}
		}
	}
	return REPLY_OK;
}
#endif

//--- rip_save_layout ----------------------------------
int rip_save_layout(void *doc, char *tempPath, SLayoutDef *pLayout)
{
//	if (pLayout->size!=sizeof(SLayout)) return 1;

	int i;
	char str[MAX_PATH];

	CRX_XmlDocument	*pDoc = (CRX_XmlDocument*)doc;
	CRX_XmlElement	*prop;
	
	//--- write file definition --------------------------
	CRX_XmlElement *layout = pDoc->InsertBefore("Files", "Layout");
	split_path(pLayout->label, NULL, str, NULL);

	layout->SetAttribute("Label",		str);
	layout->SetAttribute("Width",		pLayout->width);
	layout->SetAttribute("Height",		pLayout->height);
	layout->SetAttribute("WebWidth",	pLayout->webWidth);
	layout->SetAttribute("Columns",		pLayout->columns);
	layout->SetAttribute("ColumnDist",	pLayout->columnDist);

	split_path(pLayout->colorLayer, NULL, str, NULL);
	layout->SetAttribute("ColorLayer",		str);

	//--- write boxes ------------------------------------
	for (i=0; i<pLayout->boxCnt; i++)
	{
		snprintf(str, SIZEOF(str), "Box%02d", i);
		CRX_XmlElement *box = new CRX_XmlElement(str);
		box->SetAttribute("Type",		pLayout->box[i].hdr.boxType);
		box->SetAttribute("X",			pLayout->box[i].hdr.x);
		box->SetAttribute("Y",			pLayout->box[i].hdr.y);
		box->SetAttribute("Width",		pLayout->box[i].hdr.width);
		box->SetAttribute("Height",		pLayout->box[i].hdr.height);
		box->SetAttribute("Orientation",pLayout->box[i].hdr.orientation);
		box->SetAttribute("Mirror",		pLayout->box[i].hdr.mirror);
		box->SetAttribute("Content",	pLayout->box[i].hdr.content);
		switch(pLayout->box[i].hdr.boxType)
		{
		case BOX_TYPE_TEXT:
			prop = new CRX_XmlElement("Text");
			prop->SetAttribute("Font",		pLayout->box[i].text.font);
			prop->SetAttribute("Size",		pLayout->box[i].text.size);
			prop->SetAttribute("LineSpace",	pLayout->box[i].text.lineSpace);
			prop->SetAttribute("Alignment",	pLayout->box[i].text.alignment);
			box->LinkEndChild(prop);
			break;

		case BOX_TYPE_BARCODE:
			prop = new CRX_XmlElement("Barcode");
			prop->SetAttribute("BcType",		pLayout->box[i].bc.bcType);
			for (int n=0; n<8; n++)
			{
				snprintf(str, SIZEOF(str), "Bar%02d", n);
				prop->SetAttribute(str,		pLayout->box[i].bc.bar[n]);
			}
			for (int n=0; n<8; n++)
			{
				snprintf(str, SIZEOF(str), "Space%02d", n);
				prop->SetAttribute(str,		pLayout->box[i].bc.space[n]);
			}
			prop->SetAttribute("Stretch",	pLayout->box[i].bc.stretch);
			prop->SetAttribute("Filter",	pLayout->box[i].bc.filter);
			prop->SetAttribute("CodePage",	pLayout->box[i].bc.codePage);
			prop->SetAttribute("Check",		pLayout->box[i].bc.check);
			prop->SetAttribute("Size",		pLayout->box[i].bc.size);
			prop->SetAttribute("Format",	pLayout->box[i].bc.format);
			prop->SetAttribute("Mask",		pLayout->box[i].bc.mask);
			prop->SetAttribute("Rows",		pLayout->box[i].bc.rows);
			prop->SetAttribute("Cols",		pLayout->box[i].bc.cols);
			prop->SetAttribute("CcType",	pLayout->box[i].bc.ccType);
			prop->SetAttribute("SegPerRow",	pLayout->box[i].bc.segPerRow);
			prop->SetAttribute("Hex",		pLayout->box[i].bc.hex);

			prop->SetAttribute("Font",		pLayout->box[i].bc.font);
			prop->SetAttribute("FntSize",	pLayout->box[i].bc.fontSize);
			prop->SetAttribute("FntAbove",	pLayout->box[i].bc.fontAbove);
			prop->SetAttribute("FntDist",	pLayout->box[i].bc.fontDist);
			box->LinkEndChild(prop);
			break;
		}

		layout->LinkEndChild(box);
	}
		
	//--- add file -------------------------------
	/*
	{
		CRX_XmlElement *files = pDoc->GetEndChild("Files");
		char *font;
		char  fontlist[MAX_BOXES][32];
		int   n;
						
		add_root(str, pLayout->label, tempPath);
		files->SetBinFileCompressed(str);

		add_root(str, pLayout->colorLayer, tempPath);
		files->SetBinFileCompressed(str);

		memset(fontlist, 0, sizeof(fontlist));

		for (i=0; i<pLayout->boxCnt; i++)
		{
			if (pLayout->box[i].hdr.boxType==BOX_TYPE_BARCODE) 	 font = pLayout->box[i].bc.font;
			else if (pLayout->box[i].hdr.boxType==BOX_TYPE_TEXT) font = pLayout->box[i].text.font;
			else *font=0;
			if (*font)
			{
				for (n=0; *fontlist[n] && strcmp(font, fontlist[n]); n++)
				{
				}
				if (*fontlist[n]==0)
				{	
					strcpy(fontlist[n], font);
					files->SetBinFileCompressed(ft_get_font_file_path(font));
				}
			}
		}
	}
	*/

	return 0;	
}

//--- _set_box_hdr -------------------------
static void _set_box_hdr(SBoxHeader *hdr, SBoxHeaderDef *pHdr, int type, double factX, double factY)
{
	hdr->boxType	= type;
	hdr->mirror		= pHdr->mirror;
	hdr->rect.left	= (ULONG) (factX*pHdr->x);
	hdr->rect.right	= (ULONG) (factX*(pHdr->x+pHdr->width));
	hdr->rect.top	= (ULONG) (factY*pHdr->y);
	hdr->rect.bottom= (ULONG) (factY*(pHdr->y+pHdr->height));
}

//--- rip_set_layout ----------------------------------
int rip_set_layout(SLayoutDef *pLayout, SRipBmpInfo *pInfo)
{
	int			size=4096; // size of buffer
	int			b, i;
	BYTE		*buf;
	SMsgLayout	*layout;
	STextBox	*txtBox;
	SBarcodeBox	*bcBox;
//	SBitmapBox	*bmpBox;
	int			dx, fh;
	int			dy;
	int			orientation;
	int			dpiX, dpiY;
//	int			dotSizes=1;
	int			monoDropSize = 3;
	int			bmpLength, bmpHeight;
	int			resH;
	double		factX, factY;
	SLayoutBoxDef	*pBox;
	int			Height;

	dpiX = DPIX; //Inkjet.cfg.l_seriesRes;
	dpiY = DPIY;

	//--- ------------------------------------------------------------------------- 
	bmpLength = (int)(dpiX * pLayout->width  / 25400.0);
	bmpHeight = (int)(dpiY * pLayout->height / 25400.0);
	bmp_create(&_Bmp, bmpLength, bmpHeight, 1);
	bmp_create(&_Bmp, bmpLength, bmpHeight, pInfo->bitsPerPixel);
	if (_Bmp.buffer) memset(_Bmp.buffer, 0, _Bmp.sizeUsed);
	rip_end();
	rip_init(dpiX, dpiY, pInfo->bitsPerPixel, monoDropSize);
	ft_init(DPIX, DPIX, monoDropSize);

	//------------------------------------------------------------------------
	buf = (BYTE*)malloc(size);
	layout = (SMsgLayout*)buf;
	memset(layout, 0, size);
	layout->cmd			= CMD_SET_LAYOUT;
	layout->cmdLen		= sizeof(SMsgLayout)-sizeof(layout->data);
	layout->layoutNo	= 0;

	factX = dpiX/25400.0;
	factY = dpiY/25400.0;

	Height = 0;

	//--- text boxes ---
	for (b=0; b<pLayout->boxCnt; b++)
	{
		pBox = &pLayout->box[b];

		i = pBox->hdr.y+pBox->hdr.height;
		if (i>Height) Height=i;

		if (pBox->hdr.boxType==BOX_TYPE_TEXT)
		{
//			TrPrintf(1, "Box[%d]: Text\n", b);

			txtBox = (STextBox*) &buf[layout->cmdLen];
			layout->cmdLen += sizeof(STextBox);
			if ((int)layout->cmdLen>size) Error(ERR_CONT, 0, "Memory overflow");

			layout->boxCnt++;
			memset(txtBox, 0, sizeof(STextBox));
			_set_box_hdr(&txtBox->hdr, &pBox->hdr, BOX_TYPE_TEXT, factX, factY); 

			//--- fill ---
			txtBox->alignment = pBox->text.alignment;
			dx = 0;
			/*
			// block commented by samuel de santis on March 2018.
			// The following code appears to be wrong.
			// As a consequence 'dx' is now always equal to zero, we should remove its use.
			switch(pBox->text.alignment)
			{
			case TA_CENTER:		dx = 1; txtBox->alignment = TA_CENTER;	break;
			case TA_RIGHT:		dx = 2; txtBox->alignment = TA_RIGHT;	break;
			case TA_LEFT:		dx = 0;	txtBox->alignment = TA_LEFT;	break;
			case TA_NEW_LINE:	dx = 0;	txtBox->alignment = TA_NEW_LINE;break;
			default:
				Error(ERR_CONT, 0, "Box[%d]: Alignment %d not implemented", b+1, pBox->text.alignment);
			}
			*/
			switch(pBox->hdr.orientation)
			{
			case 900:	orientation		= 900;
						resH			= dpiX;
						break;

			case 1800:	orientation		= 1800;
						resH			= dpiY;
						break;

			case 2700:	orientation		= 2700;
						resH			= dpiX;
						break;

			default:	orientation		= 0;
						resH			= dpiY;
			}
			
			// fh = pBox->text.size*resH/72;

			fh = -pBox->text.size*64;

			txtBox->extraSpace				= 0; //pBox->fontSpace;
			txtBox->font.lfHeight			= -fh;
			txtBox->font.lfWidth			= 0; //pBox->fontWidth;
			txtBox->font.lfEscapement		= orientation;
			txtBox->font.lfOrientation		= orientation;
			txtBox->font.lfWeight			= FALSE; //(pBox->fontBold)?FW_BOLD:FW_NORMAL;
			txtBox->font.lfItalic			= FALSE; // pBox->fontItalic;
			txtBox->font.lfUnderline		= FALSE;
			txtBox->font.lfStrikeOut		= FALSE;
			txtBox->font.lfCharSet			= DEFAULT_CHARSET;
			txtBox->font.lfOutPrecision		= OUT_DEFAULT_PRECIS;
			txtBox->font.lfClipPrecision	= CLIP_DEFAULT_PRECIS;
			txtBox->font.lfQuality			= DEFAULT_QUALITY;
			txtBox->font.lfPitchAndFamily	= DEFAULT_PITCH | FF_DONTCARE;
			strncpy(txtBox->font.lfFaceName, pBox->text.font, sizeof(txtBox->font.lfFaceName));

			dy = 0;//txtMetrics.tmExternalLeading;

			switch(pBox->hdr.orientation)
			{
			case 900:	txtBox->pos.x	= (ULONG) (factX*(pBox->hdr.x)-dy);
						txtBox->pos.y	= (ULONG) (factY*(pBox->hdr.y+pBox->hdr.height-pBox->hdr.height*dx/2));
						txtBox->dx		= (USHORT)(factX*(pBox->text.lineSpace));
						txtBox->dy		= 0;
						break;

			case 1800:	txtBox->pos.x	= (ULONG) (factX*(pBox->hdr.x+pBox->hdr.width-pBox->hdr.width*dx/2));
						txtBox->pos.y	= (ULONG) (factY*(pBox->hdr.y+pBox->hdr.height)+dy);
						txtBox->dx		= 0;
						txtBox->dy		= -(float)(factY*(pBox->text.lineSpace));
						break;

			case 2700:	txtBox->pos.x	= (ULONG) (factX*(pBox->hdr.x+pBox->hdr.width)+dy);
						txtBox->pos.y	= (ULONG) (factY*(pBox->hdr.y+pBox->hdr.width*dx/2));
						txtBox->dx		= -(float)(factX*(pBox->text.lineSpace));
						txtBox->dy		= 0;
						break;

			default:	txtBox->pos.x	= (ULONG) (factX*(pBox->hdr.x+pBox->hdr.width*dx/2));
						txtBox->pos.y	= (ULONG) (factY*(pBox->hdr.y)-dy);
						txtBox->dx		= 0;
						txtBox->dy		= (float) (factY*(pBox->text.lineSpace));
			}

		}
		else if (pBox->hdr.boxType==BOX_TYPE_BARCODE)
		{
			bcBox = (SBarcodeBox*) &buf[layout->cmdLen];
			layout->cmdLen += sizeof(SBarcodeBox);
			if ((int)layout->cmdLen>size) 
			{
				//error
				int i=0;
			}

			layout->boxCnt++;
			memset(bcBox, 0, sizeof(SBarcodeBox));
			_set_box_hdr(&bcBox->hdr, &pBox->hdr, BOX_TYPE_BARCODE, factX, factY); 

			//--- fill the barcode ---------------------			
			switch (pBox->hdr.orientation)
			{
			case 900:	bcBox->orientation = deg90; break;
			case 1800:	bcBox->orientation = deg180; break;
			case 2700:	bcBox->orientation = deg270; break;
			default:bcBox->orientation	   = deg0; break;
			}

			for (i=0; i<SIZEOF(bcBox->bar); i++)
			{
				bcBox->bar[i]	= pBox->bc.bar[i];
				bcBox->space[i] = pBox->bc.space[i];
			}

			bcBox->bcType	= pBox->bc.bcType;
			bcBox->stretch	= pBox->bc.stretch;
			bcBox->filter	= pBox->bc.filter;
			bcBox->codePage	= pBox->bc.codePage;
			bcBox->rows		= pBox->bc.rows;
			bcBox->cols		= pBox->bc.cols;
			if(pBox->bc.bcType==eBC_QRCode || pBox->bc.bcType==eBC_Aztec) bcBox->check = pBox->bc.check; 
			else	bcBox->check = pBox->bc.check;
			bcBox->format	= pBox->bc.format;
			bcBox->size		= pBox->bc.size;
			bcBox->mask		= pBox->bc.mask;
			bcBox->ccType	= pBox->bc.ccType;
			bcBox->segPerRow= pBox->bc.segPerRow;
			bcBox->wchar	= !pBox->bc.hex;

			if (*pBox->bc.font)
			{
				bcBox->above	= pBox->bc.fontAbove;
				if (pBox->hdr.orientation==900 || pBox->hdr.orientation==2700)
				{
					bcBox->font.lfHeight	= pBox->bc.fontSize*64;
					bcBox->dist				= pBox->bc.fontDist*100*dpiX/dpiY;
				}
				else 
				{
//					bcBox->font.lfHeight	= -pBox->bc.fontSize*dpiY/112;
					bcBox->font.lfHeight	= pBox->bc.fontSize*64;
					bcBox->dist				= pBox->bc.fontDist*100;
				}
				bcBox->font.lfEscapement		= bcBox->orientation;
				bcBox->font.lfWidth				= 0; // pBox->fontWidth;
				bcBox->font.lfOrientation		= pBox->hdr.orientation;
				bcBox->font.lfWeight			= FW_NORMAL;
				bcBox->font.lfItalic			= FALSE;
				bcBox->font.lfUnderline			= FALSE;
				bcBox->font.lfStrikeOut			= FALSE;
				bcBox->font.lfCharSet			= DEFAULT_CHARSET;
				bcBox->font.lfOutPrecision		= OUT_DEFAULT_PRECIS;
				bcBox->font.lfClipPrecision		= CLIP_DEFAULT_PRECIS;
				bcBox->font.lfQuality			= DEFAULT_QUALITY;
				bcBox->font.lfPitchAndFamily	= DEFAULT_PITCH | FF_DONTCARE;
				strncpy(bcBox->font.lfFaceName, pBox->bc.font, sizeof(bcBox->font.lfFaceName));
			}
		}
		/*
		else if (pBox->boxType==MIC_LAY_FIX_BMP_BOX || pBox->boxType==MIC_LAY_VAR_BMP_BOX)
		{
			TrPrintf(1, "Box[%d]: Bitmap\n", b);

			bmpBox = (SBitmapBox*) &buf[layout->cmdLen];
			layout->cmdLen += sizeof(SBitmapBox);
			if ((int)layout->cmdLen>size) error(ERR_OP, 10, "%s", "Memory overflow");

			layout->boxCnt++;
			memset(bmpBox, 0, sizeof(SBitmapBox));
			set_box_hdr(&bmpBox->hdr, pBox, BOX_TYPE_BITMAP, factX, factY); 
			//--- fill the bimtp box ---
		}
		*/
		else
		{
			Error(ERR_CONT, 0, "Box[%d].type=%d not supported", b, pBox->hdr.boxType);
		}
	}
//	if (Height>IjLayout[0].printHeight) Height=IjLayout[0].printHeight;
//	*len = layout->cmdLen;

	//---------------------------------------------------------------------------
	rip_set_layout(layout);
	free(buf);
	return 0;
}

//--- rip_layout_is_equal ------------------------------------------------------
int rip_layout_is_equal(SLayoutDef *pLayout1, SLayoutDef *pLayout2)
{
	return (memcmp(pLayout1, pLayout2, sizeof(SLayoutDef))==0);
}

//--- rip_bc_states -------------------------------------------------------------------
int  rip_bc_states(int bcType)
{
	return bc_states((e_BarCType)bcType);
}

//--- rip_get_font_name ---------------------------------
int  rip_get_font_name(int nr, char *fontname)
{
	return ft_get_font_name(nr, fontname); 
}

//--- rip_data --------------------------------------------------------------------------
int rip_data(SLayoutDef *pLayout, int x, int y, void *pbmp, void *prxBmpLabel, void *prxBmpColor, int black)
{
	int				size=4096; // size of buffer
	BYTE			*buf;
	SMsgPrintData	*msg;
	int				b;
	USHORT			*pLineCnt;
	USHORT			*pLineLen;
	UTF16			*read, *write;
	UTF16			text[1024];
	UTF16			wText[256];

	buf = (BYTE*)malloc(size);
	memset(buf, 0, size);
	msg = (SMsgPrintData*)buf;
	msg->cmd		= CMD_PRINT_DATA;
	msg->cmdLen		= sizeof(SMsgPrintData)-sizeof(msg->data);
	msg->dataId		= 1;
	msg->layoutNo	= 0;
	msg->boxCnt		= 0;

	/*
	if (pLayout->box[0].lineCnt)
	{
		wcstombs(msg->filepath, (USHORT*)label->box[0].line[0].text, sizeof(msg->filepath));
		msg->page = 1;
		if (label->box[0].lineCnt>1) msg->page = _wtoi((USHORT*)label->box[0].line[1].text);
		if (msg->page<1) msg->page=1;
		TrPrintf(0, "Background >>%s<<, page %d, linecnt=%d\n", msg->filepath, msg->page, label->box[0].lineCnt);
	}
	*/

	for (b=0; b<pLayout->boxCnt; b++)
	{
		rx_mbstowcs(wText, pLayout->box[b].hdr.content, SIZEOF(wText));

		dat_interprete_data_fields(text, wText, SIZEOF(text)-1);

		msg->boxCnt++;
		pLineCnt = (USHORT*)&buf[msg->cmdLen];
		msg->cmdLen += sizeof(USHORT);

		//	TrPrintf(1, "Box[%d].lineCnt=%d\n", b, label->box[b].lineCnt);
		/*
		if (IjLayout[0].box[b].boxType==MIC_LAY_FIX_BMP_BOX)
		{
			l  = swprintf(path, L"d:\\bitmaps\\");
			char_to_wchar(&path[l], IjLayout[0].box[b].fontString, SIZEOF(path)-l);
			l = 2+2*wcslen(path); 
			TrPrintfW(1, L"Fix Bitmap >>%s<<\n", path);
			memcpy(&SendBuf[msg->cmdLen], &l, sizeof(USHORT));	msg->cmdLen += sizeof(USHORT);
			memcpy(&SendBuf[msg->cmdLen], path, l);				msg->cmdLen += l;
		}
		else if (IjLayout[0].box[b].boxType==MIC_LAY_VAR_BMP_BOX)
		{
			l  = swprintf(path, L"d:\\bitmaps\\");
			char_to_wchar(&path[l], IjLayout[0].box[b].fontString, SIZEOF(path)-l);
			if ((wchar_t*)label->box[b].line[0].text[0]) 
			{
				swprintf(&path[wcslen(path)-4], L"-%s.bmp", (wchar_t*)label->box[b].line[0].text);
				l = 2+2*wcslen(path);
			}
			else 
			{
				path[0]=0;
				l=2;
			}
			TrPrintfW(1, L"Var Bitmap >>%s<<\n", path);
			memcpy(&SendBuf[msg->cmdLen], &l, sizeof(USHORT));	msg->cmdLen += sizeof(USHORT);
			memcpy(&SendBuf[msg->cmdLen], path, l);				msg->cmdLen += l;
		}
		else
		*/
	//	if (pLayout->box[b].hdr.boxType==BOX_TYPE_TEXT)
		{
			pLineLen = (USHORT*)&buf[msg->cmdLen];
			msg->cmdLen += sizeof(USHORT);
			write = (UTF16*)&buf[msg->cmdLen];
//			for (read  = (UTF16*)pLayout->box[b].hdr.content; *read; read++)
			for (read = text; *read; read++)
			{
				if (*read==(UTF16)0x0d)
				{
					*write++ = 0;
					msg->cmdLen += (*pLineLen);
					(*pLineCnt)++;
					pLineLen = (USHORT*)&buf[msg->cmdLen];
					msg->cmdLen += sizeof(USHORT);						
				}
				else if (*read>=(UTF16)' ') 
				{
					*write++ = *read;
					(*pLineLen) += sizeof(UTF16);
				}
			}
			msg->cmdLen += (*pLineLen);
			(*pLineCnt)++;
		}
	}
	
	if (pbmp==NULL) rip_data(x, y, msg, &_Bmp,             (RX_Bitmap*)prxBmpLabel, (RX_Bitmap*)prxBmpColor, black);
	else            rip_data(x, y, msg, (RX_Bitmap*)pbmp,  (RX_Bitmap*)prxBmpLabel, (RX_Bitmap*)prxBmpColor, black);
	free(buf);
	return 0;
}

//--- rip_get_bitmap ----------------------------------------------------------------------
int  rip_get_bitmap(int *width, int *height, int *bitsperpixel, int **data, int *datasize, int *stride)
{
	if (_Bmp.buffer)
	{
		*width			= _Bmp.width;
		*height			= _Bmp.height;
		*bitsperpixel	= _Bmp.bppx;
		*stride			= _Bmp.lineLen;
		*datasize		= _Bmp.sizeUsed;
		*data			= (int*)_Bmp.buffer;
		return 0;
	}
	else return 1;
}

//--- rip_get_print_env ---------------------------------------------
int rip_get_print_env(const char *path, char *printEnv)
{
	char dir[MAX_PATH];
	char name[MAX_PATH];
	char ext[16];
	int len, l;

	*printEnv = 0;

	split_path(path, dir, name, ext);
	len = (int)strlen(dir);
	l   = (int)strlen(name);
	if (len)
	{
		dir[--len]=0;
		if (len>l && !_stricmp(&dir[len-l], name))
		{
			len -= l;
			dir[--len]=0;
		}
		for (l=len; l;l--)
		{
			if (dir[l]=='\\')
			{
				strcpy(printEnv, &dir[l+1]);
				return 0;
			}
		}
	}
	return 1;
}


//--- rip_bmp_copy ---------------------------------------------------------------------------------
int rip_bmp_copy(int *srcBmp, int srcWidth, int srcHeight, int srcStride, int srcBppx, int dist, int cnt, int *dstBmp, int dstStride)
{
	int h, i;
	int width, fill;
	BYTE *src = (BYTE*)srcBmp;
	BYTE *dst = (BYTE*)dstBmp;
	BYTE *s, *d;

	width = (srcWidth*srcBppx+7)/8;
	for (i=0; i<cnt; i++)
	{
		s=src;
		d=dst+(i*dist*srcBppx)/8;
		if (i==cnt-1) fill=0;
		else fill=((i+1)*dist*srcBppx)/8 - (i*dist*srcBppx)/8 - width;
		for (h=0; h<srcHeight; h++)
		{
			memcpy(d, s, width);
			if (fill) memset(d+width, 0xff, fill);
			s+=srcStride;
			d+=dstStride;
		}
	}
	return 0;
}
