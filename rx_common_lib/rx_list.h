// ****************************************************************************
//
//	rx_list.h
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Hubert Zimmermann
//
// ****************************************************************************

#pragma once

#include "rx_common.h"

//--- Structures --------------------------------------------------------------

//--- Prototypes --------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

int		rx_list_create	(HANDLE *handle);
int		rx_list_close	(HANDLE handle);
int		rx_list_count	(HANDLE handle);
int		rx_list_add		(HANDLE handle, void *elem);
int		rx_list_insert	(HANDLE handle, void *elem);
int		rx_list_remove	(HANDLE handle, void *elem);
int		rx_list_select	(HANDLE handle, void *elem);
int		rx_list_elem	(HANDLE handle, void **elem);
int		rx_list_begin	(HANDLE handle, void **elem);
int		rx_list_end		(HANDLE handle, void **elem);
int		rx_list_next	(HANDLE handle, void **elem);
int		rx_list_prev	(HANDLE handle, void **elem);

#ifdef __cplusplus
}
#endif
