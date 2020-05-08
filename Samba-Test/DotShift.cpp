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

#pragma once

#include "stdafx.h"
#include "Samba_def.h"
#include "Bmp.h"


//--- load_bmp -----------------------------------------------------------------------------------
// Summary: allocates a new buffer and loads the bitmap data and info
// parameters:
// path:	path of the bitmap file
// info:	reference to bitmap info structure, will be filled with the info of this bitmap
// **data:	reference to data pointer
static int load_bmp(const wchar_t *path, SBmpInfo *info, BYTE **data)
{
	CImage		image;
	int			ret;

	ret = image.Load(path);
	if (ret == 0)
	{
		info->width = image.GetWidth();
		info->height = image.GetHeight();
		info->bitsPerPixel = image.GetBPP();
		info->lineLen = (((info->width) + 15) & ~15)  * (info->bitsPerPixel) / 8;
		info->dataSize = (info->lineLen) * (info->height);
		ret = image.GetPitch();

		info->inverse = FALSE;
		if (info->bitsPerPixel==1)
		{
			RGBQUAD color;
			image.GetColorTable(0, 1, &color);
			if (color.rgbRed == 0 && color.rgbGreen == 0 && color.rgbBlue == 0) info->inverse = TRUE;
		}
		(*data) = (BYTE*)malloc(info->dataSize);
		byte *pixels = (byte*)image.GetBits();
		pixels = &pixels[-(info->lineLen)*(info->height - 1)];
		memcpy(*data, pixels, info->dataSize);
		return REPLY_OK;
	}
	return REPLY_ERROR;
}

static int BMP_Read(const wchar_t *path, SBmpInfo *info, BYTE **data)
{
// write Print Memory to file
	BITMAPFILEHEADER	header;
	BITMAPINFOHEADER	bmpInfo;
	ULONG				bmiColors[16];
	FILE *file=NULL;

	_wfopen_s(&file, path, L"rb");
	if (file)
	{		
		fread(&header, 1, sizeof(header), file);
		if (memcmp(&header.bfType, &"BM", 2)) return FALSE;

		fread(&bmpInfo,		1, sizeof(bmpInfo), file);
		fread(&bmiColors,	1, header.bfOffBits - sizeof(header) - sizeof(bmpInfo), file);
		info->bitsPerPixel	= bmpInfo.biBitCount;
		info->width			= bmpInfo.biWidth;
		info->height		= bmpInfo.biHeight;
		info->dataSize		= bmpInfo.biSizeImage;
		info->lineLen		= bmpInfo.biSizeImage/bmpInfo.biHeight;
		info->inverse		= bmiColors[0]==0;
		
		*data = (BYTE*)malloc(info->dataSize);
		fread(*data, 1, info->dataSize, file);
		fclose(file);

		return TRUE;
	}
	return FALSE;
}

//--- write_hex -------------------------------------------------------------
int write_hex(HANDLE file, BYTE *data, int len)
{
	char	buf[32];
	ULONG	written;
	int		linelen;
	int		filelen=0;

	for (linelen=0;len; len--, data++)
	{
		sprintf(buf, "%02x", *data);
		WriteFile(file, buf, 2, &written, NULL); filelen += written;
		if (++linelen==32) // 256 bit
		{
			sprintf(buf, "\r\n");
			WriteFile(file, buf, 2, &written, NULL); filelen += written;
			linelen=0;
		}
	}
	return filelen;
}

//--- WriteLine ------------------------------
void WriteLine(HANDLE file, char *str)
{
	ULONG	written;
	char	buf[4];
	
	WriteFile(file, str, strlen(str), &written, NULL);
	sprintf(buf, "\r\n");
	WriteFile(file, buf, 2, &written, NULL);
}

