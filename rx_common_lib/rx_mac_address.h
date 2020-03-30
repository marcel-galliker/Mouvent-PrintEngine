// ****************************************************************************
//
//	DIGITAL PRINTING - rx_mac_address.h
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

//	******* HOW TO SET MAC_ADDRESS while board is booting ************************************
//
//	Hit any key to stop autoboot:  0
//	SOCFPGA_ARRIA5 # setenv ethaddr 00:07:ed:01:00:00	//(ALTERA-Head_Ctrl-xxxx)
//	SOCFPGA_ARRIA5 # saveenv
//	Saving Environment to MMC...
//	Writing to MMC(0)... Timeout on data busy
//	Timeout on data busy
//	done
//	SOCFPGA_ARRIA5 # reset
//	resetting ...

#include "rx_def.h"

//--- OUI: Organisation Unique Identifiers ----------
#define MAC_OUI_MASK		mac_as_i64("ff:ff:ff:00:00:00")	
#define MAC_OUI_ALTERA		mac_as_i64("00:07:ed:00:00:00")
#define MAC_OUI_SYSTEGRA	mac_as_i64("00:0b:eb:00:00:00")
#define MAC_OUI_MICROSOFT	mac_as_i64("94:9a:a9:00:00:00")

#define MAC_OUI				MAC_OUI_ALTERA

//--- Device ID --------------------------------------
#define MAC_ID_MASK			mac_as_i64("00:00:00:ff:00:00")
#define MAC_HEAD_CTRL_ARM	mac_as_i64("00:00:00:01:00:00")	// arm processor
#define MAC_HEAD_CTRL_UDP0	mac_as_i64("00:00:00:02:00:00")	// FPGA data interface 1
#define MAC_HEAD_CTRL_UDP1	mac_as_i64("00:00:00:03:00:00")	// FPGA data interface 2
#define MAC_ENCODER_CTRL	mac_as_i64("00:00:00:04:00:00")
#define MAC_FLUID_CTRL		mac_as_i64("00:00:00:05:00:00")
#define MAC_STEPPER_CTRL	mac_as_i64("00:00:00:06:00:00")
#define MAC_ENC32_CTRL	    mac_as_i64("00:00:00:07:00:00")

#define MAC_NO_MASK			mac_as_i64("00:00:00:00:ff:ff")

#define MAC_SIMU_PLC		mac_as_i64("00:07:ee:01:02:03")

void macAddr_udp(int udpNo, int serialNo, UINT64 *macAddr);
