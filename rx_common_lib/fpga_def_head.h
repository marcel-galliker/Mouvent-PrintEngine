// ****************************************************************************
//
//	DIGITAL PRINTING - fpga_def_head.h
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_common.h"
#include "rx_def.h"

//*** main address mapping *****************************************************

// #define FPGA_BASE_ADDR	0xC0040000UL
#define FPGA_BASE_ADDR	0xFF240000UL

// #define FPGA_CFG_ADDR		(FPGA_BASE_ADDR + 0x00000000UL)	// SFpgaCfg

//*** structures ****************************************************************
#define MAX_HEADS_BOARD		4

#define MAX_JETS_HEAD		2048	// 5312 for Kyocera printhead
#define HEAD_WIDTH_SAMBA	2048
#define HEAD_OVERLAP_SAMBA	128
#define MAX_GREY_LEVELS		8

#define MAX_ETH_PORT		2

//=== HEAD CONFIGURATION =============================================================


//--- Jet assignment -------------------------------------------------
//	Assignment for Dimatix Gemini
//	-----------------------------
//	Heads: 			4
//	Jets/Head:		2048
//	Bit/Pixel:		2
//	max offsetFS:	609
//	Memory usage:	9.6MBit, 1.2MB
//
//	Assignment for Kyocera
//	-----------------------------
//	Heads: 			1
//	Jets/Head:		5312
//	Bit/Pixel:		3
//	max offsetFS:	1520
//	Memory usage:	23.1MBit, 2.9MB

//typedef struct SJetAssignment
//{
//	UINT32	offsetFS;	// jet offset in "firepulse strokes (FS)"
//	UINT32	regNo;		// number of shift-register (each 64 bits)
//	UINT32	bitNo;		// bit inside shift-register
//} SJetAssignment;

//--- Firepulse definitions ---------------------------------------
typedef struct SFirePulse
{
	UINT32  allon;			// position of the ALL-ON signal 
	UINT32	voltageCnt;		// number of voltages (<4096), values are written to NIOS
	UINT32	wave_start[8];	// max 8 values
	UINT32	wave_end[8];	// max 8 values
} SFirePulse;

/*
//--- speed compensation ------------------------------------------
typedef struct SSpeedComp
{
	// speed compensation of the print-go signal:
	// at speed "maxSpeed" the print-go is "maxComp" early to the position at speed "0"

	UINT32			maxCompFS;	// in firepulse strokes (FS)
	UINT32			maxSpeed;	// in FS/sec
} SSpeedComp;
*/

//--- 
typedef struct SFpgaMainCfg
{
	UINT32			head_type;	// 0x0000
						#define HEAD_TYPE_UNDEF		0
						#define HEAD_TYPE_GEMINI	1
						#define HEAD_TYPE_KYOCERA	2
	
	UINT32 err_reset;
		#define ERR_ETH				0x00000001	// 0x0004[0]:
		#define	ERR_DATA_AQUISITION 0x00000002	// 0x0004[1]: data aquisition
		#define	ERR_DDR3_ARBITER	0x00000004	// 0x0004[2]: ddr3 arbiter
		#define	ERR_SPEED			0x00000008	// 0x0004[3]: 
		#define	ERR_ENCODER			0x00000010  // 0x0004[4]: encoder speed
		#define ERR_PRINT_DATA		0x00000020	// 0x0004[5]: data print ctrl counters
		#define CTR_DROPS			0x00000040	// 0x0004[6]: shooting jet counters
		#define ERR_HEAD			0x00000080  // 0x0004[6]: shooting head errors

	UINT32			overheat_treshold;			// 0x0008: when head reaches this height it switches off clocks (add 128)
	UINT32			res_0c;		// 0x000c:
	UINT32			res_10;		// 0x0010:
	UINT32			res_14;		// 0x0014:
	UINT32			res_18;		// 0x0018:
	UINT32			res_1c;		// 0x001c:
	UINT32			res_20;		// 0x0020:
	UINT32			res_24;		// 0x0024:
	UINT32			res_28;		// 0x0028:
	UINT32			res_2c;		// 0x003c:
	UINT32			res_30;		// 0x0030:
	UINT32			res_34;		// 0x0004:
	UINT32			res_38;		// 0x0038:
	UINT32			res_3c;		// 0x003c:

	UINT32			cmd;
		#define CMD_MASTER_ENABLE	0x00000001	// 0x0040[0]:
		#define CMD_DDR3_DEBUG		0x00000002	// 0x0040[1]:
		#define CMD_HEAD_DATA_DEBUG	0x00000004	// 0x0040[2]:
		#define CMD_BIT3			0x00000008	// 0x0040[3]:
		#define CMD_BIT4			0x00000010	// 0x0040[4]:
		#define CMD_BIT5			0x00000020	// 0x0040[5]:
		#define CMD_BIT6			0x00000040	// 0x0040[6]:
		#define CMD_BIT7			0x00000080	// 0x0040[7]:
		#define CMD_NIOS_RESET		0x00000100	// 0x0040[8]:

	UINT32			res_44_7f[(0x80-0x44)/4];
	UINT32			led[8];						// 0x0080 .. 0x009f
} SFpgaMainCfg;						   

