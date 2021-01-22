// ****************************************************************************
//
// rx_tas.cpp	Threshold arrays
//
// ****************************************************************************
//
//	Copyright 2017 by BM-Printing SA, Switzerland. All rights reserved.
//	Written by Gilbert Therville
//
// ****************************************************************************
//
//
////#include "stdafx.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_trace.h"
#include "rx_xml.h"
#include "fastlz.h"
#include "rx_common.h"
#include "rx_file.h"
#include "rx_tas.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
// #include <io.h>

//
// utility functions in support of parsing key/value pairs out of a threshold array file's header
//
#define MAX_HEADER_LEN 2048
#define BLOCK_SIZE 2048 // number of bytes to read/block for hexbinary files

char *lSkipWhiteSpace(char *buf)
{
	while (1)
	{
		char val = *buf;

		if ((val < ' ') && (val != '\t') && (val != '\n') && (val != '\r')) return NULL; // Binary data
		if (val > ' ') return buf;
		buf++;
	}
}

char *lFindEndOfKey(char *buf)
{
	while (1)
	{
		char val = *buf;

		if ((val == '=') || (val == ' ')) return buf;
		if ((val < ' ') && (val != '\t') && (val != '\n') && (val != '\r')) return NULL;  // binary data!
		buf++;
	}
}

char *lFindEndOfValue(char *buf)
{
	while (1)
	{
		char val = *buf;

		if (val == '\"') return buf;
		if ((val < ' ') && (val != '\t') && (val != '\n') && (val != '\r')) return NULL;  // binary data!
		buf++;
	}
}
//
// Parses through a buffer to determine key/value pairs
//
// returns a pointer to the next buffer location following the value or NULL if unexpected data found
//
// Note - Key names must be specified using the ascii subset (33-127) 
//        Whitespace skipped is " ", "\t", "\n"
//        Key names and values are null terminated as they are found
//
char *lLookForKeyValuePair(char *buf, char **key, char **value)
{
	char *ptr;
	char *keyPtr;
	int  keyPtrLen;
	char *valuePtr;
	int  valuePtrLen;

	*key = *value = NULL;
	// First, look for the "/" character
	if ((ptr = lSkipWhiteSpace(buf)) == NULL) return NULL;
	if ((ptr = strchr(buf, '/')) != NULL)
	{
		keyPtr = ++ptr; // point to the start of the key name
		// Find the end of the key ("=" or " ")
		if ((ptr = lFindEndOfKey(ptr)) == NULL) return NULL;
		keyPtrLen = (int)(ptr - keyPtr);
		if ((ptr = lSkipWhiteSpace(ptr)) == NULL) return NULL; // skips to the '=' 
		if (*ptr != '=') return NULL;
		if ((ptr = lSkipWhiteSpace(ptr + 1)) == NULL) return NULL;
		if (*ptr != '\"') return NULL;
		ptr++; // point past the "
		valuePtr = ptr;
		if ((ptr = lFindEndOfValue(ptr)) == NULL) return NULL;
		valuePtrLen = (int)(ptr - valuePtr);
		if ((ptr = lSkipWhiteSpace(ptr + 1)) == NULL) return NULL;
		keyPtr[keyPtrLen] = '\0';
		valuePtr[valuePtrLen] = '\0';
		*key = keyPtr;
		*value = valuePtr;
	}
	if (ptr != NULL && *ptr == 0) ptr = NULL;
	return ptr;
}


