// ****************************************************************************
//
//	rx_antibleeding.c
//
// ****************************************************************************
//
//	Copyright 2019 by Radex AG, Switzerland. All rights reserved.
//	Written by Gilbert Therville 
//
// ****************************************************************************

//--- includes ---------------------------------------------------------------------

#include "rx_error.h"
#include "rx_trace.h"
#include "rx_threads.h"
#include "rx_mem.h"
#include "rx_screen.h"

#ifdef linux
	#include "errno.h"
#endif

#define HIGH8 (256*3/4)
#define LOW8  (256*1/4)
#define HIGH16 (65536*3/4)
#define LOW16  (65536*1/4)
#define STLOW 0 // status Low : < LOW
#define STMIDL 1 // status Middle between low and high
#define STHIGH 2 // status High > HIGH
#define DISTANCE 6 // min distance between 2 border to apply antibleeding
#define DISTANCE3 8 // min distance between 2 border to apply antibleeding algo 3
#define DISTANCE4 10 // min distance between 2 border to apply antibleeding algo 4

//--- SAntiBleedingThread-------------------------------------------------------------------------
typedef struct
{
	int				running;			// running yes/no
	HANDLE			hWaitEvent;			// handle on wait event
	int				ret;				// ret code
	SPlaneInfo		plane;				// plane info ptr
	SScreenConfig*	pscreenConfig;		// screen config ptr
} SAntiBleedingThread;

SAntiBleedingThread antibleedingthread[MAX_COLORS];


//--- _rx_antibleeding_std ----------------------------------------------------------------------
int _rx_antibleeding_std(SPlaneInfo *plane, SScreenConfig* pscreenConfig)
{
	UINT32 l;
	int i, iNewState, iLowToHigh, status, cptMiddle;
	BYTE *pSrc, *pLowToHigh;
	UINT16 *pWSrc, *pWLowToHigh;
	int FirstHighCol = plane->WidthPx;
	int LastHighCol = -1;
	int FirstHighLine = plane->lengthPx;
	int LastHighLine = -1;

	if (plane->bitsPerPixel == 16) {
		for (l = 0; l < plane->lengthPx; l++)
		{
			pWLowToHigh = 0;
			iLowToHigh = 0;
			pWSrc = (UINT16 *)(plane->buffer + ((UINT64)l * plane->lineLen));
			iNewState = 0; cptMiddle = 0;
			status = STLOW;
			for (i = 0; i < (int)plane->WidthPx; i++)
			{
				if (*pWSrc < (UINT16)LOW16) {									// value is low
					if (status != STLOW) {								// if new status
						if (status == STHIGH) {							// if high to low
							if ((i - iNewState) > DISTANCE) {			// if high for a while
								*(pWSrc - (UINT64)1) /= 2;						//		reduce edge high to low value
							}
						}
						pWLowToHigh = 0;
						iLowToHigh = 0;
						status = STLOW;
						iNewState = i; cptMiddle = 0;								//	remember new status ptr
					}
				}
				else if (*pWSrc > (UINT16)HIGH16) {								// value is high
					if (status != STHIGH) {								// if new status
						if (status == STLOW) {							// if low to high
							pWLowToHigh = pWSrc;						//		remember low status ptr
							iLowToHigh = i;
							if (cptMiddle) {
								pWLowToHigh --;
								iLowToHigh--;
							}
						}
						iNewState = i; cptMiddle = 0;								//	remember new status ptr
						status = STHIGH;
					}
					else {												// if same high status
						if ((pWLowToHigh) && ((i - iLowToHigh) > DISTANCE)) {	// if high for a while
							*pWLowToHigh /= 2;							//				reduce edge low to high value
							pWLowToHigh = 0;
							iLowToHigh = 0;
						}
					}
					if (FirstHighCol > i)	FirstHighCol =	i;			// memo for top and bottom edge algo
					if (LastHighCol < i)	LastHighCol =	i;
					if (FirstHighLine > (int)l)	FirstHighLine = (int)l;
					if (LastHighLine < (int)l)	LastHighLine = (int)l;
				}
				else {													// value is medium
					if ((cptMiddle) && (status != STMIDL)) {			// if new status (skip 1 middle value to accept 1 middle between 1 low and 1 high)
						pWLowToHigh = 0;
						iLowToHigh = 0;
						status = STMIDL;
						iNewState = l;								//	remember new status ptr
					}
					cptMiddle++;
				}
				pWSrc++;
			}
		}

		if ((FirstHighCol < (int)plane->WidthPx) && (LastHighCol >= FirstHighCol) && (FirstHighLine < (int)plane->lengthPx) && (LastHighLine >= FirstHighLine)) { // if some high value found
			if (FirstHighLine < 3) FirstHighLine = 0;
			else FirstHighLine -= 2;
			if (LastHighLine < (int)plane->lengthPx) LastHighLine++;
			if (LastHighLine < (int)plane->lengthPx) LastHighLine++;
			if (LastHighCol < (int)plane->WidthPx) LastHighCol++;

			for (i = FirstHighCol; i < LastHighCol; i++)
			{
				pWLowToHigh = 0;
				iLowToHigh = 0;
				iNewState = FirstHighLine; cptMiddle = 0;
				status = STLOW;
				for (l = (UINT32)FirstHighLine; l < (UINT32)LastHighLine; l++)
				{
					pWSrc = (UINT16 *)(plane->buffer + ((UINT64)l * plane->lineLen) + ((UINT64)i * 2));
					if (*pWSrc < (UINT16)LOW16) {									// value is low
						if (status != STLOW) {								// if new status
							if (status == STHIGH) {							// if high to low
								if ((l - iNewState) > DISTANCE) {	// if high for a while
									*(pWSrc - (UINT64)plane->WidthPx) /= 2;					//		reduce edge high to low value
								}
							}
							pWLowToHigh = 0;
							iLowToHigh = 0;
							status = STLOW;
							iNewState = l;	 cptMiddle = 0;							//	remember new status ptr
						}
					}
					else if (*pWSrc > (UINT16)HIGH16) {								// value is high
						if (status != STHIGH) {								// if new status
							if (status == STLOW) {							// if low to high
								pWLowToHigh = pWSrc;						//		remember low status ptr
								iLowToHigh = l;
								if (cptMiddle) {
									pWLowToHigh -= plane->lineLen / 2;
									iLowToHigh--;
								}
							}
							iNewState = l;								//	remember new status ptr
							status = STHIGH;
						}
						else {												// if same high status
							if ((pWLowToHigh) && ((l - iLowToHigh) > DISTANCE)) {	// if high for a while
								*pWLowToHigh /= 2;							//				reduce edge low to high value
								pWLowToHigh = 0;
							}
						}
					}
					else {													// value is medium
						if ((cptMiddle) && (status != STMIDL)) {			// if new status (skip 1 middle value to accept 1 middle between 1 low and 1 high)
							pWLowToHigh = 0;
							iLowToHigh = 0;
							status = STMIDL;
							iNewState = l;	 cptMiddle = 0;							//	remember new status ptr
						}
						cptMiddle++;
					}
				}
			}
		}
	}
	else { // bitsPerPixel == 8
		for (l = 0; l < plane->lengthPx; l++)
		{
			pLowToHigh = 0;
			iLowToHigh = 0; 
			iNewState = 0; cptMiddle = 0;
			pSrc = (BYTE *)(plane->buffer + ((UINT64)l * plane->lineLen));
			status = STLOW;
			for (i = 0; i < (int)plane->WidthPx; i++)
			{
				if (*pSrc < LOW8) {										// value is low
					if (status != STLOW) {								// if new status
						if (status == STHIGH) {							// if high to low
							if ((i - iNewState) > DISTANCE ) {		// if high for a while
								*(pSrc - 1) /= 2;						//		reduce edge high to low value
							}
						}
						pLowToHigh = 0;
						iLowToHigh = 0; 
						status = STLOW;
						iNewState = i;	 cptMiddle = 0;							//	remember new status ptr
					}
				}
				else if (*pSrc > HIGH8) {								// value is high
					if (status != STHIGH) {								// if new status
						if (status == STLOW) {							// if low to high
							pLowToHigh = pSrc;							//		remember low status ptr
							iLowToHigh = i; 
							if (cptMiddle) {
								pLowToHigh--;
								iLowToHigh--;
							}
						}
						iNewState = i; cptMiddle = 0;								//	remember new status ptr
						status = STHIGH;
					}
					else {												// if same high status
						if ((pLowToHigh) && ((i- iLowToHigh) > DISTANCE)) {	// if high for a while
							*pLowToHigh /= 2;							//				reduce edge low to high value
							pLowToHigh = 0;
							iLowToHigh = 0;
						}
					}
					if (FirstHighCol > i)	FirstHighCol = i;			// memo for top and bottom edge algo
					if (LastHighCol < i)	LastHighCol = i;
					if (FirstHighLine > (int)l)	FirstHighLine = (int)l;
					if (LastHighLine < (int)l)	LastHighLine = (int)l;
				}
				else {													// value is medium
					if ((cptMiddle) && (status != STMIDL)) {			// if new status (skip 1 middle value to accept 1 middle between 1 low and 1 high)
						pLowToHigh = 0;
						iLowToHigh = 0;
						status = STMIDL;
						iNewState = l;								//	remember new status ptr
					}
					cptMiddle++;
				}
				pSrc++;
			}
		}

		if ((FirstHighCol < (int)plane->WidthPx) && (LastHighCol >= FirstHighCol) && (FirstHighLine < (int)plane->lengthPx) && (LastHighLine >= FirstHighLine)) { // if some high value found
			if (FirstHighLine < 3) FirstHighLine = 0;
			else FirstHighLine -= 2;
			if (LastHighLine < (int)plane->lengthPx) LastHighLine++;
			if (LastHighLine < (int)plane->lengthPx) LastHighLine++;
			if (LastHighCol < (int)plane->WidthPx) LastHighCol++;

			for (i = FirstHighCol; i < LastHighCol; i++)
			{
				pLowToHigh = 0;
				iLowToHigh = 0;
				iNewState = FirstHighLine; cptMiddle = 0;
				status = STLOW;
				for (l = (UINT32)FirstHighLine; l < (UINT32)LastHighLine; l++)
				{
					pSrc = (BYTE *)(plane->buffer + ((UINT64)l * plane->lineLen) + ((UINT64)i));
					if (*pSrc < LOW8) {									// value is low
						if (status != STLOW) {								// if new status
							if (status == STHIGH) {							// if high to low
								if ((l - iNewState) > DISTANCE) {	// if high for a while
									*(pSrc - (UINT64)plane->lineLen) /= 2;					//		reduce edge high to low value
								}
							}
							pLowToHigh = 0;
							iLowToHigh = 0;
							status = STLOW;
							iNewState = l;	 cptMiddle = 0;							//	remember new status ptr
						}
					}
					else if (*pSrc > HIGH8) {								// value is high
						if (status != STHIGH) {								// if new status
							if (status == STLOW) {							// if low to high
								pLowToHigh = pSrc;						//		remember low status ptr
								iLowToHigh = l;
								if (cptMiddle) {
									pLowToHigh-= plane->lineLen;
									iLowToHigh--;
								}
							}
							iNewState = l;	 cptMiddle = 0;							//	remember new status ptr
							status = STHIGH;
						}
						else {												// if same high status
							if ((pLowToHigh) && ((l - iLowToHigh) > DISTANCE)) {	// if high for a while
								*pLowToHigh /= 2;							//				reduce edge low to high value
								pLowToHigh = 0;
								iLowToHigh = 0;
							}
						}
					}
					else {													// value is medium
						if ((cptMiddle) && (status != STMIDL)) {			// if new status (skip 1 middle value to accept 1 middle between 1 low and 1 high)
							pLowToHigh = 0;
							iLowToHigh = 0;
							status = STMIDL;
							iNewState = l;								//	remember new status ptr
						}
						cptMiddle++;
					}
				}
			}
		}
	}
	return (REPLY_OK);
}


