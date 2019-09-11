// ****************************************************************************
//
//	DIGITAL PRINTING - tse.c
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************
#ifdef linux
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/mman.h>
	#include <unistd.h>
#endif

#include "rx_error.h"
#include "rx_term.h"
#include "nios_def_head.h"
#include "tse_regs.h"
#include "tse.h"

typedef struct
{
	UINT32 remote_fault;
	UINT32 link_down_fault;
	UINT32 parallel_detect_fault;
	UINT32 receive_error;
	UINT32 flags;
} STseErrors;


//--- module globals ---------------------------------------
static int			_MemId=0;
static UINT32		*_TSE[UDP_PORT_CNT];
static INT32		_Speed[UDP_PORT_CNT];
static STseErrors	_TseErrors[UDP_PORT_CNT];
static UINT64		_MacAddr[UDP_PORT_CNT];
static int			_ErrorDelay=0;
	
//--- tse_init -------------------------------------------------
int tse_init(void *addr0, void *addr1)
{
	_TSE[0] = (UINT32*)addr0;
	_TSE[1] = (UINT32*)addr1;
	memset(_Speed, 0, sizeof(_Speed));
	memset(_TseErrors, 0, sizeof(_TseErrors));
	memset(_MacAddr, 0, sizeof(_MacAddr));
	_ErrorDelay=0;
	return REPLY_OK;
}

//--- tse_end --------------------------------------------
int tse_end(void)
{
	close (_MemId);
	_MemId = 0;
	return REPLY_OK;
}

//--- tse_error_reset -----
void tse_error_reset(void)
{
	int i;
	for (i=0; i<UDP_PORT_CNT; i++)
		_TseErrors[i].flags=0;
}