//--- Head Configuration -----------------------------------------------
typedef struct SFpgaHeadCfg
{
	UINT32			cmd_enable;				// 0x1000: c_cfg_hx_cmd_en_addr
	UINT32			cmd_err_reset;			// 0x1004: c_cfg_hx_cmd_err_rst_addr
	UINT32			cmd_ink_ctr_reset;		// 0x1008: c_cfg_hx_cmd_ink_ctr_rst_addr
	UINT32			cmd_led;				// 0x100c:
	UINT32			firstJetUsed;			// 0x1010: disableJetsLeft
	UINT32			firstJetUnused;			// 0x1014: disableJetsRight
	UINT32			cmd_06;					// 0x1018:
	UINT32			cmd_07;					// 0x101c:
	UINT32			cmd_08;					// 0x1020:
	UINT32			cmd_09;					// 0x1024:
	UINT32			cmd_10;					// 0x1028:
	UINT32			cmd_11;					// 0x102c:
	UINT32			cmd_12;					// 0x1030:
	UINT32			cmd_13;					// 0x1034:
	UINT32			cmd_14;					// 0x1038:
	UINT32			cmd_15;					// 0x103c:

	UINT32			fp_subPulses;			// 0x1040: c_cfg_hx_fp_no_per_seq_addr 
	UINT32			fp_length;				// 0x1044: c_cfg_hx_time_fp_seq_length_addr  	
	UINT32			fp_allOn;				// 0x1048: c_cfg_hx_time_allon_on_addr    
	struct									// 0x104C .. 0x1088
	{
		UINT32		on;			
		UINT32		off;
	}	fp[8];
	UINT32			res_8C;					// 0x108C:
	UINT32			res_90;					// 0x1090:
	UINT32			res_94;					// 0x1094:
	UINT32			res_98;					// 0x1098:
	UINT32			res_9C;					// 0x109C:
	UINT32			res_A0;					// 0x10A0:
	UINT32			res_A4;					// 0x10A4:
	UINT32			res_A8;					// 0x10A8:
	UINT32			res_AC;					// 0x10AC:
	UINT32			res_B0;					// 0x10B0:
	UINT32			res_B4;					// 0x10B4:
	UINT32			res_B8;					// 0x10B8:
	UINT32			res_BC;					// 0x10BC:

	UINT32			gl_2_pulse[8];			// 0x10C0 .. 0x10DC:  c_cfg_hx_gl_to_subplsx_addr
	UINT32			encoderNo;				// 0x10E0: c_cfg_h0_encoder_no_addr
	UINT32			res_E4;					// 0x10E4: c_cfg_h0_substrokes_addr
	UINT32			offset_substroke;		// 0x10E8: c_cfg_h0_fp_offset_fss_addr offset fire-sub-stroke
	UINT32			offset_stroke;			// 0x10EC: c_cfg_h0_fp_offset_fs_addr
	UINT32			res_F0;					// 0x10F0:
	UINT32			res_F4;					// 0x10F4:
	UINT32			res_F8;					// 0x10F8:
	UINT32			res_FC;					// 0x10FC:
	
	UINT32			speedComp_MaxFs;		// 0x1100: c_cfg_h0_speedcmp_max_comp_fs_addr
	UINT32			speedComp_MaxSpeed;		// 0x1104: c_cfg_h0_speedcmp_max_speed_addr 
} SFpgaHeadCfg;