//--- _rx_antibleeding_2 ----------------------------------------------------------------------
int _rx_antibleeding_2(SPlaneInfo *plane, SScreenConfig* pscreenConfig)
{
	UINT32 l;
	int i, iNewState, iLowToHigh, status, cptMiddle;
	BYTE *pSrc,  *pLowToHigh;
	UINT16 *pWSrc,  *pWLowToHigh;
	int FirstHighCol = plane->WidthPx;
	int LastHighCol = -1;
	int FirstHighLine = plane->lengthPx;
	int LastHighLine = -1;

	if (plane->bitsPerPixel == 16) {
		for (l = 0; l < plane->lengthPx; l++)
		{
			pWLowToHigh = 0;
			iLowToHigh = 0;
			iNewState = 0; cptMiddle = 0;
			pWSrc = (UINT16 *)(plane->buffer + ((UINT64)l * plane->lineLen));
			status = STLOW;
			for (i = 0; i < (int)plane->WidthPx; i++)
			{
				if (*pWSrc < (UINT16)LOW16) {									// value is low
					if (status != STLOW) {								// if new status
						if (status == STHIGH) {							// if high to low
							if ((i - iNewState) > DISTANCE) {	// if high for a while
								*(pWSrc - (UINT64)2) /= 2;
								*(pWSrc - (UINT64)1) /= 2;
//								*(pWSrc - (UINT64)2) = (*pWSrc / 2) + (*(pWSrc - (UINT64)2) / 2);	//		reduce edge high to low value
//								*(pWSrc - (UINT64)1) = (*pWSrc / 2) + (*(pWSrc - (UINT64)2) / 2);
							}
						}
						pWLowToHigh = 0;
						iLowToHigh = 0;
						status = STLOW;
						iNewState = i;	 cptMiddle = 0;							//	remember new status ptr
					}
				}
				else if (*pWSrc > (UINT16)HIGH16) {								// value is high
					if (status != STHIGH) {								// if new status
						if (status == STLOW) {							// if low to high
							pWLowToHigh = pWSrc;						//		remember low status ptr
							iLowToHigh = i;
							if (cptMiddle) {
								pWLowToHigh--;
								iLowToHigh--;
							}
						}
						iNewState = i;	 cptMiddle = 0;							//	remember new status ptr
						status = STHIGH;
					}
					else {												// if same high status
						if ((pWLowToHigh) && ((i - iLowToHigh) > DISTANCE)) {	// if high for a while
							*(pWLowToHigh + (UINT64)1)	/= 2;
							*(pWLowToHigh)				/= 2;
//							*(pWLowToHigh + (UINT64)1)	= (*(pWLowToHigh - (UINT64)1) / 2) + (*(pWLowToHigh + (UINT64)1) / 2);	//		reduce edge low to high value
//							*(pWLowToHigh)				= (*(pWLowToHigh - (UINT64)1) / 2) + (*(pWLowToHigh + (UINT64)1) / 2);
							pWLowToHigh = 0;
							iLowToHigh = 0;
						}
					}
					if (FirstHighCol > i)	FirstHighCol = i;			// memo for top and bottom edge algo
					if (LastHighCol < i)	LastHighCol = i;
					if (FirstHighLine > (int)l)	FirstHighLine = (int)l;
					if (LastHighLine < (int)l)	LastHighLine = (int)l;
				}
				else {													// value is medium
					if ((cptMiddle) && (status != STMIDL)) {			// if new status (skip 1 middle value to accept 1 middle between 1 low and 1 high)
						pWLowToHigh = 0;
						iLowToHigh = 0;
						status = STMIDL;
						iNewState = l;								//	remember new status ptr
					}
					cptMiddle++;
				}
				pWSrc++;
			}
		}

		if ((FirstHighCol < (int)plane->WidthPx) && (LastHighCol >= FirstHighCol) && (FirstHighLine < (int)plane->lengthPx) && (LastHighLine >= FirstHighLine)) { // if some high value found
			if (FirstHighLine < 3) FirstHighLine = 0;
			else FirstHighLine -= 2;
			if (LastHighLine < (int)plane->lengthPx) LastHighLine++;
			if (LastHighLine < (int)plane->lengthPx) LastHighLine++;
			if (LastHighCol < (int)plane->WidthPx) LastHighCol++;

			for (i = FirstHighCol; i < LastHighCol; i++)
			{
				pWLowToHigh = 0;
				iLowToHigh = 0;
				iNewState = FirstHighLine; cptMiddle = 0;
				status = STLOW;
				for (l = (UINT32)FirstHighLine; l < (UINT32)LastHighLine; l++)
				{
					pWSrc = (UINT16 *)(plane->buffer + ((UINT64)l * plane->lineLen) + ((UINT64)i * 2));
					if (*pWSrc < (UINT16)LOW16) {									// value is low
						if (status != STLOW) {								// if new status
							if (status == STHIGH) {							// if high to low
								if ((l - iNewState) > DISTANCE ) {	// if high for a while
									*(pWSrc - (UINT64)(plane->lineLen)) /= 2;
									*(pWSrc - (UINT64)(plane->lineLen / 2)) /= 2;
//									*(pWSrc - (UINT64)(plane->lineLen))	= ((*pWSrc) / 2) + (*(pWSrc - (UINT64)(plane->lineLen)) / 2);		//		reduce edge high to low value
//									*(pWSrc - (UINT64)(plane->lineLen/2))	= ((*pWSrc) / 2) + (*(pWSrc - (UINT64)(plane->lineLen)) / 2);
								}
							}
							pWLowToHigh = 0;
							iLowToHigh = 0;
							status = STLOW;
							iNewState = l;	 cptMiddle = 0;							//	remember new status ptr
						}
					}
					else if (*pWSrc > (UINT16)HIGH16) {								// value is high
						if (status != STHIGH) {								// if new status
							if (status == STLOW) {							// if low to high
								pWLowToHigh = pWSrc;						//		remember low status ptr
								iLowToHigh = l;
								if (cptMiddle) {
									pWLowToHigh -= plane->lineLen / 2;
									iLowToHigh--;
								}
							}
							iNewState = l; cptMiddle = 0;								//	remember new status ptr
							status = STHIGH;
						}
						else {												// if same high status
							if ((pWLowToHigh) && ((pWLowToHigh- (UINT64)plane->WidthPx)> (UINT16 *)plane->buffer) && ((l - iLowToHigh) > DISTANCE)) {	// if high for a while
								*(pWLowToHigh + (UINT64)(plane->lineLen / 2)) /= 2;
								*(pWLowToHigh) /= 2;
//								*(pWLowToHigh + (UINT64)(plane->lineLen / 2))	= (*(pWLowToHigh - (UINT64)(plane->lineLen / 2)) / 2) + (*(pWLowToHigh + (UINT64)(plane->lineLen / 2)) / 2);		//		reduce edge low to high value
//								*(pWLowToHigh)									= (*(pWLowToHigh - (UINT64)(plane->lineLen / 2)) / 2) + (*(pWLowToHigh + (UINT64)(plane->lineLen / 2)) / 2);
								pWLowToHigh = 0;
								iLowToHigh = 0;
							}
						}
					}
					else {													// value is medium
						if ((cptMiddle) && (status != STMIDL)) {			// if new status (skip 1 middle value to accept 1 middle between 1 low and 1 high)
							pWLowToHigh = 0;
							iLowToHigh = 0;
							status = STMIDL;
							iNewState = l;								//	remember new status ptr
						}
						cptMiddle++;
					}
				}
			}
		}
	}
	else { // bitsPerPixel == 8
		for (l = 0; l < plane->lengthPx; l++)
		{
			pLowToHigh = 0;
			iLowToHigh = 0;
			iNewState = 0; cptMiddle = 0;
			pSrc = (BYTE *)(plane->buffer + ((UINT64)l * plane->lineLen));
			status = STLOW;
			for (i = 0; i < (int)plane->WidthPx; i++)
			{
				if (*pSrc < LOW8) {
					// value is low
					if (status != STLOW) {								// if new status
						if (status == STHIGH) {							// if high to low
							if ((i - iNewState) > DISTANCE) {		// if high for a while
								*(pSrc - (UINT64)2) /= 2;
								*(pSrc - (UINT64)1) /= 2;
//								*(pSrc - (UINT64)2) = (*pSrc / 2) + (*(pSrc - (UINT64)2) / 2);	//		reduce edge high to low value
//								*(pSrc - (UINT64)1) = (*pSrc / 2) + (*(pSrc - (UINT64)2) / 2);
							}
						}
						pLowToHigh = 0;
						iLowToHigh = 0;
						status = STLOW;
						iNewState = i; cptMiddle = 0;
						//	remember new status ptr
					}
				}
				else if (*pSrc > HIGH8) {								// value is high
					if (status != STHIGH) {								// if new status
						if (status == STLOW) {							// if low to high
							pLowToHigh = pSrc;							//		remember low status ptr
							iLowToHigh = i;
							if (cptMiddle) {
								pLowToHigh--;
								iLowToHigh--;
							}
						}
						iNewState = i;	cptMiddle = 0;							//	remember new status ptr
						status = STHIGH;
					}
					else {												// if same high status
						if ((pLowToHigh) && ((i - iLowToHigh) > DISTANCE)) {	// if high for a while
							*(pLowToHigh + (UINT64)1) /= 2;
							*(pLowToHigh) /= 2;
//							*(pLowToHigh + (UINT64)1)	= (*(pLowToHigh - (UINT64)1) / 2) + (*(pLowToHigh + (UINT64)1) / 2);	//		reduce edge low to high value
//							*(pLowToHigh)				= (*(pLowToHigh - (UINT64)1) / 2) + (*(pLowToHigh + (UINT64)1) / 2);
							pLowToHigh = 0;
							iLowToHigh = 0;
						}
					}
					if (FirstHighCol > i)	FirstHighCol = i;			// memo for top and bottom edge algo
					if (LastHighCol < i)	LastHighCol = i;
					if (FirstHighLine > (int)l)	FirstHighLine = (int)l;
					if (LastHighLine < (int)l)	LastHighLine = (int)l;
				}
				else {													// value is medium
					if ((cptMiddle) && (status != STMIDL)) {			// if new status (skip 1 middle value to accept 1 middle between 1 low and 1 high)
						pLowToHigh = 0;
						iLowToHigh = 0;
						status = STMIDL;
						iNewState = i;								//	remember new status ptr
					}
					cptMiddle++;
				}
				pSrc++;
			}
		}

		if ((FirstHighCol < (int)plane->WidthPx) && (LastHighCol >= FirstHighCol) && (FirstHighLine < (int)plane->lengthPx) && (LastHighLine >= FirstHighLine)) { // if some high value found
			if (FirstHighLine < 3) FirstHighLine = 0;
			else FirstHighLine -= 2;
			if (LastHighLine < (int)plane->lengthPx) LastHighLine++;
			if (LastHighLine < (int)plane->lengthPx) LastHighLine++;
			if (LastHighCol < (int)plane->WidthPx) LastHighCol++;

			for (i = FirstHighCol; i < LastHighCol; i++)
			{
				pLowToHigh = 0;
				iLowToHigh = 0;
				iNewState = FirstHighLine;
				status = STLOW;
				cptMiddle = 0;
				for (l = (UINT32)FirstHighLine; l < (UINT32)LastHighLine; l++)
				{
					pSrc = (BYTE *)(plane->buffer + ((UINT64)l * plane->lineLen) + ((UINT64)i));
					if (*pSrc < LOW8) {									// value is low
						if (status != STLOW) {								// if new status
							if (status == STHIGH) {							// if high to low
								if ((l - iNewState) > DISTANCE) {	// if high for a while
									*(pSrc - (UINT64)(2 * plane->lineLen)) /= 2;
									*(pSrc - (UINT64)(1 * plane->lineLen)) /= 2;
//									*(pSrc - (UINT64)(2 * plane->lineLen))	= (*pSrc / 2) + (*(pSrc - (UINT64)(2 * plane->lineLen)) / 2);		//		reduce edge high to low value
//									*(pSrc - (UINT64)(1 * plane->lineLen))	= (*pSrc / 2) + (*(pSrc - (UINT64)(2 * plane->lineLen)) / 2);
								}
							}
							pLowToHigh = 0;
							iLowToHigh = 0;
							status = STLOW;
							cptMiddle = 0;
							iNewState = l;								//	remember new status ptr
						}
					}
					else if (*pSrc > HIGH8) {								// value is high
						if (status != STHIGH) {								// if new status
							if (status == STLOW) {							// if low to high
								pLowToHigh = pSrc;						//		remember low status ptr
								iLowToHigh = l;
								if (cptMiddle) {
									pLowToHigh-= plane->lineLen;
									iLowToHigh--;
								}
							}
							iNewState = l;								//	remember new status ptr
							status = STHIGH;
							cptMiddle = 0;
						}
						else {												// if same high status
							if ((pLowToHigh) && ((pLowToHigh - (UINT64)plane->lineLen) > (BYTE *)plane->buffer) && ((l - iLowToHigh) > DISTANCE )) {	// if high for a while
								*(pLowToHigh + (UINT64)plane->lineLen)	/= 2;
								*(pLowToHigh)							/= 2;
//								*(pLowToHigh + (UINT64)plane->lineLen)	= (*(pLowToHigh - (UINT64)plane->lineLen) / 2) + (*(pLowToHigh + (UINT64)plane->lineLen) / 2);		//		reduce edge low to high value
//								*(pLowToHigh)							= (*(pLowToHigh - (UINT64)plane->lineLen) / 2) + (*(pLowToHigh + (UINT64)plane->lineLen) / 2);
								pLowToHigh = 0;
								iLowToHigh = 0;
							}
						}
					}
					else {													// value is medium
						if ((cptMiddle) && (status != STMIDL)) {			// if new status (skip 1 middle value to accept 1 middle between 1 low and 1 high)
							pLowToHigh = 0;
							iLowToHigh = 0;
							status = STMIDL;
							iNewState = l;								//	remember new status ptr
						}
						cptMiddle++;
					}
				}
			}
		}
	}
	return (REPLY_OK);
}