//--- rx_tas_load ------------------------------------------------------------
int rx_tas_load(const char * path, STaConfig **pta) 
{
	FILE				*file= NULL;
	unsigned char buf[BLOCK_SIZE];
	unsigned char *buffer;
	STaConfig *lta;
	int error = REPLY_ERROR, bytesRead = 0, numRequiredKeysRead = 0, size1X = 0, size1Y = 0, offsetToEndOfHeader = 0, bufsize = 0;

	bool isTAFile = false, is16Bit = false, hexBinaryEncoding = false, haveHeader= true;

	file = rx_fopen(path, "rb", _SH_DENYNO);
	if (file)
		bytesRead = (int)fread(buf, 1, BLOCK_SIZE, file);

	if (bytesRead > 0)
	{
		char *ptr;
		char *key, *value;
		buf[bytesRead-1] = '\0'; // make sure there is a null terminator on the buffer for string processing
		if ((ptr = lLookForKeyValuePair((char *)buf, &key, &value)) != NULL)
		{
			if ((strcmp(key, "FileType") != 0) || (strcmp(value, "ThresholdArrayData") != 0))
			{
				ptr = NULL; //not a threshold array file
			}
			else isTAFile = true;
		}
		while (ptr && (*ptr == '/') && ((ptr = lLookForKeyValuePair(ptr, &key, &value)) != NULL))
		{
			if (strcmp(key, "Is16Bit") == 0)
			{
				numRequiredKeysRead++;
				if (strcmp(value, "true") == 0)
				{
					is16Bit = true;
				}
				else if (strcmp(value, "false") == 0)
				{
					is16Bit = false;
				}
				else ptr = NULL; // Bad parameter
			}
			else if (strcmp(key, "Size1") == 0)
			{
				int width, height;

				// format is width height
				if (sscanf(value, "%d %d", &width, &height) == 2)
				{
					numRequiredKeysRead++;
					size1X = width;
					size1Y = height;
				}
				else ptr = NULL; // Bad parameter
			}
			else if (strcmp(key, "Encoding") == 0)
			{
				numRequiredKeysRead++;
				if (strcmp(value, "Binary") == 0)
				{
					hexBinaryEncoding = false;
				}
				else if (strcmp(value, "HexBinary") == 0)
				{
					hexBinaryEncoding = true;
				}
				else ptr = NULL; // Bad parameter
			}
		}
		if ((ptr != NULL) && (isTAFile) && (numRequiredKeysRead == 3) && (size1X > 0) && (size1Y > 0))
		{
			bufsize = size1X * size1Y;
			if (is16Bit) bufsize *= 2;

			offsetToEndOfHeader = (int)((unsigned char *)ptr - buf);
			rx_file_seek(file, offsetToEndOfHeader, SEEK_SET);
			bytesRead = (int)fread(buf, 1, 2, file);
			if ((bytesRead != 2) || (buf[0] != '<') || (buf[1] != '<')) {
				fclose(file);
				return error;
			}
			buffer = (unsigned char*)malloc(bufsize);
			if (!buffer){
				fclose(file);
				return error;
			}

			if (hexBinaryEncoding)
			{
				int volatile numBytesProcessed = 0;
				unsigned char *outptr= buffer;
				int volatile hi = -1;
				int volatile lo = -1;
				// we read the data in a block at a time and convert it

				while (numBytesProcessed < bufsize)
				{
					unsigned char *inptr = buf;
					int i;
					bytesRead = (int)fread(buf, 1, BLOCK_SIZE, file);
					if (bytesRead < 0) {
						fclose(file);
						return error;
					}

					for (i = 0; i < bytesRead; i++)
					{
						if (isdigit(*inptr))
						{
							if (hi == -1)
							{
								hi = *inptr - '0';
							}
							else
							{
								lo = *inptr - '0';
							}
						}
						else if ((*inptr >= 'A') && (*inptr <= 'F'))
						{
							if (hi == -1)
							{
								hi = *inptr - 'A' + 10;
							}
							else
							{
								lo = *inptr - 'A' + 10;
							}
						}
						else if ((*inptr >= 'a') && (*inptr <= 'f'))
						{
							if (hi == -1)
							{
								hi = *inptr - 'a' + 10;
							}
							else
							{
								lo = *inptr - 'a' + 10;
							}
						}
						if ((hi != -1) && (lo != -1))
						{
							*outptr = (unsigned char)((hi << 4) + lo);
							hi = lo = -1;
							outptr++;
							numBytesProcessed++;
							if (numBytesProcessed == bufsize)
							{
								break;
							}
						}
						inptr++;
					}
					if (bytesRead == 0)
					{
						if (numBytesProcessed != bufsize)
						{
							free(buffer);
							fclose(file);
							return error;
						}
						break;
					}
				}

			}
			else
			{
				bytesRead = (int)fread(buffer, 1, bufsize, file);
				if (bytesRead != bufsize)
				{
					free(buffer);
					fclose(file);
					return error;
				}

			}
			lta = (STaConfig *)malloc(sizeof (STaConfig));
			if (!is16Bit) {
				int volatile numProcessed = 0;
				UINT16 *outptr;
				BYTE *inptr;
				inptr = lta->ta = (BYTE*)buffer;
				outptr = lta->ta16 = (UINT16*)malloc(bufsize * 2);
				srand(1); // ensure we have the same seed for all plane
				while (numProcessed < bufsize)
				{
					int val = ((int)*inptr * 257) - (rand() % 256) -1 ;
					*outptr = (UINT16)val;
					numProcessed++; inptr++; outptr++;
				}
/*				{
					int test[] = { 128, 1*256,64*256,128*256,192*256,255*256,(255 * 256) + 128 };
					int resu[] = { 0,0,0,0,0,0,0 };
					int resu16[] = { 0,0,0,0,0,0,0 };
					for (int i = 0; i < SIZEOF(test); i++) {
						inptr = lta->ta;
						outptr = lta->ta16;
						for (int j = 0; j < bufsize; j++) {
							if ((test[i]/256) >= inptr[j])
								resu[i]++;
							if (test[i] >= outptr[j])
								resu16[i]++;
						}
						TrPrintfL(20, "tas load result for value %d = 8bits: %d, 16 bits: %d", test[i],resu[i],resu16[i]);
					}

				}
*/
			}
			else {
				int volatile numProcessed = 0;
				BYTE *outptr;
				UINT16 *inptr;
				inptr = lta->ta16 = (UINT16*)buffer;
				outptr = lta->ta = (BYTE*)malloc(bufsize / 2);
				while (numProcessed < bufsize / 2)
				{
					*outptr = (BYTE)(int)(*inptr + 127) / 256;
					numProcessed++; inptr++; outptr++;
				}

			}
			lta->width = size1X;
			lta->heigth = size1Y;
			*pta = lta;
			error= REPLY_OK;
		}
	}
	if (file) fclose(file);
	return error;
}