//--- write_srd -------------------------------------------------------------
int write_srd(HANDLE file, const wchar_t *srcName, const wchar_t *strokes, int lineLen, BYTE *data, int dataSize)
{
	const wchar_t *ch;
	char	buf[256];
	char	name[256];
	ULONG	written;
	int		x, y, xx;
	UINT	fp;
	int		dotCnt;
	int		lineCnt;

	for (ch=srcName; *ch; ch++)
	{
		if (*ch=='\\') 	wcstombs(name, ch+1, sizeof(name));
	}
	strcpy(&name[strlen(name)-3], "udp");

	WriteLine(file, "Dump of: head0 shift register data");
	WriteLine(file, "==================================");
	sprintf(buf, "#fp per fp-seq : %d", wcslen(strokes)); WriteLine(file, buf);
	sprintf(buf, "greyscale-array: %c%c%c", strokes[0], strokes[1], strokes[2]);
	WriteLine(file, buf);
	sprintf(buf, "image source   : %s", name); 
	WriteLine(file, buf);
	WriteLine(file, "");
	WriteLine(file, "                       |    64    |   63    |   62    |   61    |   60    |   59    |   58    |   57    |   56    |   55    |   54    |   53    |   52    |   51    |   50    |   49    |   48    |   47    |   46    |   45    |   44    |   43    |   42    |   41    |   40    |   39    |   38    |   37    |   36    |   35    |   34    |   33    |   32    |   31    |   30    |   29    |   28    |   27    |   26    |   25    |   24    |   23    |   22    |   21    |   20    |   19    |   18    |   17    |   16    |   15    |   14    |   13    |   12    |   11    |   10    |   09    |   08    |   07    |   06    |   05    |   04    |   03    |   02    |   01    |");
	WriteLine(file, "                       |  B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  | B  | F  |");
	WriteLine(file, " y-pos |   #fp | #jets | 15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|15 0|");
	WriteLine(file, "-------+-------+-------+-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------");

	lineCnt = dataSize/(lineLen/8);

	for (y=0; y<lineCnt; y++)
	{
		for (fp=0; fp<wcslen(strokes); fp++)
		{
			if (fp==0) sprintf(buf, "% 6d  % 6d", y, fp+1);
			else       sprintf(buf, "        % 6d", fp+1);
			WriteFile(file, buf, strlen(buf), &written, NULL);
			
			dotCnt = 0;
			if (strokes[fp]==L'1')
			{
				for (x=0; x<lineLen; x++)
				{
					if (data[x/8] & 1<<(x%8)) 
						dotCnt++;
				}
			} 
			
			if (dotCnt) sprintf(buf, "  % 6d   ", dotCnt);
			else        sprintf(buf, "       -   ");
			WriteFile(file, buf, strlen(buf), &written, NULL);

			for (x=0; x<lineLen/8; x++)
			{
				switch (x & 0x03)
				{
				case 0x00:	xx = (x & ~0x03)+3; break;
				case 0x01:	xx = (x & ~0x03)+2; break;
				case 0x02:	xx = (x & ~0x03)+1; break;
				case 0x03:	xx = (x & ~0x03)+0; break;
				}

				if (strokes[fp]==L'1') sprintf(buf, "%02X", data[xx]);
				else                   sprintf(buf, "%02X", 0);
				WriteFile(file, buf, strlen(buf), &written, NULL);
				if (x%2==1) WriteFile(file, " ", 1, &written, NULL);
			}

			WriteLine(file, "");
		}
		data += (lineLen/8);
	}
	return 0;
}

//--- invert_data ---------------------
void invert_data(SBmpInfo *info, BYTE *data)
{
	int len;
	if (info->inverse)
	{
		for (len = info->lineLen*info->height; len; len--,  data++)
		{
			*data = ~*data;
		}
		info->inverse = FALSE;
	}
}

//--- convert_bppx ---------------------
void convert_bppx(SBmpInfo *info, const wchar_t *bitsPerPixel, BYTE *data1, BYTE **data2)
{
	SBmpInfo info2;
	int x, y;
	int dropsize;

	if (wcslen(bitsPerPixel)<2)
	{
		(*data2) = (BYTE*)malloc(info->dataSize);
		memcpy((*data2), data1, info->dataSize);
	}
	else
	{
		if (!wcscmp(bitsPerPixel, L"01")) dropsize=0x40;
		else if (!wcscmp(bitsPerPixel, L"10")) dropsize=0x80;
		else if (!wcscmp(bitsPerPixel, L"11")) dropsize=0xC0;
		else dropsize=0x40;

		memcpy(&info2, info, sizeof(info2));
		info2.lineLen = (info->width*2+7)/8;
		info2.dataSize = info2.lineLen*info2.height;
		info2.bitsPerPixel = 2;
		(*data2) = (BYTE*)malloc(info2.dataSize);
		memset((*data2), 0, info2.dataSize);
		if (info->bitsPerPixel==1)
		{
			for (y=0; y<info->height; y++)
			{
				for (x=0; x<info->width; x++)
				{
					if (data1[y*info->lineLen+x/8] & 0x80>>(x%8)) (*data2)[y*info2.lineLen+x/4] |= dropsize>>(2*(x%4));
				}
			}
		
		}
		memcpy(info, &info2, sizeof(info2));
	}
}