//--- _rx_antibleeding_3 ----------------------------------------------------------------------
int _rx_antibleeding_3(SPlaneInfo *plane, SScreenConfig* pscreenConfig)
{
	UINT32 l;
	int i, iNewState, iLowToHigh, status, cptMiddle;
	BYTE *pSrc, *pLowToHigh;
	UINT16 *pWSrc, *pWLowToHigh;
	int FirstHighCol = plane->WidthPx;
	int LastHighCol = -1;
	int FirstHighLine = plane->lengthPx;
	int LastHighLine = -1;

	if (plane->bitsPerPixel == 16) {
		for (l = 0; l < plane->lengthPx; l++)
		{
			pWLowToHigh = 0;
			iLowToHigh = 0;
			iNewState = 0; cptMiddle = 0;
			pWSrc = (UINT16 *)(plane->buffer + ((UINT64)l * plane->lineLen));
			status = STLOW;
			for (i = 0; i < (int)plane->WidthPx; i++)
			{
				if (*pWSrc < (UINT16)LOW16) {									// value is low
					if (status != STLOW) {								// if new status
						if (status == STHIGH) {							// if high to low
							if ((i - iNewState) > DISTANCE3) {	// if high for a while
								*(pWSrc - (UINT64)3) /= 2;
								*(pWSrc - (UINT64)2) /= 2;
								*(pWSrc - (UINT64)1) /= 3;
							}
						}
						pWLowToHigh = 0;
						iLowToHigh = 0;
						status = STLOW;
						iNewState = i;	 cptMiddle = 0;							//	remember new status ptr
					}
				}
				else if (*pWSrc > (UINT16)HIGH16) {								// value is high
					if (status != STHIGH) {								// if new status
						if (status == STLOW) {							// if low to high
							pWLowToHigh = pWSrc;						//		remember low status ptr
							iLowToHigh = i;
							if (cptMiddle) {
								pWLowToHigh--;
								iLowToHigh--;
							}
						}
						iNewState = i;	 cptMiddle = 0;							//	remember new status ptr
						status = STHIGH;
					}
					else {												// if same high status
						if ((pWLowToHigh) && ((i - iLowToHigh) > DISTANCE3)) {	// if high for a while
							*(pWLowToHigh + (UINT64)2) /= 2;
							*(pWLowToHigh + (UINT64)1) /= 2;
							*(pWLowToHigh) /= 3;
							//							*(pWLowToHigh + (UINT64)1)	= (*(pWLowToHigh - (UINT64)1) / 2) + (*(pWLowToHigh + (UINT64)1) / 2);	//		reduce edge low to high value
							//							*(pWLowToHigh)				= (*(pWLowToHigh - (UINT64)1) / 2) + (*(pWLowToHigh + (UINT64)1) / 2);
							pWLowToHigh = 0;
							iLowToHigh = 0;
						}
					}
					if (FirstHighCol > i)	FirstHighCol = i;			// memo for top and bottom edge algo
					if (LastHighCol < i)	LastHighCol = i;
					if (FirstHighLine > (int)l)	FirstHighLine = (int)l;
					if (LastHighLine < (int)l)	LastHighLine = (int)l;
				}
				else {													// value is medium
					if ((cptMiddle) && (status != STMIDL)) {			// if new status (skip 1 middle value to accept 1 middle between 1 low and 1 high)
						pWLowToHigh = 0;
						iLowToHigh = 0;
						status = STMIDL;
						iNewState = l;								//	remember new status ptr
					}
					cptMiddle++;
				}
				pWSrc++;
			}
		}

		if ((FirstHighCol < (int)plane->WidthPx) && (LastHighCol >= FirstHighCol) && (FirstHighLine < (int)plane->lengthPx) && (LastHighLine >= FirstHighLine)) { // if some high value found
			if (FirstHighLine < 3) FirstHighLine = 0;
			else FirstHighLine -= 2;
			if (LastHighLine < (int)plane->lengthPx) LastHighLine++;
			if (LastHighLine < (int)plane->lengthPx) LastHighLine++;
			if (LastHighCol < (int)plane->WidthPx) LastHighCol++;

			for (i = FirstHighCol; i < LastHighCol; i++)
			{
				pWLowToHigh = 0;
				iLowToHigh = 0;
				iNewState = FirstHighLine; cptMiddle = 0;
				status = STLOW;
				for (l = (UINT32)FirstHighLine; l < (UINT32)LastHighLine; l++)
				{
					pWSrc = (UINT16 *)(plane->buffer + ((UINT64)l * plane->lineLen) + ((UINT64)i * 2));
					if (*pWSrc < (UINT16)LOW16) {									// value is low
						if (status != STLOW) {								// if new status
							if (status == STHIGH) {							// if high to low
								if ((l - iNewState) > DISTANCE3) {	// if high for a while
									*(pWSrc - (UINT64)(3*plane->lineLen/2))	/= 2;
									*(pWSrc - (UINT64)(plane->lineLen))		/= 2;
									*(pWSrc - (UINT64)(plane->lineLen / 2)) /= 3;
									//									*(pWSrc - (UINT64)(plane->lineLen))	= ((*pWSrc) / 2) + (*(pWSrc - (UINT64)(plane->lineLen)) / 2);		//		reduce edge high to low value
									//									*(pWSrc - (UINT64)(plane->lineLen/2))	= ((*pWSrc) / 2) + (*(pWSrc - (UINT64)(plane->lineLen)) / 2);
								}
							}
							pWLowToHigh = 0;
							iLowToHigh = 0;
							status = STLOW;
							iNewState = l;	 cptMiddle = 0;							//	remember new status ptr
						}
					}
					else if (*pWSrc > (UINT16)HIGH16) {								// value is high
						if (status != STHIGH) {								// if new status
							if (status == STLOW) {							// if low to high
								pWLowToHigh = pWSrc;						//		remember low status ptr
								iLowToHigh = l;
								if (cptMiddle) {
									pWLowToHigh -= plane->lineLen / 2;
									iLowToHigh--;
								}
							}
							iNewState = l; cptMiddle = 0;								//	remember new status ptr
							status = STHIGH;
						}
						else {												// if same high status
							if ((pWLowToHigh) && ((pWLowToHigh - (UINT64)plane->WidthPx) > (UINT16 *)plane->buffer) && ((l - iLowToHigh) > DISTANCE3)) {	// if high for a while
								*(pWLowToHigh + (UINT64)(plane->lineLen))	/= 2;
								*(pWLowToHigh + (UINT64)(plane->lineLen / 2)) /= 2;
								*(pWLowToHigh) /= 3;
								//								*(pWLowToHigh + (UINT64)(plane->lineLen / 2))	= (*(pWLowToHigh - (UINT64)(plane->lineLen / 2)) / 2) + (*(pWLowToHigh + (UINT64)(plane->lineLen / 2)) / 2);		//		reduce edge low to high value
								//								*(pWLowToHigh)									= (*(pWLowToHigh - (UINT64)(plane->lineLen / 2)) / 2) + (*(pWLowToHigh + (UINT64)(plane->lineLen / 2)) / 2);
								pWLowToHigh = 0;
								iLowToHigh = 0;
							}
						}
					}
					else {													// value is medium
						if ((cptMiddle) && (status != STMIDL)) {			// if new status (skip 1 middle value to accept 1 middle between 1 low and 1 high)
							pWLowToHigh = 0;
							iLowToHigh = 0;
							status = STMIDL;
							iNewState = l;								//	remember new status ptr
						}
						cptMiddle++;
					}
				}
			}
		}
	}
	else { // bitsPerPixel == 8
		for (l = 0; l < plane->lengthPx; l++)
		{
			pLowToHigh = 0;
			iLowToHigh = 0;
			iNewState = 0; cptMiddle = 0;
			pSrc = (BYTE *)(plane->buffer + ((UINT64)l * plane->lineLen));
			status = STLOW;
			for (i = 0; i < (int)plane->WidthPx; i++)
			{
				if (*pSrc < LOW8) {
					// value is low
					if (status != STLOW) {								// if new status
						if (status == STHIGH) {							// if high to low
							if ((i - iNewState) > DISTANCE3) {		// if high for a while
								*(pSrc - (UINT64)3) /= 2;
								*(pSrc - (UINT64)2) /= 2;
								*(pSrc - (UINT64)1) /= 3;
								//								*(pSrc - (UINT64)2) = (*pSrc / 2) + (*(pSrc - (UINT64)2) / 2);	//		reduce edge high to low value
								//								*(pSrc - (UINT64)1) = (*pSrc / 2) + (*(pSrc - (UINT64)2) / 2);
							}
						}
						pLowToHigh = 0;
						iLowToHigh = 0;
						status = STLOW;
						iNewState = i; cptMiddle = 0;
						//	remember new status ptr
					}
				}
				else if (*pSrc > HIGH8) {								// value is high
					if (status != STHIGH) {								// if new status
						if (status == STLOW) {							// if low to high
							pLowToHigh = pSrc;							//		remember low status ptr
							iLowToHigh = i;
							if (cptMiddle) {
								pLowToHigh--;
								iLowToHigh--;
							}
						}
						iNewState = i;	cptMiddle = 0;							//	remember new status ptr
						status = STHIGH;
					}
					else {												// if same high status
						if ((pLowToHigh) && ((i - iLowToHigh) > DISTANCE3)) {	// if high for a while
							*(pLowToHigh + (UINT64)2) /= 2;
							*(pLowToHigh + (UINT64)1) /= 2;
							*(pLowToHigh) /= 3;
							//							*(pLowToHigh + (UINT64)1)	= (*(pLowToHigh - (UINT64)1) / 2) + (*(pLowToHigh + (UINT64)1) / 2);	//		reduce edge low to high value
							//							*(pLowToHigh)				= (*(pLowToHigh - (UINT64)1) / 2) + (*(pLowToHigh + (UINT64)1) / 2);
							pLowToHigh = 0;
							iLowToHigh = 0;
						}
					}
					if (FirstHighCol > i)	FirstHighCol = i;			// memo for top and bottom edge algo
					if (LastHighCol < i)	LastHighCol = i;
					if (FirstHighLine > (int)l)	FirstHighLine = (int)l;
					if (LastHighLine < (int)l)	LastHighLine = (int)l;
				}
				else {													// value is medium
					if ((cptMiddle) && (status != STMIDL)) {			// if new status (skip 1 middle value to accept 1 middle between 1 low and 1 high)
						pLowToHigh = 0;
						iLowToHigh = 0;
						status = STMIDL;
						iNewState = i;								//	remember new status ptr
					}
					cptMiddle++;
				}
				pSrc++;
			}
		}

		if ((FirstHighCol < (int)plane->WidthPx) && (LastHighCol >= FirstHighCol) && (FirstHighLine < (int)plane->lengthPx) && (LastHighLine >= FirstHighLine)) { // if some high value found
			if (FirstHighLine < 3) FirstHighLine = 0;
			else FirstHighLine -= 2;
			if (LastHighLine < (int)plane->lengthPx) LastHighLine++;
			if (LastHighLine < (int)plane->lengthPx) LastHighLine++;
			if (LastHighCol < (int)plane->WidthPx) LastHighCol++;

			for (i = FirstHighCol; i < LastHighCol; i++)
			{
				pLowToHigh = 0;
				iLowToHigh = 0;
				iNewState = FirstHighLine;
				status = STLOW;
				cptMiddle = 0;
				for (l = (UINT32)FirstHighLine; l < (UINT32)LastHighLine; l++)
				{
					pSrc = (BYTE *)(plane->buffer + ((UINT64)l * plane->lineLen) + ((UINT64)i));
					if (*pSrc < LOW8) {									// value is low
						if (status != STLOW) {								// if new status
							if (status == STHIGH) {							// if high to low
								if ((l - iNewState) > DISTANCE3) {	// if high for a while
									*(pSrc - (UINT64)(3 * plane->lineLen)) /= 2;
									*(pSrc - (UINT64)(2 * plane->lineLen)) /= 2;
									*(pSrc - (UINT64)(1 * plane->lineLen)) /= 3;
									//									*(pSrc - (UINT64)(2 * plane->lineLen))	= (*pSrc / 2) + (*(pSrc - (UINT64)(2 * plane->lineLen)) / 2);		//		reduce edge high to low value
									//									*(pSrc - (UINT64)(1 * plane->lineLen))	= (*pSrc / 2) + (*(pSrc - (UINT64)(2 * plane->lineLen)) / 2);
								}
							}
							pLowToHigh = 0;
							iLowToHigh = 0;
							status = STLOW;
							cptMiddle = 0;
							iNewState = l;								//	remember new status ptr
						}
					}
					else if (*pSrc > HIGH8) {								// value is high
						if (status != STHIGH) {								// if new status
							if (status == STLOW) {							// if low to high
								pLowToHigh = pSrc;						//		remember low status ptr
								iLowToHigh = l;
								if (cptMiddle) {
									pLowToHigh -= plane->lineLen;
									iLowToHigh--;
								}
							}
							iNewState = l;								//	remember new status ptr
							status = STHIGH;
							cptMiddle = 0;
						}
						else {												// if same high status
							if ((pLowToHigh) && ((pLowToHigh - (UINT64)plane->lineLen) > (BYTE *)plane->buffer) && ((l - iLowToHigh) > DISTANCE3)) {	// if high for a while
								*(pLowToHigh + (UINT64)2*plane->lineLen) /= 2;
								*(pLowToHigh + (UINT64)plane->lineLen) /= 2;
								*(pLowToHigh) /= 3;
								//								*(pLowToHigh + (UINT64)plane->lineLen)	= (*(pLowToHigh - (UINT64)plane->lineLen) / 2) + (*(pLowToHigh + (UINT64)plane->lineLen) / 2);		//		reduce edge low to high value
								//								*(pLowToHigh)							= (*(pLowToHigh - (UINT64)plane->lineLen) / 2) + (*(pLowToHigh + (UINT64)plane->lineLen) / 2);
								pLowToHigh = 0;
								iLowToHigh = 0;
							}
						}
					}
					else {													// value is medium
						if ((cptMiddle) && (status != STMIDL)) {			// if new status (skip 1 middle value to accept 1 middle between 1 low and 1 high)
							pLowToHigh = 0;
							iLowToHigh = 0;
							status = STMIDL;
							iNewState = l;								//	remember new status ptr
						}
						cptMiddle++;
					}
				}
			}
		}
	}
	return (REPLY_OK);
}


