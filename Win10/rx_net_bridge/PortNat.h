// ****************************************************************************
//
//	DIGITAL PRINTING - portnat.h
//
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#define NAT_TABLE_SIZE 256*256 // Size of NAT table
#define NAT_TIMEOUT 600 //This is timeout in seconds for which we keep the inactive entry in the NAT table until removal (10 minutes)
#define MAX_STRING 512

typedef struct CNATEntry
{
	struct in_addr			m_IpSrc;	// Source IP address
	struct in_addr			m_IpDst;	// Destination IP address
	struct in_addr			m_IpNAT;	// IP for NAT

	ULARGE_INTEGER	m_ulTimeStamp; // Last packet this entry was applied to
 
	struct CNATEntry*		prevEntry; // Previous NAT entry
	struct CNATEntry*		nextEntry; // Next NAT entry
} CNATEntry;

// Describes single UDP/TCP NAT entry
typedef struct CPortNATEntry
{
	CNATEntry	   base;
	unsigned short m_usSrcPort; // Source port address
	unsigned short m_usDstPort; // Destination port address
	unsigned short m_usNATPort; // Port for NAT
} CPortNATEntry;

// TCP/UDP NAT table
typedef struct CPortNATTable
{
	// Hash table for TCP connections
	CPortNATEntry		**Table; // Used for allocating and searching NAT entries for outgoing packets
	CPortNATEntry		**PortTable; // Used for mapping incoming packets to NAT entries
} CPortNATTable;

void portnat_init (CPortNATTable *ptable);
void portnat_end  (CPortNATTable *ptable);

// Outgoing NAT methods
CPortNATEntry* portnat_Allocate (CPortNATTable *ptable, struct in_addr ip_src, unsigned short port_src, struct in_addr ip_dst, unsigned short port_dst);
CPortNATEntry* portnat_Find     (CPortNATTable *ptable, struct in_addr ip_src, unsigned short port_src, struct in_addr ip_dst, unsigned short port_dst);

// Incoming NAT methods
CPortNATEntry* portnat_Map (CPortNATTable *ptable, unsigned short port_dst);

// Deleting entries methods
void portnat_Free(CPortNATTable *ptable, CPortNATEntry* pNE);
void portnat_RemoveAll(CPortNATTable *ptable);