typedef struct SFpgaUdpCfg
{
	struct                                  // 0x7000..0x7001c								
	{
		UINT32			blkNo0;				// first data block number
		UINT32			blkNoEnd;			// last data block number
	} block[MAX_HEADS_BOARD];
	UINT32			udp_block_size;			// 0x7020: index (0..3) to RX_UdpBlockSize
} SFpgaUdpCfg;
extern UINT32 RX_UdpBlockSize[4]; // defined in rx_common.c

//--- Ethernet ports ------------------------------------------
typedef struct SFpgaEthCfg
{
	UINT32			ipAddr;				// 	0x0000: ip address
	UINT32			ipPort;				//	0x0004: ip port
	UINT32			macAddressL;		//	0x0008:
	UINT32			macAddressH;		//	0x000C:
										//  0x0010:
	UINT32			arp_replyReady:1;	// 
	UINT32			arp_requestDone:1;	

	UINT32			res_0014_0400[(0x0400-0x0014)/4];	// 0x0014 ...
	UINT32			arp_reply_buf[16];					// 0x0400
	UINT32			res_0440_0600[(0x0600-0x0440)/4];	// 0x0440 ...
	UINT32			arp_request_buf[16];				// 0x0600
} SFpgaEthCfg;

//--- SFpgaEncoderCfg ----------------------------------------------
typedef struct SFpgaEncoderCfg
{
	UINT32 cmd;
		#define ENC_SIGNAL_MODE		0x00000001
		#define ENC_BIT_1			0x00000002
		#define ENC_BIT_2			0x00000004
		#define ENC_BIT_3			0x00000008
		#define ENC_HEAD0_ENABLE	0x00000010
		#define ENC_HEAD1_ENABLE	0x00000020
		#define ENC_HEAD2_ENABLE	0x00000040
		#define ENC_HEAD3_ENABLE	0x00000080
		#define ENC_ENABLE			0x80000000

	UINT32	dist_pm;				// 0x0004: increment distance in pm
	
	struct
	{
		UINT32	value:28;		// 0x0008[0-27];
		UINT32  bit_28_30:3;
		UINT32	enable:1;		// 0x0008[31]:
	} synth;
	UINT32 shake_interval;			// 0x000c:
	UINT32 manual_printGo;			// 0x0010: triggers on change of bit 0
} SFpgaEncoderCfg;

//--- SFpgaHeadBoardCfg ------------------------------------------------
typedef struct SFpgaHeadBoardCfg
{
	SFpgaMainCfg	*cfg;

	SFpgaHeadCfg  	*head[MAX_HEADS_BOARD];
	SFpgaEthCfg		*eth[MAX_ETH_PORT];
	
	SFpgaUdpCfg		*udp;

	SFpgaEncoderCfg *encoder;
} SFpgaHeadBoardCfg;

//===  PRINT DATA DEFINITION ======================================================================

#define MAX_UDP_MSG_SIZE		1472					// for simulation, see SFpgaEthCfg for real system
														// multiples of 256 Bit
#define DATA_BLOCK_SIZE_STD		(45*32)					// used for standard ip blocks (1440 Bytes)
#define DATA_BLOCK_SIZE_2		(90*32)					// used for standard ip blocks (2880 Bytes)
#define DATA_BLOCK_SIZE_3		(180*32)				// used for standard ip blocks (5760 Bytes)
#define DATA_BLOCK_SIZE_JUMBO	(270*32)				// used for jumbo frame ip blocks (8640 Bytes)

#define DATA_BLOCK_CNT_			(5000*8)				// for simulation, see SFpgaEthCfg for real system
#define DATA_BLOCK_MEMSIZE		(0x20000-0x08000)

