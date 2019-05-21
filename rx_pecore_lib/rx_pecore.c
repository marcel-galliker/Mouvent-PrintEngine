// ****************************************************************************
//
//	rx_pecore.c
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- Includes ----------------------------------------------------------------
#include "rx_pecore.h"

//--- Locals ------------------------------------------------------------------
static int _trace=FALSE;

//--- Publics -----------------------------------------------------------------
WriteTextCallback WriteText = NULL;

// ****************************************************************************

//--- pe_send_msg -------------------------------------------------------------
int	pe_send_msg(RX_SOCKET *socket, void *msg)
{
	Smvt_prt_header *hdr = (Smvt_prt_header*)msg;

	pe_printf(_trace, "Send msg 0x%X (%d) - len %d", hdr->id, (hdr->id & ~MVT_CORE_INTERNAL), hdr->length);

	return sok_send_wide(socket, msg);
}

// ****************************************************************************
// *			P U B L I C S
// ****************************************************************************

//--- rx_pewrite_callback -----------------------------------------------------
void rx_pewrite_callback(WriteTextCallback writetext)
{
	WriteText = writetext;
}