//--- _rx_antibleeding_4 ----------------------------------------------------------------------
int _rx_antibleeding_4(SPlaneInfo *plane, SScreenConfig* pscreenConfig)
{
	UINT32 l;
	int i, iNewState, iLowToHigh, status, cptMiddle;
	BYTE *pSrc, *pLowToHigh;
	UINT16 *pWSrc, *pWLowToHigh;
	int FirstHighCol = plane->WidthPx;
	int LastHighCol = -1;
	int FirstHighLine = plane->lengthPx;
	int LastHighLine = -1;

	if (plane->bitsPerPixel == 16) {
		for (l = 0; l < plane->lengthPx; l++)
		{
			pWLowToHigh = 0;
			iLowToHigh = 0;
			iNewState = 0; cptMiddle = 0;
			pWSrc = (UINT16 *)(plane->buffer + ((UINT64)l * plane->lineLen));
			status = STLOW;
			for (i = 0; i < (int)plane->WidthPx; i++)
			{
				if (*pWSrc < (UINT16)LOW16) {									// value is low
					if (status != STLOW) {								// if new status
						if (status == STHIGH) {							// if high to low
							if ((i - iNewState) > DISTANCE4) {	// if high for a while
								*(pWSrc - (UINT64)4) /= 2;
								*(pWSrc - (UINT64)3) /= 2;
								*(pWSrc - (UINT64)2) /= 3;
								*(pWSrc - (UINT64)1) /= 3;
							}
						}
						pWLowToHigh = 0;
						iLowToHigh = 0;
						status = STLOW;
						iNewState = i;	 cptMiddle = 0;							//	remember new status ptr
					}
				}
				else if (*pWSrc > (UINT16)HIGH16) {								// value is high
					if (status != STHIGH) {								// if new status
						if (status == STLOW) {							// if low to high
							pWLowToHigh = pWSrc;						//		remember low status ptr
							iLowToHigh = i;
							if (cptMiddle) {
								pWLowToHigh--;
								iLowToHigh--;
							}
						}
						iNewState = i;	 cptMiddle = 0;							//	remember new status ptr
						status = STHIGH;
					}
					else {												// if same high status
						if ((pWLowToHigh) && ((i - iLowToHigh) > DISTANCE4)) {	// if high for a while
							*(pWLowToHigh + (UINT64)3) /= 2;
							*(pWLowToHigh + (UINT64)2) /= 2;
							*(pWLowToHigh + (UINT64)1) /= 3;
							*(pWLowToHigh) /= 3;
							//							*(pWLowToHigh + (UINT64)1)	= (*(pWLowToHigh - (UINT64)1) / 2) + (*(pWLowToHigh + (UINT64)1) / 2);	//		reduce edge low to high value
							//							*(pWLowToHigh)				= (*(pWLowToHigh - (UINT64)1) / 2) + (*(pWLowToHigh + (UINT64)1) / 2);
							pWLowToHigh = 0;
							iLowToHigh = 0;
						}
					}
					if (FirstHighCol > i)	FirstHighCol = i;			// memo for top and bottom edge algo
					if (LastHighCol < i)	LastHighCol = i;
					if (FirstHighLine > (int)l)	FirstHighLine = (int)l;
					if (LastHighLine < (int)l)	LastHighLine = (int)l;
				}
				else {													// value is medium
					if ((cptMiddle) && (status != STMIDL)) {			// if new status (skip 1 middle value to accept 1 middle between 1 low and 1 high)
						pWLowToHigh = 0;
						iLowToHigh = 0;
						status = STMIDL;
						iNewState = l;								//	remember new status ptr
					}
					cptMiddle++;
				}
				pWSrc++;
			}
		}

		if ((FirstHighCol < (int)plane->WidthPx) && (LastHighCol >= FirstHighCol) && (FirstHighLine < (int)plane->lengthPx) && (LastHighLine >= FirstHighLine)) { // if some high value found
			if (FirstHighLine < 3) FirstHighLine = 0;
			else FirstHighLine -= 2;
			if (LastHighLine < (int)plane->lengthPx) LastHighLine++;
			if (LastHighLine < (int)plane->lengthPx) LastHighLine++;
			if (LastHighCol < (int)plane->WidthPx) LastHighCol++;

			for (i = FirstHighCol; i < LastHighCol; i++)
			{
				pWLowToHigh = 0;
				iLowToHigh = 0;
				iNewState = FirstHighLine; cptMiddle = 0;
				status = STLOW;
				for (l = (UINT32)FirstHighLine; l < (UINT32)LastHighLine; l++)
				{
					pWSrc = (UINT16 *)(plane->buffer + ((UINT64)l * plane->lineLen) + ((UINT64)i * 2));
					if (*pWSrc < (UINT16)LOW16) {									// value is low
						if (status != STLOW) {								// if new status
							if (status == STHIGH) {							// if high to low
								if ((l - iNewState) > DISTANCE4) {	// if high for a while
									*(pWSrc - (UINT64)(2 * plane->lineLen )) /= 2;
									*(pWSrc - (UINT64)(3 * plane->lineLen / 2)) /= 2;
									*(pWSrc - (UINT64)(plane->lineLen)) /= 3;
									*(pWSrc - (UINT64)(plane->lineLen / 2)) /= 3;
									//									*(pWSrc - (UINT64)(plane->lineLen))	= ((*pWSrc) / 2) + (*(pWSrc - (UINT64)(plane->lineLen)) / 2);		//		reduce edge high to low value
									//									*(pWSrc - (UINT64)(plane->lineLen/2))	= ((*pWSrc) / 2) + (*(pWSrc - (UINT64)(plane->lineLen)) / 2);
								}
							}
							pWLowToHigh = 0;
							iLowToHigh = 0;
							status = STLOW;
							iNewState = l;	 cptMiddle = 0;							//	remember new status ptr
						}
					}
					else if (*pWSrc > (UINT16)HIGH16) {								// value is high
						if (status != STHIGH) {								// if new status
							if (status == STLOW) {							// if low to high
								pWLowToHigh = pWSrc;						//		remember low status ptr
								iLowToHigh = l;
								if (cptMiddle) {
									pWLowToHigh -= plane->lineLen / 2;
									iLowToHigh--;
								}
							}
							iNewState = l; cptMiddle = 0;								//	remember new status ptr
							status = STHIGH;
						}
						else {												// if same high status
							if ((pWLowToHigh) && ((pWLowToHigh - (UINT64)plane->WidthPx) > (UINT16 *)plane->buffer) && ((l - iLowToHigh) > DISTANCE4)) {	// if high for a while
								*(pWLowToHigh + (UINT64)(3*plane->lineLen / 2)) /= 2;
								*(pWLowToHigh + (UINT64)(plane->lineLen)) /= 2;
								*(pWLowToHigh + (UINT64)(plane->lineLen / 2)) /= 3;
								*(pWLowToHigh) /= 3;
								//								*(pWLowToHigh + (UINT64)(plane->lineLen / 2))	= (*(pWLowToHigh - (UINT64)(plane->lineLen / 2)) / 2) + (*(pWLowToHigh + (UINT64)(plane->lineLen / 2)) / 2);		//		reduce edge low to high value
								//								*(pWLowToHigh)									= (*(pWLowToHigh - (UINT64)(plane->lineLen / 2)) / 2) + (*(pWLowToHigh + (UINT64)(plane->lineLen / 2)) / 2);
								pWLowToHigh = 0;
								iLowToHigh = 0;
							}
						}
					}
					else {													// value is medium
						if ((cptMiddle) && (status != STMIDL)) {			// if new status (skip 1 middle value to accept 1 middle between 1 low and 1 high)
							pWLowToHigh = 0;
							iLowToHigh = 0;
							status = STMIDL;
							iNewState = l;								//	remember new status ptr
						}
						cptMiddle++;
					}
				}
			}
		}
	}
	else { // bitsPerPixel == 8
		for (l = 0; l < plane->lengthPx; l++)
		{
			pLowToHigh = 0;
			iLowToHigh = 0;
			iNewState = 0; cptMiddle = 0;
			pSrc = (BYTE *)(plane->buffer + ((UINT64)l * plane->lineLen));
			status = STLOW;
			for (i = 0; i < (int)plane->WidthPx; i++)
			{
				if (*pSrc < LOW8) {
					// value is low
					if (status != STLOW) {								// if new status
						if (status == STHIGH) {							// if high to low
							if ((i - iNewState) > DISTANCE4) {		// if high for a while
								*(pSrc - (UINT64)4) /= 2;
								*(pSrc - (UINT64)3) /= 2;
								*(pSrc - (UINT64)2) /= 3;
								*(pSrc - (UINT64)1) /= 3;
								//								*(pSrc - (UINT64)2) = (*pSrc / 2) + (*(pSrc - (UINT64)2) / 2);	//		reduce edge high to low value
								//								*(pSrc - (UINT64)1) = (*pSrc / 2) + (*(pSrc - (UINT64)2) / 2);
							}
						}
						pLowToHigh = 0;
						iLowToHigh = 0;
						status = STLOW;
						iNewState = i; cptMiddle = 0;
						//	remember new status ptr
					}
				}
				else if (*pSrc > HIGH8) {								// value is high
					if (status != STHIGH) {								// if new status
						if (status == STLOW) {							// if low to high
							pLowToHigh = pSrc;							//		remember low status ptr
							iLowToHigh = i;
							if (cptMiddle) {
								pLowToHigh--;
								iLowToHigh--;
							}
						}
						iNewState = i;	cptMiddle = 0;							//	remember new status ptr
						status = STHIGH;
					}
					else {												// if same high status
						if ((pLowToHigh) && ((i - iLowToHigh) > DISTANCE4)) {	// if high for a while
							*(pLowToHigh + (UINT64)3) /= 2;
							*(pLowToHigh + (UINT64)2) /= 2;
							*(pLowToHigh + (UINT64)1) /= 3;
							*(pLowToHigh) /= 3;
							//							*(pLowToHigh + (UINT64)1)	= (*(pLowToHigh - (UINT64)1) / 2) + (*(pLowToHigh + (UINT64)1) / 2);	//		reduce edge low to high value
							//							*(pLowToHigh)				= (*(pLowToHigh - (UINT64)1) / 2) + (*(pLowToHigh + (UINT64)1) / 2);
							pLowToHigh = 0;
							iLowToHigh = 0;
						}
					}
					if (FirstHighCol > i)	FirstHighCol = i;			// memo for top and bottom edge algo
					if (LastHighCol < i)	LastHighCol = i;
					if (FirstHighLine > (int)l)	FirstHighLine = (int)l;
					if (LastHighLine < (int)l)	LastHighLine = (int)l;
				}
				else {													// value is medium
					if ((cptMiddle) && (status != STMIDL)) {			// if new status (skip 1 middle value to accept 1 middle between 1 low and 1 high)
						pLowToHigh = 0;
						iLowToHigh = 0;
						status = STMIDL;
						iNewState = i;								//	remember new status ptr
					}
					cptMiddle++;
				}
				pSrc++;
			}
		}

		if ((FirstHighCol < (int)plane->WidthPx) && (LastHighCol >= FirstHighCol) && (FirstHighLine < (int)plane->lengthPx) && (LastHighLine >= FirstHighLine)) { // if some high value found
			if (FirstHighLine < 3) FirstHighLine = 0;
			else FirstHighLine -= 2;
			if (LastHighLine < (int)plane->lengthPx) LastHighLine++;
			if (LastHighLine < (int)plane->lengthPx) LastHighLine++;
			if (LastHighCol < (int)plane->WidthPx) LastHighCol++;

			for (i = FirstHighCol; i < LastHighCol; i++)
			{
				pLowToHigh = 0;
				iLowToHigh = 0;
				iNewState = FirstHighLine;
				status = STLOW;
				cptMiddle = 0;
				for (l = (UINT32)FirstHighLine; l < (UINT32)LastHighLine; l++)
				{
					pSrc = (BYTE *)(plane->buffer + ((UINT64)l * plane->lineLen) + ((UINT64)i));
					if (*pSrc < LOW8) {									// value is low
						if (status != STLOW) {								// if new status
							if (status == STHIGH) {							// if high to low
								if ((l - iNewState) > DISTANCE4) {	// if high for a while
									*(pSrc - (UINT64)(4 * plane->lineLen)) /= 2;
									*(pSrc - (UINT64)(3 * plane->lineLen)) /= 2;
									*(pSrc - (UINT64)(2 * plane->lineLen)) /= 3;
									*(pSrc - (UINT64)(1 * plane->lineLen)) /= 3;
									//									*(pSrc - (UINT64)(2 * plane->lineLen))	= (*pSrc / 2) + (*(pSrc - (UINT64)(2 * plane->lineLen)) / 2);		//		reduce edge high to low value
									//									*(pSrc - (UINT64)(1 * plane->lineLen))	= (*pSrc / 2) + (*(pSrc - (UINT64)(2 * plane->lineLen)) / 2);
								}
							}
							pLowToHigh = 0;
							iLowToHigh = 0;
							status = STLOW;
							cptMiddle = 0;
							iNewState = l;								//	remember new status ptr
						}
					}
					else if (*pSrc > HIGH8) {								// value is high
						if (status != STHIGH) {								// if new status
							if (status == STLOW) {							// if low to high
								pLowToHigh = pSrc;						//		remember low status ptr
								iLowToHigh = l;
								if (cptMiddle) {
									pLowToHigh -= plane->lineLen;
									iLowToHigh--;
								}
							}
							iNewState = l;								//	remember new status ptr
							status = STHIGH;
							cptMiddle = 0;
						}
						else {												// if same high status
							if ((pLowToHigh) && ((pLowToHigh - (UINT64)plane->lineLen) > (BYTE *)plane->buffer) && ((l - iLowToHigh) > DISTANCE4)) {	// if high for a while
								*(pLowToHigh + (UINT64)3 * plane->lineLen) /= 2;
								*(pLowToHigh + (UINT64)2 * plane->lineLen) /= 2;
								*(pLowToHigh + (UINT64)plane->lineLen) /= 3;
								*(pLowToHigh) /= 3;
								//								*(pLowToHigh + (UINT64)plane->lineLen)	= (*(pLowToHigh - (UINT64)plane->lineLen) / 2) + (*(pLowToHigh + (UINT64)plane->lineLen) / 2);		//		reduce edge low to high value
								//								*(pLowToHigh)							= (*(pLowToHigh - (UINT64)plane->lineLen) / 2) + (*(pLowToHigh + (UINT64)plane->lineLen) / 2);
								pLowToHigh = 0;
								iLowToHigh = 0;
							}
						}
					}
					else {													// value is medium
						if ((cptMiddle) && (status != STMIDL)) {			// if new status (skip 1 middle value to accept 1 middle between 1 low and 1 high)
							pLowToHigh = 0;
							iLowToHigh = 0;
							status = STMIDL;
							iNewState = l;								//	remember new status ptr
						}
						cptMiddle++;
					}
				}
			}
		}
	}
	return (REPLY_OK);
}