//--- shift_data_1bit -------------------------------------------------------------
// summary:	prepares the head data for one printhead.
static int shift_data_1bit(SBmpInfo *info, BYTE *srcData, BYTE **dstData, int *dstSize, int pixel0, int backwards, int rightToLeft, int version)
{
	/*
	if (info->bitsPerPixel != 1)
	{
		AfxMessageBox(L"Only 1 Bit Per Pixel bitmaps are supported!", MB_ICONERROR);
		return REPLY_ERROR;
	}
	*/

	int lineLen = NOZZLE_CNT / 8;
	int size = lineLen*(info->height + NOZZLE_MAX_Y);
	BYTE *data = (BYTE*)malloc(size);
	memset(data, 0, size);

	int width;
	int w, h;
	int pos;
	int dots;
	BYTE *src;
	SLineMapping *pmap;
	
	if (version==2)	Nozzles_initLineMapping_V2(backwards, rightToLeft);
	else			Nozzles_initLineMapping(backwards, rightToLeft);

	if (info->width > LineCnt) width = LineCnt;
	else width = info->width;

	/*
	if (info->inverse)
	{
		dots=0;
		for (h=0; h<info->height; h++)
		{
			if (FALSE && backwards) src = &srcData[h*info->lineLen];
			else           src = &srcData[(info->height-h-1)*info->lineLen];
			for (w=0; w<width; w++)
			{
				if (!(src[w/8] & 0x80>>(w&7)))
				{ 
					pmap = &LineMapping[w+pixel0];
					pos = lineLen*(pmap->delay+h) + pmap->byteOffset; 
					if (pos>=0 && pos<size)
						data[pos] |= pmap->bitSet;
					dots++;
				}
			}
		}
		*dstData = data;
		*dstSize = size;
		return REPLY_OK;
	}
	else
	*/
	{
		dots=0;
		for (h=0; h<info->height; h++)
		{
			if (FALSE && backwards) src = &srcData[h*info->lineLen];
			else           src = &srcData[(info->height-h-1)*info->lineLen];
			for (w=0; w<width; w++)
			{
				if (src[w/8] & 0x80>>(w&7))
				{ 
					pmap = &LineMapping[w+pixel0];
					pos =lineLen*(pmap->delay+h) + pmap->byteOffset; 
					if (pos>=0 && pos<size)
						data[pos] |= pmap->bitSet;
					dots++;
				}
			}
		}
		*dstData = data;
		*dstSize = size;
		return REPLY_OK;
	}

	return REPLY_ERROR;
}

//--- shift_data_1bit_reverse -------------------------------------------------------------
// summary:	prepares the head data for one printhead.
int shift_data_1bit_reverse(SBmpInfo *info, BYTE *srcData, BYTE *dstData, int size, int version)
{
	memset(dstData, 0, size);

	int i, y, xx, yy;
	int cnt=0, cnt1=0;
	int srcLineLen = NOZZLE_CNT/8;
	BYTE *src;

	memset(info, 0, sizeof(*info));
	info->bitsPerPixel = 1;
	info->width		= NOZZLE_CNT+NOZZLE_OVERLAP;
	info->lineLen	= info->width/8;
	info->height	= size/srcLineLen;
	size = info->height*info->lineLen;

	if (version==2)	Nozzles_initLineMapping_reverse_V2();	
	else			Nozzles_initLineMapping_reverse();

	for (y=0; y<info->height; y++)
	{
		src = &srcData[y*srcLineLen];
		for (i=0; i<SIZEOF(ReverseLineMapping); i++)
		{
			if (src[i/8] & (0x01<<(i&7))) 
			{
				xx = ReverseLineMapping[i].x;
				yy = y-ReverseLineMapping[i].y;
				yy=yy*info->lineLen+xx/8;
				if (yy<size && yy>=0) 
					dstData[yy] |= 0x80>>(xx&7); 
			}
		}	
	}
	info->height	-= NOZZLE_MAX_Y;
	return REPLY_OK;
}