//--- rx_tas_rotate ------------------------------------------------------------
int rx_tas_rotate(STaConfig *ptasrc, int idx, STaConfig **pta)
{
	int bufsize, iLineCnt, iColCnt;
	BYTE *pSrc, *pDes;
	UINT16 *pSrc16, *pDes16;
	STaConfig *lta;

	lta = (STaConfig *)malloc(sizeof(STaConfig));
	if ((idx == 1) || (idx == 3)) {
		lta->width = ptasrc->heigth;
		lta->heigth = ptasrc->width;
	}
	else if ((idx == 0) || (idx == 2)) {
		lta->heigth = ptasrc->heigth;
		lta->width = ptasrc->width;
	}
	else 
		return Error(ERR_CONT, 0, "rx_tas_rotate for idx 0,1,2 or 3", idx);

	bufsize= lta->width * lta->heigth;
	 lta->ta= (BYTE *) malloc (bufsize);
	lta->ta16 = (UINT16*)malloc(bufsize * 2);

	switch (idx)
	{
	default:
	case 0:
		memcpy(lta->ta, ptasrc->ta, bufsize);
		memcpy(lta->ta16, ptasrc->ta16, bufsize*2);
		break;
	case 1: // 90°
		for (iLineCnt = 0; iLineCnt < ptasrc->heigth; iLineCnt++)
		{
			pSrc = ptasrc->ta + (iLineCnt * ptasrc->width);
			pSrc16 = ptasrc->ta16 + (iLineCnt * ptasrc->width);
			for (iColCnt = 0; iColCnt < ptasrc->width; iColCnt++)
			{
				pDes = lta->ta + ((iColCnt)* lta->width) + ((lta->width - iLineCnt - 1));
				*pDes = *pSrc;
				pSrc++;
				pDes16 = lta->ta16 + ((iColCnt)* lta->width) + ((lta->width - iLineCnt - 1));
				*pDes16 = *pSrc16;
				pSrc16++;

			}
		}
		break;
	case 2: // 180 °
		for (iLineCnt = 0; iLineCnt < ptasrc->heigth; iLineCnt++)
		{
			pSrc = ptasrc->ta + (iLineCnt * ptasrc->width);
			pSrc16 = ptasrc->ta16 + (iLineCnt * ptasrc->width);
			pDes = lta->ta + ((lta->heigth - iLineCnt - 1) * lta->width) + ((lta->width - 1) );
			pDes16 = lta->ta16 + ((lta->heigth - iLineCnt - 1) * lta->width) + ((lta->width - 1));
			for (iColCnt = 0; iColCnt < ptasrc->width; iColCnt++)
			{
				*pDes = *pSrc;
				pSrc++;
				pDes-- ;
				*pDes16 = *pSrc16;
				pSrc16++;
				pDes16--;
			}
		}
		break;
	case 3: // 270 °
		for (iLineCnt = 0; iLineCnt < ptasrc->heigth; iLineCnt++)
		{
			pSrc = ptasrc->ta + (iLineCnt * ptasrc->width);
			pSrc16 = ptasrc->ta16 + (iLineCnt * ptasrc->width);
			for (iColCnt = 0; iColCnt < ptasrc->width; iColCnt++)
			{
				pDes = lta->ta + ((lta->heigth - iColCnt - 1)* lta->width) + ((iLineCnt));
				*pDes = *pSrc;
				pSrc++;
				pDes16 = lta->ta16 + ((lta->heigth - iColCnt - 1)* lta->width) + ((iLineCnt));
				*pDes16 = *pSrc16;
				pSrc16++;
			}
		}

		break;
	}
	*pta = lta;
	return REPLY_OK;
}