//--- _rx_antibleeding_plane ----------------------------------------------------------------------
int _rx_antibleeding_plane(SPlaneInfo *plane, SScreenConfig* pscreenConfig)
{

	if (plane->buffer == NULL) return Error(ERR_CONT, 0, "No buffer for _rx_antibleeding_plane: %d", plane->planenumber);

	if (pscreenConfig->antiBleeding==2)
		return (_rx_antibleeding_2(plane, pscreenConfig));
	else if (pscreenConfig->antiBleeding == 3)
		return (_rx_antibleeding_3(plane, pscreenConfig));
	else if (pscreenConfig->antiBleeding == 4)
		return (_rx_antibleeding_4(plane, pscreenConfig));
	else
		return (_rx_antibleeding_std(plane, pscreenConfig));

}


//---  _rx_thread_antibleeding_plane ------------------------------------------------------------
static void *_rx_thread_antibleeding_plane(void *par)
{
	SAntiBleedingThread *pantibleedingthread = (void *)par;
	pantibleedingthread->running = 1;
	pantibleedingthread->ret = _rx_antibleeding_plane(&pantibleedingthread->plane, pantibleedingthread->pscreenConfig);
	pantibleedingthread->running = 0;
	rx_sem_post(pantibleedingthread->hWaitEvent);

	return NULL;
}