//	the print data memory is organized as
//	UINT8	printData[DATA_BLOCK_CNT][DATA_BLOCK_SIZE];
//	- each UDP message sends one data block.
//  - when it is received the corresponding used bit is set.
//	- when it is printed its used flag is cleared.
//  - the printing algorithm checks that the corresponding used flag is set. If not it signals an error and stops printing.

//--- SFpgaUdpMsg --------------------------------------------------------------
typedef struct SFpgaUdpMsg
{
	UINT32	blkNo;							// blockNo==0xffffffff: reset all used flags
	UINT8	blkData[DATA_BLOCK_SIZE_JUMBO];	// block size variable! (see SFpgaEthCfg)
} SFpgaUdpMsg;

//--- SFpgaImage ---------------------------------------------------------------
typedef struct SFpgaImage
{
	UINT32	bitPerPixel;		// 0x0000:	1, 2 (4)
	UINT32	blkNo;				// 0x0004: starting block no
	UINT32	widthPx;			// 0x0008: height in pixels, used to cut pixels
	UINT32	widthBytes;			// 0x000c: height in bytes, used to update mem-pointer
	UINT32	lengthPx;			// 0x0010: length in pixels
	INT32	jetPx0;				// 0x0014: Jet number to witch Pixel 0 is mapped (range [-7..2048+127])
	UINT32	flags;				// 0x0018: Bit 0: 0=forward, 1=backward
								//		   Bit 1: Bidirectional	
	UINT32	flipHorizontal;		// 0x001c:
	UINT32	clearBlockUsed;		// 0x0020: 1=clear the flag
	UINT32  blkCnt;				// 0x0024: unused in FPGA
} SFpgaImage;

//--- SFpgaImageList ----------------------------------------------------------
typedef struct SFpgaImageList
{
	//-- set by ARM processor
	//--- used by FPGA
	UINT32		inIdx;			// index where next image will be set
	UINT32		outIdx;			// index of image that will be printed on next PrintGo
	SFpgaImage	image[256];		// is filled by ARM processor
} SFpgaImageList;

#define IMAGE_LIST_SIZE	128


typedef struct SFpgaPrintList
{
	UINT32	imgInIdx[MAX_HEADS_BOARD];	// 0x20000 .. 0x2000c:	
	
	UINT32	res_10;				// 0x20010:
	UINT32	res_14;				// 0x20014:
	UINT32	res_18;				// 0x20018:
	UINT32	res_1c;				// 0x2001c:
	UINT32	res_20;				// 0x20020:
	UINT32	res_24;				// 0x20024:
	UINT32	res_28;				// 0x20028:
	UINT32	res_2c;				// 0x2002c:
	UINT32	res_30;				// 0x20030:
	UINT32	res_34;				// 0x20034:
	UINT32	res_38;				// 0x20038:
	UINT32	res_3c;				// 0x2003c:

	SFpgaImage image[MAX_HEADS_BOARD];		// 0x20040 .. 0x20064: image[0]
											// 0x20068 .. 0x2008C: image[1]
											// 0x20090 .. 0x200B4: image[2]
											// 0x200B8 .. 0x200DC: image[3]

} SFpgaPrintList;

//--- SFpgaPrintGoList -------------------------------------------------------
typedef struct SFpgaPrintGoList
{
	//-- set by FPGA
	//-- used by FPGA
	UINT32		inIdx;			// index where next value will  be set
	UINT32		outIdx;			// index of next PrintGo to be fired
	UINT32		pos[256];		// positions
} SFpgaPrintGoList;

//--- SFpgaPrintData --------------------------------------------------------
typedef struct SFpgaPrintData
{
	SFpgaImageList		imageList	[MAX_HEADS_BOARD];
	SFpgaPrintGoList	pg			[MAX_HEADS_BOARD];
} SFpgaPrintData;

//=== HEAD STATUS =====================================================