//--- tse_set_mac_addr -----------------------
int tse_set_mac_addr(int no, UINT64 macAddr)
{
	UINT32		rx_clk, tx_clk, rgmii_clk;
	UINT32		rx_dv,tx_en,rgmii_ctr_sig;
	int			time;

	if (no<0 || no>=SIZEOF(_TSE) || _TSE[no]==NULL) return REPLY_ERROR;
	if (macAddr==_MacAddr[no]) return REPLY_OK;
	
	_ErrorDelay = 10;
	
	_TSE[no][TSE_PHY_BASIC_CONTROL] = 0x09140;			// Basic control	-> Software Reset
	
	// Config Auto-Negotiation Advertisement
	// we do NOT support: 
	//					T4
	//					100Mbps half- and 100MBs full duplex
	//					10Mbbps half- and 10MBs full duplex
	//
	// only 1000Mbps full duplex is supported!
	
	_TSE[no][TSE_PHY_AUTO_NEG_ADVERTISEMENT]	= 0x01;		
	
	_TSE[no][TSE_PHY_BASIC_CONTROL] = 0x0140;			// Basic control	-> disable Auto-negotiation, Full-Duplex, 1000Mbps
	
	_TSE[no][TSE_MDIO_ADDR_0] = PHY_ADDRESS;			// Bits[4:0] 		—> 5-bit PHY address

	// RGMII Control Signal Pad Skew
	rx_dv = 0x07;		// default (0.0ns) 4bit Reg
	tx_en = 0x07;		// default (0.0ns) 4bit Reg

	// RGMII Control Signal Pad Skew
	rgmii_ctr_sig = (rx_dv << 4) + tx_en;

	_TSE[no][TSE_PHY_MMD_ACCESS_CONTROL]		= 0x00000002;
	_TSE[no][TSE_PHY_MMD_ACCESS_REGISTER_DATA]	= 0x00000004;
	_TSE[no][TSE_PHY_MMD_ACCESS_CONTROL]		= 0x00004002;
	_TSE[no][TSE_PHY_MMD_ACCESS_REGISTER_DATA]  = rgmii_ctr_sig;

	// RGMII Rx Data Pad Skew
	_TSE[no][TSE_PHY_MMD_ACCESS_CONTROL]		= 0x00000002;
	_TSE[no][TSE_PHY_MMD_ACCESS_REGISTER_DATA]  = 0x00000005;
	_TSE[no][TSE_PHY_MMD_ACCESS_CONTROL]		= 0x00004002;
	_TSE[no][TSE_PHY_MMD_ACCESS_REGISTER_DATA]  = 0x00007777; 	// default (+-0ns) 4bit

	// RGMII Tx Data Pad Skew
	_TSE[no][TSE_PHY_MMD_ACCESS_CONTROL]		= 0x00000002;
	_TSE[no][TSE_PHY_MMD_ACCESS_REGISTER_DATA]  = 0x00000006;
	_TSE[no][TSE_PHY_MMD_ACCESS_CONTROL]		= 0x00004002;
	_TSE[no][TSE_PHY_MMD_ACCESS_REGISTER_DATA]	= 0x00007777;	// default (+-0ns) 4bit

	// RGMII Clock Pad Skew
	rx_clk = 0x0f;			// default (+-0ns) 5bit
	tx_clk = 0x00;			// -0.9ns
	
	rgmii_clk = (tx_clk << 5) + rx_clk;

	_TSE[no][TSE_PHY_MMD_ACCESS_CONTROL]		= 0x00000002;
	_TSE[no][TSE_PHY_MMD_ACCESS_REGISTER_DATA]	= 0x00000008;
	_TSE[no][TSE_PHY_MMD_ACCESS_CONTROL]		= 0x00004002;
	_TSE[no][TSE_PHY_MMD_ACCESS_REGISTER_DATA]	= rgmii_clk;

	//TSE[no][TSE_PHY_REMOTE_LOOPBACK,0x01f4);				// enable analog loopback
	//TSE[no][TSE_PHY_REMOTE_LOOPBACK,0x00f4);				// disable analog loopback

	/*
	// Read back pad skew registers
	_TSE[no][TSE_PHY_MMD_ACCESS_CONTROL]		= 0x00000002;
	_TSE[no][TSE_PHY_MMD_ACCESS_REGISTER_DATA]	= 0x00000008;
	_TSE[no][TSE_PHY_MMD_ACCESS_CONTROL]		= 0x00004002;
	val = _TSE[no][TSE_PHY_MMD_ACCESS_REGISTER_DATA];
	arm_ptr->stat.rgmii_clk_pad_skeq[interface_nr]=val;
	*/

	/*
	_TSE[no][TSE_PHY_MMD_ACCESS_CONTROL]		= 0x00000002;
	_TSE[no][TSE_PHY_MMD_ACCESS_REGISTER_DATA]	= 0x00000004;
	_TSE[no][TSE_PHY_MMD_ACCESS_CONTROL]		= 0x00004002;
	val = IORD_32DIRECT(base,TSE_PHY_MMD_ACCESS_REGISTER_DATA);
	arm_ptr->stat.rgmii_tx_en_pad_skew[interface_nr]=val;
	*/
	/*
	_TSE[no][TSE_PHY_MMD_ACCESS_CONTROL]		= 0x00000002;
	_TSE[no][TSE_PHY_MMD_ACCESS_REGISTER_DATA]	= 0x00000006;
	_TSE[no][TSE_PHY_MMD_ACCESS_CONTROL]		= 0x00004002;
	val = IORD_32DIRECT(base,TSE_PHY_MMD_ACCESS_REGISTER_DATA);
	arm_ptr->stat.rgmii_tx_data_pad_skew[interface_nr]=val;
	*/
	
	// TSE Initialisation
	_TSE[no][TSE_COMMAND_CONFIG] = 0x00002000;			// Software reset RX / TX off

	time = rx_get_ticks();
	while(_TSE[no][TSE_COMMAND_CONFIG] & 0x0003)
	{
		if (rx_get_ticks()-time>1000) 
			return REPLY_ERROR;
	};										// wait RX and TX off

	_TSE[no][TSE_TX_SECTION_EMPTY]	= 0x30;
	_TSE[no][TSE_TX_ALMOST_FULL]	= 0x03;
	_TSE[no][TSE_TX_ALMOST_EMPTY]	= 0x08;

	_TSE[no][TSE_RX_SECTION_EMPTY]	= 0x30;
	_TSE[no][TSE_RX_ALMOST_FULL]	= 0x08;
	_TSE[no][TSE_RX_ALMOST_EMPTY]	= 0x08;

	_TSE[no][TSE_TX_SECTION_FULL]	= 0x10;
	_TSE[no][TSE_RX_SECTION_FULL]	= 0x10;
		
	_TSE[no][TSE_MAC_0] = macAddr & 0xffffffff;
	_TSE[no][TSE_MAC_1] = (macAddr>>32) & 0xffffffff;
	
	// Set Frame-Length for TSE in FPGA
	// Options:
	// udp-block-lenght	 TSE-Frame-Length-Config (+78)
	// dec  dec  hex
	// 1440 1518 0x000005EE (default/reset)
	// 2880 2958 0x00000B8E
	// 5760 5838 0x000016CE
	// 8640 8718 0x0000220E
	_TSE[no][TSE_FRM_LENGTH]		= 0x0000220E;

	_TSE[no][TSE_COMMAND_CONFIG]	= 0x00002000;	// Software Reset

	time = rx_get_ticks();
	while(_TSE[no][TSE_COMMAND_CONFIG] & 0x2000)
	{
		if (rx_get_ticks()-time>1000) 
			return REPLY_ERROR;
	};

	_TSE[no][TSE_COMMAND_CONFIG] = (TX_ENA + RX_ENA + ETH_SPEED);	// 1GB

	time = rx_get_ticks();
	while((_TSE[no][TSE_COMMAND_CONFIG] & 0x0003)!=0x0003)										// wait RX and TX on
	{
		if (rx_get_ticks()-time>1000) 
			return REPLY_ERROR;		
	};

	_MacAddr[no] = macAddr;
	
	return REPLY_OK;
}

