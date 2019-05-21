// ****************************************************************************
//
//	DIGITAL PRINTING - icmnat.c
//
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "stdafx.h"
#include "IcmNat.h"

//--- icmnat_init ----------------------------------------------
void icmnat_init (CIcmpNATTable *ptable)
{
	memset(ptable, 0, sizeof(CIcmpNATTable));
	ptable->Table		= malloc(NAT_TABLE_SIZE*sizeof(CIcmpNATEntry*));
	ptable->PortTable	= malloc(NAT_TABLE_SIZE*sizeof(CIcmpNATEntry*));
	memset(ptable->Table,	  0, NAT_TABLE_SIZE*sizeof(CIcmpNATEntry*));
	memset(ptable->PortTable, 0, NAT_TABLE_SIZE*sizeof(CIcmpNATEntry*));
}

//--- icmnat_end ------------------------------------------
void icmnat_end  (CIcmpNATTable *ptable)
{
	icmnat_RemoveAll(ptable);
	free(ptable->Table);
	free(ptable->PortTable);
}

//--- icmnat_Allocate -----------------------------------
CIcmpNATEntry* icmnat_Allocate (CIcmpNATTable *ptable, struct in_addr ip_src, struct in_addr ip_dst, unsigned short icmp_id)
{
	SYSTEMTIME		systime;
	FILETIME		ftime;
	ULARGE_INTEGER	time;

	//Get current time
	GetSystemTime(&systime);
	SystemTimeToFileTime(&systime, &ftime);
	time.HighPart = ftime.dwHighDateTime;
	time.LowPart = ftime.dwLowDateTime;

	// Find first non-allocated or outdated local ID
	for (int i = 2; i < NAT_TABLE_SIZE; ++i)
	{
		if (ptable->PortTable[i] != NULL)
		{
			// Free ID if NAT entry is out-of-date
			if(((time.QuadPart - ptable->PortTable[i]->base.m_ulTimeStamp.QuadPart)/10000000) > NAT_TIMEOUT)
			{
				icmnat_Free(ptable, ptable->PortTable[i]);
			}
		}

		if (ptable->PortTable[i] == NULL)
		{
			// Found free port, allocate and initialize new NAT entry
			CIcmpNATEntry* pNE = malloc(sizeof(CIcmpNATEntry));
			memset(pNE, 0, sizeof(CIcmpNATEntry));
			pNE->base.m_IpSrc.S_un.S_addr = ntohl(ip_src.S_un.S_addr);
			pNE->base.m_IpDst.S_un.S_addr = ntohl(ip_dst.S_un.S_addr);
			pNE->m_usIcmpId = ntohs(icmp_id);
            pNE->m_usNATIcmpId = i;
			pNE->base.m_ulTimeStamp = time;

			ptable->PortTable[i] = pNE;

			unsigned short usIndex = pNE->base.m_IpDst.S_un.S_un_w.s_w1 + pNE->base.m_IpDst.S_un.S_un_w.s_w2;

			pNE->base.nextEntry = (CNATEntry*)ptable->Table[usIndex];
			if(ptable->Table[usIndex])
				ptable->Table[usIndex]->base.prevEntry = (CNATEntry*)pNE;
			ptable->Table[usIndex] = pNE;

			return pNE;
		}
	}

	return NULL;
}

//--- icmnat_Find -------------------------------
CIcmpNATEntry* icmnat_Find     (CIcmpNATTable *ptable, struct in_addr ip_src, struct in_addr ip_dst, unsigned short icmp_id)
{
	unsigned short usIndex = ntohs(ip_dst.S_un.S_un_w.s_w1) + ntohs(ip_dst.S_un.S_un_w.s_w2);
	
	CIcmpNATEntry* pNE = ptable->Table[usIndex];

	while (pNE)
	{
		if ((pNE->base.m_IpSrc.S_un.S_addr == ntohl(ip_src.S_un.S_addr))&&
			(pNE->base.m_IpDst.S_un.S_addr == ntohl(ip_dst.S_un.S_addr))&&
			(pNE->m_usIcmpId == ntohs(icmp_id))
			)
		{
			SYSTEMTIME systime;
			FILETIME ftime;

			// Update timestamp
			GetSystemTime(&systime);
			SystemTimeToFileTime(&systime, &ftime);
			pNE->base.m_ulTimeStamp.HighPart = ftime.dwHighDateTime;
			pNE->base.m_ulTimeStamp.LowPart = ftime.dwLowDateTime;

			return pNE;
		}

		pNE = (CIcmpNATEntry*)pNE->base.nextEntry;
	}
	return pNE;
}

//--- icmnat_Map -------------------------------------------
CIcmpNATEntry* icmnat_Map (CIcmpNATTable *ptable, unsigned short icmp_id)
{
	return ptable->PortTable[ntohs(icmp_id)];
}

//--- icmnat_Free --------------------------------------------
void icmnat_Free(CIcmpNATTable *ptable, CIcmpNATEntry* pNE)
{
	// Deallocate entry resources
	unsigned short usDel = pNE->base.m_IpDst.S_un.S_un_w.s_w1 + pNE->base.m_IpDst.S_un.S_un_w.s_w2;

	if (pNE->base.prevEntry == NULL)
	{
		// Head of the list
		ptable->Table[usDel] = (CIcmpNATEntry*)pNE->base.nextEntry;
		if (ptable->Table[usDel])
			ptable->Table[usDel]->base.prevEntry = NULL;
	}
	else
	{
		// In the middle or at the end of the list
		pNE->base.prevEntry->nextEntry = pNE->base.nextEntry;
		if (pNE->base.nextEntry)
			pNE->base.nextEntry->prevEntry = pNE->base.prevEntry;
	}
				
	
	ptable->PortTable[pNE->m_usNATIcmpId] = NULL;
	free(pNE);
}

//--- icmnat_RemoveAll --------------------------------------------
void icmnat_RemoveAll(CIcmpNATTable *ptable)
{
	for (int i = 0; i < NAT_TABLE_SIZE; ++i)
	{
		if (ptable->PortTable[i] != NULL)
		{
			icmnat_Free(ptable, ptable->PortTable[i]);
		}
	}
}