//--- Info Flags --------------------------------------------------
typedef union SFpgaHeadInfo
{
	struct
	{
		UINT32	head_req_fsm_0_running:1;	// 0x00000001
		UINT32  head_req_fsm_1_running:1;	// 0x00000002
		UINT32	head_req_fsm_2_running:1;	// 0x00000004
		UINT32  head_req_fsm_3_running:1;	// 0x00000008
		UINT32	main_fsm_0_running:1;		// 0x00000010
		UINT32	main_fsm_1_running:1;		// 0x00000020
		UINT32	main_fsm_2_running:1;		// 0x00000040
		UINT32	main_fsm_3_running:1;		// 0x00000080
		UINT32	head_fsm_0_running:1;		// 0x00000100
		UINT32  head_fsm_1_running:1;		// 0x00000200
		UINT32	head_fsm_2_running:1;		// 0x00000400
		UINT32  head_fsm_3_running:1;		// 0x00000800
		UINT32	nios_reset:1;				// 0x00001000
		UINT32	fpga_shutdown_from_nios:1;	// 0x00002000
		UINT32	info14:1;					// 0x00004000
		UINT32	info15:1;					// 0x00008000
		UINT32	clearing_udp_flags:1;		// 0x00010000
		UINT32	info17:1;
		UINT32	info18:1;
		UINT32	info19:1;
		UINT32	info20:1;
		UINT32	info21:1;
		UINT32	info22:1;
		UINT32	info23:1;
		UINT32	info24:1;
		UINT32	info25:1;
		UINT32	info26:1;
		UINT32	info27:1;
		UINT32	info28:1;
		UINT32	info29:1;
		UINT32	info30:1;
		UINT32	info31:1;		
	};
	UINT32 flags;
} SFpgaHeadInfo;

//--- Error Flags --------------------------------------------------------
typedef struct SFpgaHeadErr
{
	UINT32	err_0:1;
	UINT32	err_1:1;
	UINT32	err_2:1;
	UINT32	err_3:1;
	UINT32	err_4:1;
	UINT32	err_5:1;
	UINT32	err_6:1;
	UINT32	err_7:1;
	UINT32	err_8:1;
	UINT32	err_9:1;
	UINT32	err10:1;
	UINT32	err11:1;
	UINT32	err12:1;
	UINT32	err13:1;
	UINT32	err14:1;
	UINT32	err15:1;
	UINT32	err16:1;
	UINT32	err17:1;
	UINT32	err18:1;
	UINT32	err19:1;
	UINT32	err20:1;
	UINT32	err21:1;
	UINT32	err22:1;
	UINT32	err23:1;
	UINT32	err24:1;
	UINT32	err25:1;
	UINT32	err26:1;
	UINT32	err27:1;
	UINT32	err28:1;
	UINT32	err29:1;
	UINT32	err30:1;
	UINT32	err31:1;
} SFpgaHeadErr;

//--- SFpgaHeadStat -----------------------------------------------

#define SPEED_MM_SEC(x) ((x*160) / 256000)	// converts speed to mm/sec

