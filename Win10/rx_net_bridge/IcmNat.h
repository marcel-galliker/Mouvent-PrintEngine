// ****************************************************************************
//
//	DIGITAL PRINTING - icmnat.h
//
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "PortNat.h"
// Describes single ICMP NAT entry

typedef struct CIcmpNATEntry
{
	CNATEntry	base;
	unsigned short m_usIcmpId;		// Original ICMP ID
	unsigned short m_usNATIcmpId;	// NAT ICMP ID
} CIcmpNATEntry;

// TCP/UDP NAT table
typedef struct CIcmpNATTable
{
	// Hash table for TCP connections
	CIcmpNATEntry		**Table; // Used for allocating and searching NAT entries for outgoing packets
	CIcmpNATEntry		**PortTable; // Used for mapping incoming packets to NAT entries
} CIcmpNATTable;

void icmnat_init (CIcmpNATTable *ptable);
void icmnat_end  (CIcmpNATTable *ptable);


// Outgoing NAT methods
CIcmpNATEntry* icmnat_Allocate (CIcmpNATTable *ptable, struct in_addr ip_src, struct in_addr ip_dst, unsigned short icmp_id);
CIcmpNATEntry* icmnat_Find     (CIcmpNATTable *ptable, struct in_addr ip_src, struct in_addr ip_dst, unsigned short icmp_id);

// Incoming NAT methods
CIcmpNATEntry* icmnat_Map (CIcmpNATTable *ptable, unsigned short icmp_id);

// Deleting entries methods
void icmnat_Free(CIcmpNATTable *ptable, CIcmpNATEntry* pNE);
void icmnat_RemoveAll(CIcmpNATTable *ptable);