//--- shift_bmp -------------------------------------------------------------------
int shift_bmp(const wchar_t *in_path, const wchar_t *out_path, const wchar_t *dotGeneration, const wchar_t *bitsPerPixel, int pixel0, int backwards, int rightToLeft, int version)
{
	SBmpInfo info;
	BYTE *data=NULL, *dstData;
	int dstSize;
	ULONG written;
	wchar_t path[MAX_PATH];
	wchar_t p[MAX_PATH];

	wsprintf(path, L"%s%s.%s%s", out_path, dotGeneration, backwards?"b":"f", rightToLeft?"r":"l");

	BMP_Read(in_path, &info, &data);
	invert_data(&info, data);
//	convert_bppx(&info, bitsPerPixel, data1, &data);
	if (shift_data_1bit(&info, data, &dstData, &dstSize, pixel0, backwards, rightToLeft, version) == REPLY_OK)
	{
		HANDLE hex_file;
		wsprintf(p, L"%s.hex", path);
		hex_file = CreateFile(p, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if (hex_file != INVALID_HANDLE_VALUE)
		{
			write_srd(hex_file, in_path, dotGeneration, 2048, dstData, dstSize);
			CloseHandle(hex_file);
		}

		HANDLE file;
		wsprintf(p, L"%s.srd", path);
		file = CreateFile(p, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if (file != INVALID_HANDLE_VALUE)
		{
			WriteFile(file, dstData, dstSize, &written, NULL);
			CloseHandle(file);
		}

		free(dstData);
	}
	free(data);
	return REPLY_OK;
}

//--- shift_bmp_reverse -------------------------------------------------------------------
int shift_bmp_reverse_srd(const wchar_t *in_path, const wchar_t *out_path, int version)
{
	SBmpInfo info;
	BYTE *srcData, *dstData;
	ULONG srcSize, read;
	HANDLE file;

	file = CreateFile(in_path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (file!=INVALID_HANDLE_VALUE)
	{
		srcSize = GetFileSize(file, NULL);
		srcData = (BYTE*)malloc(srcSize);
		dstData	= (BYTE*)malloc(2*srcSize);
		memset(srcData, 0, srcSize);
		memset(dstData, 0, 2*srcSize);
		ReadFile(file, srcData, srcSize, &read, NULL);
		CloseHandle(file);

		if (shift_data_1bit_reverse(&info, srcData, dstData, read, version) == REPLY_OK)
		{
			BMP_write(out_path, dstData, info.bitsPerPixel, info.width, info.height);
			wchar_t msg[256];
			wsprintf(msg, L"Converted file >>%s>> created", out_path);
			AfxMessageBox(msg, MB_OK);
		}
		free(dstData);
		free(srcData);
	}

	return REPLY_OK;
}

//--- read_hex_file --------------------------------
static void read_hex_file(FILE *file, BYTE *buf, int bufSize, ULONG *pread)
{
	char str[1024];
	int line=0;
	int pos, i;
//	UINT32 *pdata;
//	UCHAR   dat[4], dat1[4];
	UINT32	data;
	
	*pread = 0;
	memset(buf, 0, bufSize);
	while(true)
	{
		if (!fgets(str, sizeof(str), file)) break;
		line++;
		if (line>10)
		{
			pos=25;
			for (i=0;i<64;i++)
			{
				sscanf(&str[pos], "%04x", &data);
				buf[*pread+4*i+3] = (data>>8)&0xff;
				buf[*pread+4*i+2] = data&0xff;
				pos+=5;
				sscanf(&str[pos], "%04x", &data);
				buf[*pread+4*i+1] = (data>>8)&0xff;
				buf[*pread+4*i+0] = data&0xff;
				pos+=5;
			}
			(*pread) += 256;
		}
	}
}

int shift_bmp_reverse(const wchar_t *in_path, const wchar_t *out_path, int version)
{
	SBmpInfo info;
	BYTE *srcData, *dstData;
	ULONG srcSize, read;
	fpos_t size;
	FILE  *file;

	file = _wfopen(in_path, L"rt");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		fgetpos(file, &size);
		fseek(file, 0, SEEK_SET);
		srcSize = (ULONG)size;
		srcData = (BYTE*)malloc(srcSize);
		dstData	= (BYTE*)malloc(2*srcSize);
		memset(dstData, 0, 2*(int)size);
		read_hex_file(file, srcData, srcSize, &read);
		fclose(file);

		if (shift_data_1bit_reverse(&info, srcData, dstData, read, version) == REPLY_OK)
		{
			BMP_write(out_path, dstData, info.bitsPerPixel, info.width, info.height);
			wchar_t msg[256];
			wsprintf(msg, L"Converted file >>%s>> created", out_path);
			AfxMessageBox(msg, MB_OK);
		}
		free(dstData);
		free(srcData);
	}

	return REPLY_OK;
}

//--- udp_data -----------------------------------------------------------------
int udp_data(const wchar_t *in_path, const wchar_t *out_path, const wchar_t *info_path, int pixel0, const wchar_t *bitsPerPixel)
{
	SBmpInfo info;
	BYTE *data1=NULL, *data=NULL, *pdata;
	ULONG len, written;
	BYTE *buf, *dst;
	HANDLE file;
	int i, blkSize, size;
	int srcDataLen, dstDataLen, h;
	int backwards, rightToLeft;
	wchar_t path[256];

	int BlockSize[4] = {45*32, 90*32, 180*32, 270*32};


	BMP_Read(in_path, &info, &data1);
	invert_data(&info, data1);
	convert_bppx(&info, bitsPerPixel, data1, &data);
	srcDataLen = (info.width*info.bitsPerPixel+7)/8;
	dstDataLen = ((info.width*info.bitsPerPixel+255)&~255)/8; // alignment to 256 bits
	
	for (i=0; i<SIZEOF(BlockSize); i++)
	{	
		pdata=data;
		blkSize = BlockSize[i];
		
		wsprintf(path, L"%s%d.udp", out_path, blkSize);
		file = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if (file==INVALID_HANDLE_VALUE)  return REPLY_ERROR;
		size = ((dstDataLen*info.height+blkSize-1)/blkSize) * blkSize;
		buf  = (BYTE*)malloc(size);
		memset(buf, 0, size);

		for (h=0, dst=buf+dstDataLen*info.height; h<info.height; h++)
		{
			dst  -= dstDataLen;
			memcpy(dst,	pdata, srcDataLen);
			pdata += info.lineLen;
		}

		write_hex(file, buf, size);
		CloseHandle(file);
		free(buf);
	}
	free(data1);
	free(data);

	//--- info ---------------------------
	for  (int i=0; i<4; i++)
	{
		backwards   = i&1;
		rightToLeft = (i>>1)&1;

		wsprintf(path, L"%s%s%s.txt", info_path, backwards?"b":"f", rightToLeft?"r":"l");

		file = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if (file==INVALID_HANDLE_VALUE)  return REPLY_ERROR;

		char str[128];

		len = sprintf(str, "bitPerPixel:\t%d\r\n",		info.bitsPerPixel);	WriteFile(file, str, len, &written, NULL);
		len = sprintf(str, "widthPx:\t%d\r\n",			info.width);		WriteFile(file, str, len, &written, NULL);
		len = sprintf(str, "widthBytes:\t%d\r\n",		srcDataLen);		WriteFile(file, str, len, &written, NULL);
		len = sprintf(str, "lengthPx:\t%d\r\n",			info.height);		WriteFile(file, str, len, &written, NULL);
		len = sprintf(str, "blockNo:\t%d\r\n",			0);					WriteFile(file, str, len, &written, NULL);
//		len = sprintf(str, "blockCnt:\t%d\r\n",			size/BLOCK_SIZE);	WriteFile(file, str, len, &written, NULL);
		len = sprintf(str, "jetPx0:\t\t%d\r\n",			pixel0);			WriteFile(file, str, len, &written, NULL);
		len = sprintf(str, "backward:\t%d\r\n",			backwards);			WriteFile(file, str, len, &written, NULL);
		len = sprintf(str, "flipHorizontal:\t%d\r\n",	rightToLeft);		WriteFile(file, str, len, &written, NULL);
		len = sprintf(str, "clearBlockUsed:\t%d\r\n",	0);					WriteFile(file, str, len, &written, NULL);
		CloseHandle(file);
	}

	return REPLY_OK;
}

