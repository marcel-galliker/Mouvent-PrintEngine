// ********************************************************void ********************
//
//	DIGITAL PRINTING - fpga_msg.h
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "fpga_msg.h"

typedef struct
{
	RX_SOCKET sender;
	union
	{
		SMsgHdr			hdr;
		SFpgaImageCmd	img;
		SBlockUsedCmd	usd;
		SHeadBoardCfg	cfg;
	};
}  SFpgaMsg;

//--- fpga_handle_msg --------------------------------------------------------
void  fpga_handle_msg(RX_SOCKET sender, void *pmsg)
{
	SMsgHdr *phdr = (SMsgHdr*)pmsg;

	switch(phdr->msgId)
	{
		case CMD_FPGA_IMAGE:		_do_fpga_image		(socket, (SFpgaImageCmd*)	 pmsg);	 break;
		case CMD_FPGA_SIMU_ENCODER:	_do_simu_encoder	(socket, (UINT32*)		   &phdr[1]);break;
		case CMD_GET_BLOCK_USED:	_do_block_used		(socket, (SBlockUsedCmd*)	pmsg);	 break;
		case CMD_HEAD_BOARD_CFG:	_do_head_board_cfg	(socket, (SHeadBoardCfg*)  &phdr[1]);break;

	}
}
