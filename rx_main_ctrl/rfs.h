// ****************************************************************************
//
//	rfs.h	Remote File System
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

void rfs_reset(const char *ipAddr);
// void rfs_update(const char *ipAddr, const char *srcBinPath, const char *deviceType, const char *dstPath);
void rfs_update_start(UINT64 macAddr, const char *ipAddr, int port, const char *srcBinPath, const char *deviceType, const char *dstPath, const char *process);
// int RFS_start (const char *ipAddr, const char *process);