//--- rx_tas_shift ------------------------------------------------------------
int rx_tas_shift(STaConfig *ptasrc, STaConfig **pta)
{
	int bufsize, iLineCnt, width1, width2, height1, height2;
	BYTE *pSrc, *pDes;
	UINT16 *pSrc16, *pDes16;
	STaConfig *lta;

	lta = (STaConfig *)malloc(sizeof(STaConfig));
	lta->width = ptasrc->heigth;
	lta->heigth = ptasrc->width;
	bufsize = lta->width * lta->heigth;
	lta->ta = (BYTE *)malloc(bufsize);
	lta->ta16 = (UINT16*)malloc(bufsize * 2);
	width1 = lta->width / 2;
	width2 = lta->width - width1;
	height1 = lta->heigth / 2;
	height2 = lta->heigth - height1;
	for (iLineCnt = 0; iLineCnt < height1; iLineCnt++)
	{
		// copy quater TL to BR
		pSrc = ptasrc->ta + (iLineCnt * ptasrc->width);
		pDes = lta->ta + ((height1 + iLineCnt) * lta->width) + width2;
		memcpy(pDes, pSrc, width1);

		pSrc16 = ptasrc->ta16 + (iLineCnt * ptasrc->width);
		pDes16 = lta->ta16 + ((height1 + iLineCnt) * lta->width) + width2;
		memcpy(pDes16, pSrc16, width1 * 2);

		// copy quater TR to BL
		pSrc = ptasrc->ta + (iLineCnt * ptasrc->width) + width1;
		pDes = lta->ta + ((height1 + iLineCnt) * lta->width);
		memcpy(pDes, pSrc, width2);

		pSrc16 = ptasrc->ta16 + (iLineCnt * ptasrc->width) + width1;
		pDes16 = lta->ta16 + ((height1 + iLineCnt) * lta->width);
		memcpy(pDes16, pSrc16, width2 * 2);
	}

	for (iLineCnt = 0; iLineCnt < height2; iLineCnt++)
	{
		// copy quater BL to TR
		pSrc = ptasrc->ta + ((height1+iLineCnt) * ptasrc->width);
		pDes = lta->ta + (iLineCnt * lta->width) + width2;
		memcpy(pDes, pSrc, width1);

		pSrc16 = ptasrc->ta16 + ((height1 + iLineCnt) * ptasrc->width);
		pDes16 = lta->ta16 + (iLineCnt * lta->width) + width2;
		memcpy(pDes16, pSrc16, width1 * 2);

		// copy quater BR to TL
		pSrc = ptasrc->ta + ((height1 + iLineCnt) * ptasrc->width) + width1;
		pDes = lta->ta + (iLineCnt * lta->width);
		memcpy(pDes, pSrc, width2);

		pSrc16 = ptasrc->ta16 + ((height1 + iLineCnt) * ptasrc->width) + width1;
		pDes16 = lta->ta16 + (iLineCnt * lta->width);
		memcpy(pDes16, pSrc16, width2 * 2);
	}

	*pta = lta;
	return REPLY_OK;
}