typedef struct SFpgaHeadStat
{
	// clock frequency 160 MHz
//	UINT32				info;			//  0x0000
	SFpgaHeadInfo		info;
	UINT32				eth;			//	0x0004: ethernet status
	BYTE				dataLevel_0;	//	0x0008:
	BYTE				dataLevel_1;	//	0x0009:
	BYTE				dataMax_0;		//	0x000A:
	BYTE				dataMax_1;		//	0x000B:
	UINT32				ddr3Arbiter;	//	0x000C:
	UINT8				pg_ctr_10[4];	//	0x0010:
	UINT32				encderCtrl;		//	0x0014:
	
	UINT16				arp_req_bc_0;	//	0x0018: (31:16) ETH0 arp-request mit broadcast (TSE)
	UINT16				arp_req_my_0;	//	0x0018: (15:0)  ETH0 arp-request mit mac (TSE)
	UINT16				arp_req_bc_1;	//	0x001C: (31:16) ETH1 arp-request mit broadcast (TSE)
	UINT16				arp_req_my_1;	//	0x001C: (15:0)  ETH1 arp-request mit mac (TSE)
	UINT32				res_20;			//	0x0020
	struct 
	{
		UINT16			frames_all;	//	0x0024/0x0028: 
	 	UINT16			frames_my;	//	0x0026/0x002a:
	} eth_ctr[2];
	UINT16				udp_data[2];	//	0x002C:
	UINT16				udp_alive[2];	//	0x0030:
	UINT16				arp_rep[2];		//	0x0034: ETH arp-reply (TSE)
	UINT16				udp_all[2];		//	0x0038: ETH udp mit korrektem header (TSE)
	UINT32				udp_invalid_pyld_length; //	0x003:C

	INT32				temp;			//  0x0040:

	UINT8				pg_abort_ctr[4];//	0x0044:
	UINT8				img_line_fp[4];	//	0x0048:
	UINT16				temp_ctr[2];	//	0x004C:
	UINT32				head_dot_cnt[4];//	0x0050 - 0x005C:
	UINT32				head_fp_cnt[4];	//	0x0060 - 0x006C:
	UINT32				res_70;			//	0x0070:
	UINT32				res_74;			//	0x0074:
	UINT32				ddr3_arb_max_ctr;//	0x0078:
	UINT16				ddr3_arb_state;	//	0x007C:
	UINT16				ddr3_arb_hang;	//	0x007C:

	struct                              // 0x0080..0x009F
	{
		UINT32			valid;
		UINT32			data;
	} ddr3[4];

	struct                              // 0x00A0..0x00BF
	{
		UINT32			valid;
		UINT32			data;
	} head_data[4];

	SVersion			version;		//  0x00C0 .. 0x00CC

	UINT32				res_D0;			//	0x00D0:
	UINT32				res_D4;			//	0x00D4:
	UINT32				res_D8;			//	0x00D8:
	UINT32				res_DC;			//	0x00DC:
	UINT32				res_E0;			//	0x00E0:
	UINT32				res_E4;			//	0x00E4:
	UINT32				res_E8;			//	0x00E8:
	UINT32				res_EC;			//	0x00EC:
	UINT32				res_F0;			//	0x00F0:
	UINT32				res_F4;			//	0x00F4:
	UINT32				res_F8;			//	0x00F8:
	UINT32				res_FC;			//	0x00FC:

	struct 
	{									//	0x0100: 0x0104:
		UINT32 requestReady:1;
		UINT32 replyDone:1;
	} unused_arp[2];

	UINT32				fsm_state[6];	//	0x0108 .. 0x011c:

	UINT32				res_2c120;		//	0x0120:
	UINT32				res_2c124;		//	0x0124:
	UINT32				res_2c128;		//	0x0128:
	UINT32				res_2c12c;		//	0x012c:
	UINT32				res_2c130;		//	0x0130:
	UINT32				res_2c134;		//	0x0134:
	UINT32				res_2c138;		//	0x0138:
	UINT32				res_2c13c;		//	0x013c:

	UINT32				enc_fp_cnt[4];	// 0x0140 - 0x014f
	UINT32				unused_enc_pg_cnt[4];	// 0x0150 - 0x015f

	UINT32				res_160_180[0x20/4];			// 0x160 - 0x17f
	UINT32				blockOutNo[4];					// 0x0180 - 0x018f
	UINT32				res_190_1C0[(0x01C0-0x0190)/4];	 
	UINT32				print_done_ctr[4];				// 0x01C0 - 0x01CF
	UINT32				res_1d0_1ff[(0x0200-0x01d0)/4];

	//--- ENCODER -----------------------------------------------------
	UINT32				enc_position[4];	// 0x0200
	struct {
		UINT32			current;
		UINT32			min;
		UINT32			max;
	}					enc_speed[4];

	UINT32				pg_in_position[4];
	UINT32				pg_out_position[4];
	UINT32				pg_out_delay[4];

	UINT32				fp_ok_cnt[4];
	UINT32				fp_late_cnt[4];

	UINT32				img_start_ctr[4];
	UINT32				img_late_ctr[4];
	UINT32				shake_start_ctr[4];
	UINT32				fp_ctr[4];
	UINT32				pg_ctr[4];

	UINT32				speed_current1;	
	UINT32				speed_min1;		
	UINT32				speed_max1;			// 0x02e8	

	UCHAR				tel_pg_cnt[4];			// 0x02ec
	UINT32				res_xxx[(0x300-0x2f0)/4];
	
	UINT32				enc_tel[8];			// 0x0300
		#define ENC_PG_FLAG 0x08
	UINT32				debug[8];
} SFpgaHeadStat;