//--- rx_antibleeding_bmp ----------------------------------------------------------------------
int rx_antibleeding_bmp(SBmpInfo *pinfo, SScreenConfig* pscreenConfig)
{
	int iCntPlane, l_threadMax, j, h, k;
	int ret = REPLY_OK;
	HANDLE hWaitEvent;

	if (!pinfo->planes) return Error(ERR_CONT, 0, "No planes in bitmap");

	for (j = 0; j < pinfo->planes; j++)
	{
		antibleedingthread[j].ret = REPLY_OK;
		antibleedingthread[j].running = 0;
	}
	iCntPlane = 0;
	l_threadMax = rx_core_cnt();
	while (iCntPlane < pinfo->planes)
	{
		hWaitEvent = rx_sem_create();
		if (!hWaitEvent)
			return Error(ERR_CONT, 0, "Error rx_sem_create in rx_antibleeding_bmp");
		for (j = 0; j < l_threadMax; j++)
		{
			if (iCntPlane >= pinfo->planes)
				break;
			antibleedingthread[j].plane.aligment = pinfo->aligment;
			antibleedingthread[j].plane.WidthPx = pinfo->srcWidthPx;
			antibleedingthread[j].plane.lengthPx = pinfo->lengthPx;
			antibleedingthread[j].plane.bitsPerPixel = pinfo->bitsPerPixel;
			antibleedingthread[j].plane.lineLen = pinfo->lineLen;
			antibleedingthread[j].plane.dataSize = pinfo->dataSize;
			antibleedingthread[j].plane.resol = pinfo->resol;
			antibleedingthread[j].plane.planenumber = iCntPlane;
			antibleedingthread[j].plane.buffer = *pinfo->buffer[iCntPlane];
							   
			antibleedingthread[j].running = 1;
			antibleedingthread[j].hWaitEvent = hWaitEvent;
			antibleedingthread[j].pscreenConfig = pscreenConfig;
			rx_thread_start(_rx_thread_antibleeding_plane, (void*)&antibleedingthread[j], 0, "_rx_thread_antibleeding_plane");
			iCntPlane++;
		}

		while (1) {
			rx_sem_wait(hWaitEvent, INFINITE);
			for (h = 0; h < j; h++) {
				if (antibleedingthread[h].running)
					break;
			}
			if (h == j)
				break;
		}
		rx_sem_destroy(&hWaitEvent);

		for (k = 0; k < h; k++)
		{
			if (antibleedingthread[k].ret != REPLY_OK)
				ret = antibleedingthread[k].ret;
		}
	}
	return (ret);
}