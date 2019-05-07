// ****************************************************************************
//
//	DIGITAL PRINTING - portnat.c
//
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "stdafx.h"
#include "PortNat.h"

//--- icmnat_init ----------------------------------------------
void portnat_init (CPortNATTable *ptable)
{
	memset(ptable, 0, sizeof(CPortNATTable));
	ptable->Table		= malloc(NAT_TABLE_SIZE*sizeof(CPortNATTable*));
	ptable->PortTable	= malloc(NAT_TABLE_SIZE*sizeof(CPortNATTable*));
	memset(ptable->Table,	  0, NAT_TABLE_SIZE*sizeof(CPortNATTable*));
	memset(ptable->PortTable, 0, NAT_TABLE_SIZE*sizeof(CPortNATTable*));
}

//--- icmnat_end ------------------------------------------
void portnat_end  (CPortNATTable *ptable)
{
	portnat_RemoveAll(ptable);
	free(ptable->Table);
	free(ptable->PortTable);
}

//--- portnat_Allocate -----------------------------------
CPortNATEntry* portnat_Allocate (CPortNATTable *ptable, struct in_addr ip_src, unsigned short port_src, struct in_addr ip_dst, unsigned short port_dst)
{
	SYSTEMTIME		systime;
	FILETIME		ftime;
	ULARGE_INTEGER	time;

	//Get current time
	GetSystemTime(&systime);
	SystemTimeToFileTime(&systime, &ftime);
	time.HighPart = ftime.dwHighDateTime;
	time.LowPart = ftime.dwLowDateTime;

	// Find first non-allocated or outdated local port
	for (int i = 10000; i < NAT_TABLE_SIZE; ++i)
	{
		if (ptable->PortTable[i] != NULL)
		{
			// Free port if NAT entry is out-of-date
			if(((time.QuadPart - ptable->PortTable[i]->base.m_ulTimeStamp.QuadPart)/10000000) > NAT_TIMEOUT)
			{
				portnat_Free(ptable, ptable->PortTable[i]);
			}
		}

		if (ptable->PortTable[i] == NULL)
		{
			// Found free port, allocate and initialize new NAT entry
			CPortNATEntry* pNE = malloc(sizeof(CPortNATEntry));
			memset(pNE, 0, sizeof(CPortNATEntry));
			pNE->base.m_IpSrc.S_un.S_addr = ntohl(ip_src.S_un.S_addr);
			pNE->base.m_IpDst.S_un.S_addr = ntohl(ip_dst.S_un.S_addr);
			pNE->m_usDstPort = ntohs(port_dst);
            pNE->m_usNATPort = i;
			pNE->m_usSrcPort = ntohs (port_src);
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

//--- portnat_Find ------------------------------------------
CPortNATEntry* portnat_Find (CPortNATTable *ptable, struct in_addr ip_src, unsigned short port_src, struct in_addr ip_dst, unsigned short port_dst)
{
	unsigned short usIndex = ntohs(ip_dst.S_un.S_un_w.s_w1) + ntohs(ip_dst.S_un.S_un_w.s_w2);
	
	CPortNATEntry* pNE = ptable->Table[usIndex];

	while (pNE)
	{
		if ((pNE->base.m_IpSrc.S_un.S_addr == ntohl(ip_src.S_un.S_addr))&&
			(pNE->base.m_IpDst.S_un.S_addr == ntohl(ip_dst.S_un.S_addr))&&
			(pNE->m_usDstPort == ntohs(port_dst))&&
			(pNE->m_usSrcPort == ntohs(port_src))
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

		pNE = (CPortNATEntry*)pNE->base.nextEntry;
	}

	return pNE;
}

//--- portnat_Map -------------------------------------------
CPortNATEntry* portnat_Map (CPortNATTable *ptable, unsigned short port_dst)
{
	return ptable->PortTable[ntohs(port_dst)];
}

//--- portnat_Free -------------------------------------------------
void portnat_Free(CPortNATTable *ptable, CPortNATEntry* pNE)
{
	// Deallocate entry resources
	unsigned short usDel = pNE->base.m_IpDst.S_un.S_un_w.s_w1 + pNE->base.m_IpDst.S_un.S_un_w.s_w2;

	if (pNE->base.prevEntry == NULL)
	{
		// Head of the list
		ptable->Table[usDel] = (CPortNATEntry*)pNE->base.nextEntry;
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
				
	
	ptable->PortTable[pNE->m_usNATPort] = NULL;
	free(pNE);
}

//--- portnat_RemoveAll -----------------------------------------------------
void portnat_RemoveAll(CPortNATTable *ptable)
{
	for (int i = 0; i < NAT_TABLE_SIZE; ++i)
	{
		if (ptable->PortTable[i] != NULL)
		{
			portnat_Free(ptable, ptable->PortTable[i]);
		}
	}
}