typedef struct SFpgaHeadError
{
	struct							// 0x0000 - 0x000F
	{
		UINT16 dest_ip;			// 
		UINT16 dest_port;		// 
		UINT16 checksum;		// 
		UINT16 udp_checksum;	//
	} eth[2];

	UINT16	udp_fifo_full[2];		// 0x0010:
	UINT16	eth_fifo_full[2];		// 0x0014:
	UINT16	udp_block_error;		// 0x0018:
	UINT16	res_1a;					// 0x001A:

	struct							// 0x001C - 0x004B
	{
		UINT16	fifo_img_line;		// 0x001C:
		UINT16	write_img_line;		// 0x001E:	"write image line error 5" (ursprünglicher globaler Error): Time-out: Bild-Info nach Bildüberlappung nicht oder zu spät verfügbar
		UINT16	read_img_line;		// 0x0020:
		UINT16	prepare_img_line;	// 0x0022:
		UINT16	enc_fp_missed;		// 0x0024:	
		UINT16  latch_missed;		// 0x0026:
	} head[4];

	struct                          // 0x004C:
	{
		UINT8	err0;
		UINT8	err1;
		UINT8	err2;
		UINT8	crc;
	} encoder[4];
	
	UINT32 synch_test;				// 0x005c	c_enc_tel_res_bits_error_addr		

	struct                          // 0x0060 - 0x007F
	{
		UINT16	waveform_busy;
		UINT16	fsm_busy;
		UINT32	time;
	} enc_fp[4];

	struct                        // 0x2D080 ,  0x2D084
	{
		UINT16 arp_req_missed;
		UINT16 arp_ip_mismatch;
	} arp_error[2];

	struct						// 0x2D088 ,  0x2D08C
	{
		UINT16 too_long;
		UINT16 too_short;
	} dup_error[2];
	UINT32	enc_tel_cnt;		// 0x2D090
	UINT32	overheat_error;		// 0x2D094
	UINT16	udp_length_error;	// 0x2D098
	UINT16	res_9A;				// 0x2D09A
	UINT16	udp_flush_fifo;		// 0x2D09C
	UINT16	res_9E;				// 0x2D09E
	UINT8	img_line_err[4][4]; // 0x2D0A0 .. 0x2DAE
	UINT8	clear_area_32[4];	// 0x2D0B0
	//	"write image line error 0": erste Bildzeile nach Bildlücke nicht in img-FIFO verfügbar
	//	"write image line error 1": erste Bildzeile nach Bildüberlappung nicht in img-FIFO verfügbar
	//	"write image line error 2": erste Bildzeile nach Unterbruchlos-Druck (Bild an Bild ohne Lücke) nicht in img-FIFO verfügbar
	//	"write image line error 3": Bildzeile (nicht die erste) nicht in img-FIFO verfügbar

} SFpgaHeadError;

typedef struct SFpgaDataStat
{
	UINT8	imgOutIdx[MAX_HEADS_BOARD][4];	// 0x0000 - 0x000C
	UINT32	res_10_3F[0x0c];				// 0x0010 - 0x003C
	UINT32	pgInIdx[MAX_HEADS_BOARD];		// 0c0040 - 0x004C
	UINT32	pgOutIdx[MAX_HEADS_BOARD];		// 0c0050 - 0x005C
	UINT32	res_60_7F[(0x0080-0x0060)/4];	// 0x0060 - 0x007C
	UINT32	blockCnt[MAX_HEADS_BOARD];		// 0x0080 - 0x008C
	UINT32	wf_busy_warn[MAX_HEADS_BOARD];	// 0x0090 0 0x009C
} SFpgaDataStat;

typedef struct SFpgeEncoderStat
{
	UINT32	posFs;
	UINT32	posFss;
	UINT32	freqFs;
	UINT32	inkCtrl;
	UINT32	pgCnt[1];//[MAX_HEADS_BOARD];
	UINT32	pdCnt[1];//[MAX_HEADS_BOARD];
} SFpgeEncoderStat;