//--- rx_tas_addnoise_old ------------------------------------------------------------
int rx_tas_addnoise_old(STaConfig *ptasrc, int noise)
{
	int iLineCnt, iColCnt, iSubLineCnt, iSubColCnt, iHeight, iWidth;
	BYTE *pSrc, lSrcNext, lSrcPrev;
	UINT16 *pSrc16, lSrc16Next, lSrc16Prev;
	int i, range, loop;

	if (noise > 64) // basic cell dimension depends on noise 16*16, 32*32, 64*64
		iHeight = 16;
	else if (noise > 32)
		iHeight = 32;
	else
		iHeight = 16;
	for (iLineCnt = 0; iLineCnt < ptasrc->heigth; iLineCnt+= iHeight)
	{
	if (iLineCnt + iHeight > ptasrc->heigth)
		iHeight = ptasrc->heigth - iLineCnt;
	if (noise > 64)
		iWidth = 16;
	else if (noise > 32)
		iWidth = 32;
	else
		iWidth = 16;
	for (iColCnt = 0; iColCnt < ptasrc->width; iColCnt+= iWidth)
		{
			if (iColCnt + iWidth > ptasrc->width)
				iWidth = ptasrc->width - iColCnt;
			loop = iHeight * iWidth * noise / 100;
			for (i = 0; i < loop; i++) {
				range = (rand() % (iHeight*iWidth));
				iSubLineCnt = range / iHeight;
				iSubColCnt = range % iHeight;
				pSrc = ptasrc->ta + ((iLineCnt + iSubLineCnt) * ptasrc->width) + iColCnt + iSubColCnt;
				pSrc16 = ptasrc->ta16 + ((iLineCnt + iSubLineCnt) * ptasrc->width) + iColCnt + iSubColCnt;
				lSrcNext = *pSrc;
				lSrc16Next = *pSrc16;
				if (i > 0) { // copy previous one to next (execpt  first)
					*pSrc = lSrcPrev;
					*pSrc16 = lSrc16Prev;
				}
				lSrcPrev = lSrcNext;
				lSrc16Prev = lSrc16Next;
			}
		}
	}
	return REPLY_OK;
}


//--- rx_tas_addnoise ------------------------------------------------------------
int rx_tas_addnoise(STaConfig *ptasrc, int noise)
{
	int iLineCnt, iColCnt, maxnoise, inoise;
	BYTE *pSrc;
	UINT16 *pSrc16;

	if (!noise) // early return
		return REPLY_OK;
	if (noise > 100) // noise max : 100%
		noise = 100;
	if (noise < 2) // min noise : 2% (+1 % - 1 %)
		noise = 2;

	pSrc = ptasrc->ta;
	pSrc16 = ptasrc->ta16;
	for (iLineCnt = 0; iLineCnt < ptasrc->heigth; iLineCnt ++)
	{
		for (iColCnt = 0; iColCnt < ptasrc->width; iColCnt ++, pSrc++, pSrc16++)
		{
			//ta
			maxnoise = min(*pSrc,255-*pSrc );
			maxnoise = maxnoise * noise / 100;
			if (maxnoise) {
				inoise = maxnoise -(rand() % (2 * maxnoise));
				*pSrc = (BYTE)(inoise + (int)*pSrc);
			}
			
			//ta16
			maxnoise = min(*pSrc16, 65535- *pSrc16);
			maxnoise = maxnoise * noise / 100;
			if (maxnoise) {
				inoise = maxnoise - (rand() % (2 * maxnoise));
				*pSrc16 = (UINT16)(inoise + (int)*pSrc16);
			}
		}
	}
	return REPLY_OK;
}


//--- rx_tas_free ------------------------------------------------------------
int rx_tas_free(STaConfig *pta)
{
	if (pta) {
		if (pta->ta)
			free(pta->ta);
		if (pta->ta16)
			free(pta->ta16);
		free(pta);
	}
	return REPLY_OK;
}