//--- tse_display_status --------------------------------------
void tse_display_status(INT32 speed[2])
{		
	int i, mb[2];
	for (i=0; i<2; i++)
		mb[i]=speed[i]*RX_HBConfig.dataBlkSize/1024;
	term_printf("UDP speed:       % 4d  % 4d     msg/s %06d %06d	     MB/s %7s %7s\n", _Speed[0], _Speed[1], speed[0], speed[1], value_str3(mb[0]), value_str3(mb[1]));
}

//--- tse_check_errors ----------------------------
int tse_check_errors(int menu)
{
	int i;		
	for (i=0; i<SIZEOF(_TSE); i++)
	{
		if (_TSE[i] && _MacAddr[i])
		{
			if (_TSE[i][TSE_PHY_BASIC_STATUS] & LINK_STATUS)
			{
				if (_TSE[i][TSE_PHY_CONTROL] & SPEED_STATUS_1000) _Speed[i]=1000;
				else                                              _Speed[i]=100;
			}
			else _Speed[i]=0;
			
			_TseErrors[i].remote_fault			= (_TSE[i][TSE_PHY_INTERRUPT_CONTROL_STATUS] & REMOTE_FAULT_OCCURED);
			_TseErrors[i].link_down_fault		= (_TSE[i][TSE_PHY_INTERRUPT_CONTROL_STATUS] & LINK_DOWN_OCCURED);
			_TseErrors[i].parallel_detect_fault = (_TSE[i][TSE_PHY_INTERRUPT_CONTROL_STATUS] & PARALLEL_DETECT_OCCURED);
			_TseErrors[i].receive_error			= (_TSE[i][TSE_PHY_INTERRUPT_CONTROL_STATUS] & RECEIVE_ERROR_OCCURED);
		}
	}
	
	if (_ErrorDelay==0)
	{
		for (i=0; i<SIZEOF(_TSE); i++)
		{
//			if (_TseErrors[i].remote_fault)				ErrorFlag(ERR_CONT, &_TseErrors[i].flags, 0x00, 0, "TSE: Remote fault occurred on UDP %d", i);
			if (_TseErrors[i].link_down_fault)			ErrorFlag(ERR_CONT, &_TseErrors[i].flags, 0x01, 0, "TSE: Link-down occurred on UDP %d", i);
			if (_TseErrors[i].parallel_detect_fault)	ErrorFlag(ERR_CONT, &_TseErrors[i].flags, 0x02, 0, "TSE: Parallel detect fault occurred on UDP %d", i);
			if (_TseErrors[i].receive_error)			ErrorFlag(ERR_CONT, &_TseErrors[i].flags, 0x04,	0, "TSE: Receive error occurred on UDP %d", i);
				
			if (_Speed[i]==0)	ErrorFlag(ERR_CONT, &_TseErrors[i].flags, 0x08, 0, "UDP %d not connected", i);
			if (_Speed[i]==100) ErrorFlag(ERR_CONT, &_TseErrors[i].flags, 0x10, 0, "UDP %d only at 100 Mbit/s (needs 1 Gbit/s)", i);
				
			if (RX_HBConfig.printerType==printer_DP803)
			{
				if (_Speed[i]==1000 && _TseErrors[i].flags&0x08)
				{
					Error(LOG, 0, "UDP %d reconnected", i);
					_TseErrors[i].flags &= ~0x08;					
				}
			}				
		}
	}
	
	if(menu && _ErrorDelay>0) _ErrorDelay--;
	return REPLY_OK;
}
