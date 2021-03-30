// ****************************************************************************
//
//	DIGITAL PRINTING - putty.c
//
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "rx_def.h"
#include "rx_term.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "fpga.h"
#include "nios.h"
#include "tse.h"
#include "EEprom.h"
#include "rx_head_ctrl.h"
#include "conditioner.h"
#include "version.h"
#include "putty.h"


//--- macros ------------------------------------------------------------------
#define PRINTF(n)  for(i=0; i<n; i++) term_printf

//--- statics --------------------------------------------------------
static char _input[64];
static int	_status=FALSE;
static int  _cond=TRUE;
static int  _nios=TRUE;
static int  _eeprom=FALSE;
static int  _mvteeprom=FALSE;

static HANDLE	hPuttyThread;
static int		_PuttyRunning;

//--- prototypes ----------------------------------------------------------
static void* _putty_thread(void* lpParameter);
static void _main_menu(void);
static void _display_mvteeprom(void);

//--- putty_init --------------------------------------
void putty_init(void)
{
	memset(_input, 0, sizeof(_input));
	_PuttyRunning = TRUE;
	hPuttyThread = rx_thread_start(_putty_thread, NULL, 0, "Putty Thread");					
}

//--- _putty_thread ----------------------------------
static void* _putty_thread(void* lpParameter)
{
//	rx_set_tread_priority(40);

	while(_PuttyRunning)
	{		
		rx_sleep(500);
		term_printf("\033[2J"); // Clear screen
		term_printf("rx_head_ctrl %s (%s, %s)", version, __DATE__, __TIME__);
		putty_display_fpga_error();
		putty_display_fpga_status();
		putty_display_nios_status(_nios, _status);

		if (_eeprom) eeprom_display();
		if (_mvteeprom) _display_mvteeprom();

		if (_cond) putty_display_cond_status(_status);			
		_main_menu();
	}
	return NULL;
}

//--- _main_menu -----------------------------------
static void _main_menu(void)
{
	char str[MAX_PATH];
	int synth=FALSE;
	static int cnt=0;
	int i;
	int no;
	
	term_printf("\n");
	term_printf("MENU ------------------------- %d\n", ++cnt);
//		term_printf("r<filename>: load rbf\n");
//		term_printf("t: Test, send UDP DataBlock\n");
//		term_printf("T: Test, send 1000 UDP DataBlocks\n");
	term_printf("d<n>: Drop size fixed (n=0..3)\n");
	term_printf("p<filename>: print bmp file (default=angle.bmp)\n");
	//term_printf("P<filename>: print tif file (default=maserati_K.tif)\n");
	term_printf("g: print GO\n");
//	term_printf("u: Display block used flags\n");
	term_printf("l: start pressure log of head[0]\n");	
	term_printf("R: trace registers\n");
	term_printf("t: TCP/IP Stress Test\n");
	term_printf("E: Reset pending errors\n");
	term_printf("h<xxx>: Frequency in Hz\n");
	term_printf("d<x>:   Drop size fixed 0..3\n");
	if (_status)
	{
		term_printf("o: cond. OFF mode\n");
		term_printf("r: cond. Print mode\n");	
//		term_printf("n: cond. Watchdog Test\n");
		term_printf("q: start log\n");
		term_printf("i: heater [C] (<=50)\n");	

		term_printf("z<x>:   Calibrate pressure sensor\n");
    	term_printf("u<x>:   Delete user calibrated offset\n");
	}
	if (_cond)
	{
		term_printf("P<n>: Set pump to 40%\n");		
		term_printf("#<n><number>: Set SerialNo of conditioner<n>\n");	
	}
	if (_mvteeprom)
	{
		term_printf("ra<h><angle>:   set rob_angle[<h>]=<angle>\n");
    	term_printf("rs<h><stitch>:  set rob_stitch [<h>]=<stitch>\n");
	}
	if (_status) term_printf("s: hide status       ");
	else         term_printf("s: show status       ");
	if (_cond)   term_printf("c: hide conditioner  ");
	else         term_printf("c: show conditioner  ");
	if (_nios)   term_printf("n: hide nios         ");
	else         term_printf("n: show nios         ");
	if (_eeprom) term_printf("e: hide eeprom\n");
	else         term_printf("e: show eeprom\n");
	if (_mvteeprom) term_printf("m: hide mouvent eeprom\n");
	else            term_printf("m: show mouvent eeprom\n");
	
	term_printf("x: exit\n");
	term_printf(">");
	term_flush();
	
	if (term_get_str(str, sizeof(str)))
	{
		strcpy(_input, str);
	}	
}

//--- putty_handle_menu --------------------------
void putty_handle_menu(char *str)
{
	switch (str[0])
	{
		case 'e': _eeprom = ! _eeprom;	break;
		case 's': _status = !_status;	break;
		case 'm': _mvteeprom = ! _mvteeprom;	
				  // if (_mvteeprom) eeprom_read(); 
				  break;
		case 'n': _nios   = !_nios;		break;
		case 'c': _cond   = !_cond;		break;
	}
}

//--- putty_input ---------------------------------------
char *putty_input(char *str, int size)
{
	strncpy(str, _input, size);
	memset(_input, 0, sizeof(_input));
	return str;
}

//--- _speed_160 ------------------
static int _speed_160(int ctr)
{
	if (ctr<=0) return 0;
	return 140000000/ctr;
}

//--- _droplets --------------
static int _droplets(UINT8 level)
{
	int cnt;
	for (cnt=0; level; level>>=1)
	{
		if (level&1) cnt++;		
	}
	return cnt;
}

//--- putty_display_fpga_status -----------------------------------------------
void putty_display_fpga_status(void)
{
	//--- simulation -------------
	static UINT32 data[4];
	static UINT32 cnt[4]={0,0,0,0};
					
	int head, change=FALSE;
	int f, freq;
	int blkCnt;
	char str[32], str1[32];
	char line[90];
	int i,l;
	int no[MAX_HEADS_BOARD];
		
	for(i=0; i<MAX_HEADS_BOARD; i++) no[i]= RX_HBConfig.reverseHeadOrder? MAX_HEADS_BOARD-1-i:i;
		
	//-----------------------------------------
	blkCnt = RX_HBConfig.dataBlkCntHead/32;

	term_printf("\n");
	term_printf("--- FPGA Status -----------------------------------\n");
	term_printf("Version: %d.%d.%d.%d                Linux: V%d\n",			RX_FpgaStat.version.major, RX_FpgaStat.version.minor, RX_FpgaStat.version.revision, RX_FpgaStat.version.build, RX_LinuxDeployment);
	term_printf("Temp:    %d  Overheat.Error: %d  FPGA.cmd=0x%04x\n",		RX_FpgaStat.temp-128, RX_FpgaError.overheat_error, RX_FpgaCmd);
	freq=_speed_160(RX_FpgaEncCfg.synth.value);
	term_printf("Encoder:  linkEnable=%d, telCnt=%u, synthEnable=%d, synthHz=%d'%03d\n", (RX_FpgaEncCfg.cmd & ENC_ENABLE)!=0, fpga_get_encTelFreq(), RX_FpgaEncCfg.synth.enable, freq/1000, freq%1000);
//		term_printf("info:  0x%08x\n", RX_FpgaStat.info);
	
	term_printf("Speed [Hz]:   ");
	freq=0;
	for(i=0; i<MAX_HEADS_BOARD; i++)
	{
		f=_speed_160(RX_FpgaStat.enc_speed[i].current);
		term_printf("%08d   ", f);
		if (f>freq) freq=f;
	}
	term_printf("     %d m/min\n", (int)(60.0*freq/1200.0*0.0254+0.5));
//		term_printf("speed min:   "); PRINTF(4)("%08d   ", _speed_160(RX_FpgaStat.enc_speed[i].min));		term_printf("\n");
//		term_printf("speed max[Hz]:"); PRINTF(4)("%08d   ", _speed_160(RX_FpgaStat.enc_speed[i].max));		term_printf("\n");
//		term_printf("UDP Blocks sent:%06d  BlkSize=%d  Blk/Head=%d\n",			udp_test_sent_blocks(), fpga_udp_block_size(), RX_HBConfig.head[1].blkNo0);		
	tse_display_status(RX_UdpSpeed);
	term_printf("ETH ALL Frames: %06d %06d     mac[0]: %s  ip[0]: %s\n",					RX_FpgaStat.eth_ctr[0].frames_all,	RX_FpgaStat.eth_ctr[1].frames_all,   RX_MacAddr[0], RX_IpAddr[0]);		
	term_printf("ETH MY  Frames: %06d %06d     mac[1]: %s  ip[1]: %s\n",					RX_FpgaStat.eth_ctr[0].frames_my,	RX_FpgaStat.eth_ctr[1].frames_my,    RX_MacAddr[1], RX_IpAddr[1]);			
	term_printf("UDP ALL:        %06d %06d     ARP REQ (BC) %06d %06d  IP ERR %06d %06d\n",	RX_FpgaStat.udp_all[0],	  RX_FpgaStat.udp_all[1],	RX_FpgaStat.arp_req_bc_0,			RX_FpgaStat.arp_req_bc_1, RX_FpgaError.arp_error[0].arp_ip_mismatch, RX_FpgaError.arp_error[1].arp_ip_mismatch);	
	term_printf("UDP ALIVE:      %06d %06d     ARP REQ (MY) %06d %06d  MISSED %06d %06d\n",	RX_FpgaStat.udp_alive[0], RX_FpgaStat.udp_alive[1], RX_FpgaStat.arp_req_my_0,			RX_FpgaStat.arp_req_my_1, RX_FpgaError.arp_error[0].arp_req_missed,	 RX_FpgaError.arp_error[1].arp_req_missed);			
	term_printf("UDP DATA:       %06d %06d     ARP REPLY    %06d %06d\n",					RX_FpgaStat.udp_data[0],				RX_FpgaStat.udp_data[1],				RX_FpgaStat.arp_rep[0],				RX_FpgaStat.arp_rep[1]);	
	
//		term_printf("FIFO Level:     %06d %06d     max          %06d %06d\n",	RX_FpgaStat.dataLevel_0,				RX_FpgaStat.dataLevel_1,				RX_FpgaStat.dataMax_0,				RX_FpgaStat.dataMax_1);	
		
//		term_printf("FSM-state:      "); PRINTF(6)("%04x ", RX_FpgaStat.fsm_state[i]); term_printf("\n");
//		term_printf("DDR-ARB-state:  %04x           DDR-ARB-err: %04d max: %04d\n", RX_FpgaStat.ddr3_arb_state, RX_FpgaStat.ddr3_arb_hang, RX_FpgaStat.ddr3_arb_max_ctr);
		
	term_printf("             ");
	for (i=0; i<MAX_HEADS_BOARD; i++)
	{		
		strcpy(line, "----------");
		l= sprintf(str, " %d:%s ", no[i]+1, RX_HBConfig.head[no[i]].name);
		memcpy(&line[(10-l)/2], str, l);			
		term_color(RX_RGB[no[i]]);
		term_printf(line);
		term_color(BLACK);
		term_printf("  ");
	};		
	term_printf("\n");

//	term_printf("blockUsed:  0x%08x 0x%08x 0x%08x 0x%08x  cmd: 0x%08x  enc.cmd: 0x%08x\n",	Fpga.blockUsed[0],		Fpga.blockUsed[1*blkCnt],			Fpga.blockUsed[2*blkCnt],			Fpga.blockUsed[3*blkCnt], FpgaCfg.cfg->cmd, FpgaCfg.encoder->cmd);
//	term_printf("blockUsed:    "); PRINTF(4)("%08x   ",	RX_BlockUsed[no[i]]);						term_printf("\n");
	term_printf("blockCnt:     "); PRINTF(4)("%09d   ",	RX_FpgaData.blockCnt[no[i]]);			term_printf("\n");
	term_printf("imgInIdx/buf: "); PRINTF(4)("  %03d %03d   ",  RX_FpgaPrint.imgInIdx[no[i]],    RX_HBStatus[0].head[no[i]].imgInCnt-RX_HBStatus[0].head[no[i]].printDoneCnt); term_printf("\n");
	term_printf("imgOutIdx:    "); PRINTF(4)("  %03d %03d   ", RX_FpgaData.imgOutIdx[no[i]][0], RX_FpgaData.imgOutIdx[no[i]][1]);	term_printf("\n");

	term_printf("GreyLevel:    "); PRINTF(4)("%d %d %d       ",    _droplets(RX_GreyLevel[no[i]][1]), _droplets(RX_GreyLevel[no[i]][2]), _droplets(RX_GreyLevel[no[i]][3]) );term_printf("\n");
//	term_printf("Print GO ENC: "); PRINTF(4)("%09d   ",    RX_HBStatus[0].head[no[i]].encPgCnt);									term_printf("\n");
	term_printf("Print GO:     "); PRINTF(4)("%09d   ",    RX_FpgaStatRunning.pg_ctr[no[i]]);										term_printf("\n");
//	term_printf("Print GO Tel: "); PRINTF(4)("%09d   ",    RX_FpgaStat.tel_pg_cnt[no[i]]);											term_printf("\n");
//	term_printf("PG abort:     "); PRINTF(4)("%09d   ",    RX_FpgaStat.pg_abort_ctr[no[i]]);										term_printf("\n");
	term_printf("Print DONE:   "); PRINTF(4)("%09d   ",    RX_FpgaStatRunning.print_done_ctr[no[i]]);								term_printf("\n");
//	term_printf("Print GO Pos: "); PRINTF(4)("%09d   ",	   RX_FpgaStat.pg_in_position[no[i]]);										term_printf("\n");
//	term_printf("Print Pos:    "); PRINTF(4)("%09d   ",	   RX_HBStatus[0].head[no[i]].encPos-RX_FpgaStat.pg_in_position[no[i]]);	term_printf("\n");
//	term_printf("Blk0:         "); PRINTF(4)("%09d   ",    FpgaCfg.udp->block[no[i]].blkNo0);										term_printf("\n");
//	term_printf("BlkEnd:       "); PRINTF(4)("%09d   ",    FpgaCfg.udp->block[no[i]].blkNoEnd);										term_printf("\n");
//	term_printf("BlkOut:       "); PRINTF(4)("%09d   ",    _BlockOutIdx[no[i]]);													term_printf("\n");
//	term_printf("Udp ptr:      "); PRINTF(4)("%09d   ",    RX_FpgaStat.blockOutNo[no[i]]);											term_printf("\n");
//	term_printf("imgLineFP:    "); PRINTF(4)("%09d   ",    RX_FpgaStat.img_line_fp[no[i]]);											term_printf("\n");
	term_printf("Enc #:        "); PRINTF(4)("% 9d   ",    RX_HBConfig.head[no[i]].encoderNo);										term_printf("\n");
	term_printf("Enc Position: "); PRINTF(4)("%07d.%d   ", RX_FpgaStat.enc_position[no[i]]/8, RX_FpgaStat.enc_position[no[i]]%8);	term_printf("\n");
	term_printf("Enc Position: "); PRINTF(4)("%09d   ",    RX_HBStatus[0].head[no[i]].encPos);										term_printf("\n");
//	term_printf("enc_fp_ct:    "); PRINTF(4)("%09d   ",    RX_FpgaStat.enc_fp_cnt[no[i]]);											term_printf("\n");
	term_printf("head_fp_ct:   "); PRINTF(4)("%09d   ",	   RX_FpgaStat.head_fp_cnt[no[i]]);											term_printf("\n");
	term_printf("head_dt_ct:   "); PRINTF(4)("%09lld   ",  RX_HBStatus[0].head[no[i]].dotCnt);										term_printf("\n");

	if (FALSE)
	{
		term_printf("\n");
//			term_printf("Encoder ----------------------------------\n");
	//	term_printf("Enc Position:");	  PRINTF(4)("%06d.%d   ", RX_FpgaStat.enc_position[i]/8, RX_FpgaStat.enc_position[i]%8); term_printf("\n");
	//	term_printf("pos bwd:     "); PRINTF(4)("%08d   ", RX_FpgaStat.pg_in_position[i]>>31);	term_printf("\n");
		/*
		if (FpgaCfg.encoder->cmd & ENC_SIGNAL_MODE)
		{
			speed = SPEED_MM_SEC(RX_FpgaStat.speed_current1);
			term_printf("Speed (mm/sec):   %d  min=%d max=%d freq=%d Hz\n",	speed, SPEED_MM_SEC(RX_FpgaStat.speed_min1), SPEED_MM_SEC(RX_FpgaStat.speed_max1),  (int)((speed*1200.0)/25.4));
		}
		else
		{			
			term_printf("speed:       "); PRINTF(4)("%08d   ", _speed_160(RX_FpgaStat.enc_speed[i].current));	term_printf("\n");
			term_printf("speed min:   "); PRINTF(4)("%08d   ", _speed_160(RX_FpgaStat.enc_speed[i].min));		term_printf("\n");
			term_printf("speed max:   "); PRINTF(4)("%08d   ", _speed_160(RX_FpgaStat.enc_speed[i].max));		term_printf("\n");
		}
		*/
//			int mask = 0x7fffffff;
//			term_printf("PG-IN pos:   "); PRINTF(4)("%08d   ", RX_FpgaStat.pg_in_position[i]&mask);			term_printf("\n");
//			term_printf("PG-OUT pos:  "); PRINTF(4)("%08d   ", RX_FpgaStat.pg_out_position[i]);				term_printf("\n");
//			term_printf("PG-OUT delay:"); PRINTF(4)("%08d   ", RX_FpgaStat.pg_out_delay[i]);				term_printf("\n");

//			term_printf("PG IN  idx:  "); PRINTF(4)("%08d   ", RX_FpgaData.pgInIdx[i]);						term_printf("\n");
//			term_printf("PG OUT idx:  "); PRINTF(4)("%08d   ", RX_FpgaData.pgOutIdx[i]);					term_printf("\n");

//			term_printf("FP OK:       "); PRINTF(4)("%08d   ", RX_FpgaStat.fp_ok_cnt[i]);					term_printf("\n");
//			term_printf("FP late:     "); PRINTF(4)("%08d   ", RX_FpgaStat.fp_late_cnt[i]);					term_printf("\n");
//			term_printf("IMG start:   "); PRINTF(4)("%08d   ", RX_FpgaStat.img_start_ctr[i]);				term_printf("\n");
//			term_printf("IMG late:    "); PRINTF(4)("%08d   ", RX_FpgaStat.img_late_ctr[i]);				term_printf("\n");
//			term_printf("SHAKE start: "); PRINTF(4)("%08d   ", RX_FpgaStat.shake_start_ctr[i]);				term_printf("\n");
//			term_printf("TEL-PG:    0x%08x\n", RX_FpgaStat.tel_pg_cnt);
//			term_printf("TEL-Synch: 0x%08x\n", RX_FpgaError.synch_test);
//			term_printf("FP:          "); PRINTF(4)("%08d   ", RX_FpgaStat.fp_ctr[i]);						term_printf("\n");
//			term_printf("PG:          "); PRINTF(4)("%08d   ", RX_FpgaStat.pg_ctr[i]);						term_printf("\n");

		term_printf("Encoder TEL: "); PRINTF(8)("0x%08x ", RX_FpgaStat.enc_tel[i]);							term_printf("\n");
	//	term_printf("Encoder Test:"); PRINTF(8)("0x%08x ", RX_FpgaStat.enc_test[i]);						term_printf("\n");
		term_printf("Encoder crc: "); PRINTF(4)("%03d  ",  RX_FpgaError.encoder[i].crc);					term_printf("\n");
		term_printf("Encoder c0:  "); PRINTF(4)("%03d  ",  RX_FpgaError.encoder[i].err0);					term_printf("\n");
		term_printf("Encoder c1:  "); PRINTF(4)("%03d  ",  RX_FpgaError.encoder[i].err1);					term_printf("\n");
		term_printf("Encoder c2:  "); PRINTF(4)("%03d  ",  RX_FpgaError.encoder[i].err2);					term_printf("\n");
			
	//	term_printf("Debug Register : "); PRINTF(8)("0x%08x ", RX_FpgaStat.debug[i]);				term_printf("\n");

		//		term_printf("\n");
//		term_printf("info:      %08x\n", *(UINT32*)Fpga.stat);
	}
}

//--- _display_mvteeprom -----------------------------------------------------------------------
static void _display_mvteeprom(void)
{
	int				i, n, l;
	char str[64];
	char line[90] = {0};

	int no[MAX_HEADS_BOARD];
		
	for(i=0; i<MAX_HEADS_BOARD; i++) no[i]= RX_HBConfig.reverseHeadOrder? MAX_HEADS_BOARD-1-i:i;
	term_printf("\n");
	term_printf("Mouvent EEprom  ");
				
	for (i=0; i<MAX_HEADS_BOARD; i++)
	{			
		strcpy(line, "--------------");
		l = sprintf(str, " %d:%s ", no[i]+1, RX_HBConfig.head[no[i]].name);
		memcpy(&line[(15-l)/2], str, l);
			
		term_color(RX_RGB[no[i]]);
		term_printf(line);
		term_color(BLACK);
		term_printf("  ");
	};		
	term_printf("\n");

    term_printf("eeprom_fuji: ");
    PRINTF(4)("r=%d(%d)   ", 
			_NiosMem->stat.eeprom_fuji_readCnt[no[i]], _NiosMem->cfg.eeprom_fuji_readCnt[no[i]]);
	term_printf("\n");

    term_printf("eeprom_mvt: ");
    PRINTF(4)("r=%d(%d) w=%d(%d)   ", 
			_NiosMem->stat.eeprom_mvt_readCnt[no[i]], _NiosMem->cfg.eeprom_mvt_readCnt[no[i]],
			_NiosMem->stat.eeprom_mvt_writeCnt[no[i]], _NiosMem->cfg.eeprom_mvt_writeCnt[no[i]]);
	term_printf("\n");

    term_printf("eeprom_dens: ");
    PRINTF(4)("r=%d(%d) w=%d(%d)   ", 
			_NiosMem->stat.eeprom_density_readCnt[no[i]], _NiosMem->cfg.eeprom_density_readCnt[no[i]],
			_NiosMem->stat.eeprom_density_writeCnt[no[i]], _NiosMem->cfg.eeprom_density_writeCnt[no[i]]);
	term_printf("\n");

	term_printf("clusterNo:        "); PRINTF(4)("%12d    ", RX_HBStatus->head[no[i]].eeprom_mvt.clusterNo);		 term_printf("\n");
	term_printf("flowResistance:   "); PRINTF(4)("%12d    ", RX_HBStatus->head[no[i]].eeprom_mvt.flowResistance); term_printf("\n");
	term_printf("dropletsPrinted:  "); PRINTF(4)("%12lld    ", RX_HBStatus->head[no[i]].eeprom_mvt.dropletsPrinted);term_printf("\n");

	term_printf("Voltage:         "); PRINTF(4)("          %03d   ",	RX_HBStatus->head[no[i]].eeprom_density.voltage);	term_printf("\n");
	for (n=0; n<MAX_DENSITY_VALUES; n++)
	{
		term_printf("Density[%02d]:     ", n); PRINTF(4)("         %04d   ",	RX_HBStatus->head[no[i]].eeprom_density.densityValue[n]);	term_printf("\n");
	}
	for (n=0; n<10; n++)
	{
		term_printf("DisabledJet[%d]:  ", n); PRINTF(4)("         %04d   ",	RX_HBStatus->head[no[i]].eeprom_density.disabledJets[n]);	term_printf("\n");
	}
	term_printf("Robot-Angle:     "); PRINTF(4)("         %s   ",	value_str_screw(RX_HBStatus->head[no[i]].eeprom_mvt.robot.angle));	term_printf("\n");
	term_printf("Robot-Stitch:    "); PRINTF(4)("        %s   ",	value_str_screw(RX_HBStatus->head[no[i]].eeprom_mvt.robot.stitch));	term_printf("\n");
}

//--- putty_display_fpga_error --------------------------------------------------
void  putty_display_fpga_error(void)
{
	int				i;

	term_printf("\n");
	term_printf("--- FPGA Errors ---------------------------------------\n");
	term_printf("eth_dest_ip:    ");	PRINTF(2)("0x%04x ", RX_FpgaError.eth[i].dest_ip);	
	term_printf("   fifo_img_line:  "); PRINTF(4)("0x%04x ", RX_FpgaError.head[i].fifo_img_line);	term_printf("\n");
	term_printf("eth_dest_port:  ");	PRINTF(2)("0x%04x ", RX_FpgaError.eth[i].dest_port);
	term_printf("   write_img_line: "); PRINTF(4)("0x%04x ", RX_FpgaError.head[i].write_img_line);	term_printf("(PG but no data)\n");
	term_printf("eth_checksum:   ");	PRINTF(2)("0x%04x ", RX_FpgaError.eth[i].checksum);
	term_printf("   read_img_line:  "); PRINTF(4)("0x%04x ", RX_FpgaError.head[i].read_img_line);	term_printf("\n");
	term_printf("eth_udp_chk:    ");	PRINTF(2)("0x%04x ", RX_FpgaError.eth[i].udp_checksum);
	term_printf("   prep_img_line:  "); PRINTF(4)("0x%04x ", RX_FpgaError.head[i].prepare_img_line);term_printf("\n");
	term_printf("eth_fifo_full:  ");	PRINTF(2)("0x%04x ", RX_FpgaError.eth_fifo_full[i]);		
	term_printf("   wf-busy ERR:    ");	PRINTF(4)("0x%04x ", RX_FpgaError.enc_fp[i].waveform_busy);	term_printf("\n");

	term_printf("udp_fifo_full:  ");	PRINTF(2)("0x%04x ", RX_FpgaError.udp_fifo_full[i]);
	term_printf("   wf-busy WARN:   ");	PRINTF(4)("0x%04x ", RX_FpgaData.wf_busy_warn[i]);	term_printf("\n");
	term_printf("udp_too_short:  ");	PRINTF(2)("0x%04x ", RX_FpgaError.dup_error[i].too_short);	
	term_printf("   enc_fp_missed:  ");	PRINTF(4)("0x%04x ", RX_FpgaError.head[i].enc_fp_missed);	term_printf("\n");
	term_printf("udp_too_long:   ");	PRINTF(2)("0x%04x ", RX_FpgaError.dup_error[i].too_long);
//		term_printf("   fsm-busy:    ");	PRINTF(4)("0x%04x ", RX_FpgaError.enc_fp[i].fsm_busy);		term_printf("\n");
//		term_printf("   time:        ");	PRINTF(4)("0x%04x ", RX_FpgaError.enc_fp[i].time);			term_printf("\n");
//		term_printf("   latch_missed:   ");	PRINTF(4)("0x%04x ", RX_FpgaError.head[i].latch_missed);	term_printf("\n");
	term_printf("   enc_crc:        ");	PRINTF(4)("0x%04x ", RX_FpgaError.encoder[i].crc);			term_printf("\n");
//	term_printf("clear_area_32:  "); PRINTF(4)("%08d   ",    RX_FpgaError.clear_area_32[i]);			term_printf("\n");
//	term_printf("ETH.invalid.pyLdLen :%d UDP.PyldLenErr: %d UDP.FifoFlush: %d\n", RX_FpgaStat.udp_invalid_pyld_length, RX_FpgaError.udp_length_error, RX_FpgaError.udp_flush_fifo);

	term_printf("\n");
	term_printf("img_line_err[0]: "); PRINTF(4)("%08d   ",    RX_FpgaError.img_line_err[0][i]);			term_printf(" 1st data line missing\n");
	term_printf("img_line_err[1]: "); PRINTF(4)("%08d   ",    RX_FpgaError.img_line_err[1][i]);			term_printf(" PG: img-info missing\n");
	term_printf("img_line_err[2]: "); PRINTF(4)("%08d   ",    RX_FpgaError.img_line_err[2][i]);			term_printf(" not used\n");
	term_printf("img_line_err[3]: "); PRINTF(4)("%08d   ",    RX_FpgaError.img_line_err[3][i]);			term_printf(" data line missing\n");		
	
	term_printf("fifo_img_line:   "); PRINTF(4)("%08d   ",    RX_FpgaError.head[i].fifo_img_line);		term_printf("\n");
	term_printf("write_img_line:  "); PRINTF(4)("%08d   ",    RX_FpgaError.head[i].write_img_line);		term_printf("\n");
	term_printf("read_img_line:   "); PRINTF(4)("%08d   ",    RX_FpgaError.head[i].read_img_line);		term_printf("\n");
	term_printf("prepare_img_line:"); PRINTF(4)("%08d   ",    RX_FpgaError.head[i].prepare_img_line);	term_printf("\n");
	term_printf("enc_fp_missed:   "); PRINTF(4)("%08d   ",    RX_FpgaError.head[i].enc_fp_missed);		term_printf("\n");
	term_printf("latch_missed:    "); PRINTF(4)("%08d   ",    RX_FpgaError.head[i].latch_missed);		term_printf("\n");
}

//--- putty_display_nios_status ----------------------------
void putty_display_nios_status(int nios, int status)
{
	int speed0, speed1, i;
	int temp;
	char str[64];
	int no[MAX_HEADS_BOARD];
	
	for(i=0; i<MAX_HEADS_BOARD; i++) no[i]= RX_HBConfig.reverseHeadOrder? MAX_HEADS_BOARD-1-i:i;
				
	if (nios_loaded())
	{
		term_printf("\n--- NIOS Status ----------------- FPGA-QSYS: id=%d time=%d\n", fpga_qsys_id(), fpga_qsys_timestamp());
		if (!nios) return;
		
		sprintf(str, "%d.%d.%d.%d", RX_NiosStat.version.major, RX_NiosStat.version.minor, RX_NiosStat.version.revision, RX_NiosStat.version.build);
        term_printf("Version:     %-20s NIOS-QSYS: id=%d time=%d\t", str, RX_NiosStat.QSYS_id, RX_NiosStat.QSYS_timestamp);
		
		if (RX_NiosStat.error.fpga_incompatible) term_printf("ERROR\n");
		else                                    term_printf("OK\n");

		term_printf("alive:           %03d.%03d  Power: %s Temp:%s FixedDropSize:%d Droplets:%d\n", RX_NiosStat.alive/1000, RX_NiosStat.alive%1000, PowerStateStr(RX_NiosStat.powerState), value_str_temp(RX_NiosStat.headcon_amc_temp), NIOS_FixedDropSize, NIOS_Droplets);
//		term_printf("arm_timeout:     %d  \n", RX_NiosStat.error.arm_timeout);

		if (RX_NiosStat.info.cooler_pcb_present)
		{
			term_printf("Cooler:        ");
			if (RX_NiosStat.error.cooler_overpressure) term_color(RED);
			term_printf("pres=%s mbar", value_str(RX_NiosStat.cooler_pressure));
			term_color(BLACK);
			term_printf("  ");			
			if (RX_NiosStat.error.cooler_overheated) term_color(RED);
			term_printf("  temp=%sC", value_str_temp(RX_NiosStat.cooler_temp));
			term_color(BLACK);
			term_printf("  overheated=%d ", RX_NiosStat.error.cooler_overheated);
			term_printf("  overpressure=%d ", RX_NiosStat.error.cooler_overpressure);
			/*
			{
				UCHAR *id=(UCHAR*)&RX_NiosStat.cooler_pressure_ID;
				term_printf("  ID=%d.%d.%d.%d", id[0], id[1], id[2], id[3]);				
			}
			*/
			term_printf("\n");			
		}
		else
		{
			term_printf("Cooler:        PCB missing\n");
		}

		term_printf("printed: [l]    "); for (i=0; i<MAX_HEADS_BOARD; i++) term_printf("  %14s", value_str3(RX_HBStatus->head[no[i]].printed_ml)); term_printf("\n");
		term_printf("printed: [time] "); for (i=0; i<MAX_HEADS_BOARD; i++) term_printf("  %14s", value_str_time(RX_HBStatus->head[no[i]].eeprom_mvt.printingSec)); term_printf("\n");
		term_printf("Temp Head:      "); for (i=0; i<MAX_HEADS_BOARD; i++) term_printf("  %14s", value_str_temp(RX_NiosStat.head_temp[no[i]])); term_printf("\n");
		term_printf("Cluster:  No: %06d", RX_HBStatus->clusterNo);
		{
			int h, m, s;
			s = RX_NiosStat.cond[0].clusterTime%60;
			h = RX_NiosStat.cond[0].clusterTime/3600;
			m = (RX_NiosStat.cond[0].clusterTime%3600)/60;					
			term_printf("  time: %d:%02d:%02d  ", h, m, s);
		}
		term_printf("\n");
	}		
	else 
	{
		term_printf("\n--- NIOS not loaded ---\n");
	}
}

static const char *_ErrorStrings[] = 
{
	" 0 connection lost    ",	// 0
	" 1 struct missmatch   ",	// 1
	" 2 version            ",	// 2
	" 3 alive              ",	// 3
	" 4 pres in hw         ",	// 4
	" 5 pres out hw        ",	// 5
	" 6 pump hw            ",	// 6
	" 7 tank not changing  ",	// 7
	" 8 head not changing  ",	// 8
	" 9 head overheat      ",	// 9
	"10 temp ink overheat  ",	// 10
	"11 heater overheated  ",	// 11	
	"12 err_12             ",	// 12
	"13 temp tank falling  ",	// 13
	"14 temp tank too low  ",	// 14
	"15 p in too high      ",	// 15
	"16 p out too high     ",	// 16
	"17 pump no ink        ",	// 17
	"18 pump watchdog      ",	// 18
	"19 meniscus           ",	// 19
	"20 sensor not calibr  ",	// 20
	"21 download           ",	// 21
	"22 rebooted           ",	// 22
	"23 head termistor nc  ",	// 23
	"24 temp heater nc     ",	// 24
	"25 temp inlet nc      ",	// 25
	"26 power 24V          ",	// 26
	"27 power heater       ",	// 27
	"28 flush failed       ",	// 28
};

//--- putty_display_cond_status ----------------------------
void putty_display_cond_status(int status)
{
	int i, j, l;
	int alternate = 0;
	char str[64];
	char line[90] = {0};
	SHeadEEpromMvt mem;
	static const char ERR[2] = { '.', 'X' };
	static const char STATUS_STRING[2][9] = { " enabled", "disabled" };

	int no[MAX_HEADS_BOARD];

	for(i=0; i<MAX_HEADS_BOARD; i++) no[i]= RX_HBConfig.reverseHeadOrder? MAX_HEADS_BOARD-1-i:i;
		
	/*
	if (arg_simu_conditioner)
	{
		for (i=0; i<SIZEOF(RX_HBStatus->head); i++)
		{
			RX_HBStatus->head[i].tempHead	= (RX_HBStatus->head[i].tempHead+1)%100;
			RX_HBStatus->head[i].tempCond	= 2;
			RX_HBStatus->head[i].presIn		= 3;
			RX_HBStatus->head[i].presOut	= 4;
			RX_HBStatus->head[i].meniscus	= 41;
			RX_HBStatus->head[i].pumpSpeed	= 5;
			RX_HBStatus->head[i].pumpFeedback = 5;
			RX_HBStatus->head[i].printingSeconds= 6;
	//		RX_HBStatus->head[i].ctrlMode	= 7;
		}		
	}
	*/
	if (nios_loaded())
	{		
		term_printf("\n");

		term_printf("-- Conditioner -- "); 		
				
		for (i=0; i<MAX_HEADS_BOARD; i++)
		{			
			strcpy(line, "--------------");
			l = sprintf(str, " %d:%s ", no[i]+1, RX_HBConfig.head[no[i]].name);
			memcpy(&line[(15-l)/2], str, l);
			
			term_color(RX_RGB[no[i]]);
			term_printf(line);
			term_color(BLACK);
			term_printf("  ");
		};		
		term_printf("\n");
		
		term_printf("Version:          ");		
		memset(line, ' ', 5*16);
		for (i=0; i<MAX_HEADS_BOARD; i++)
		{			
			l = sprintf(str, "%lu.%lu.%lu.%lu", RX_NiosStat.cond[no[i]].version.major, RX_NiosStat.cond[no[i]].version.minor, RX_NiosStat.cond[no[i]].version.revision, RX_NiosStat.cond[no[i]].version.build);
			memcpy(&line[16*i+(14-l)], str, l);
		}
		term_printf("%s\n", line);		
		term_printf("SerialNo:        "); PRINTF(MAX_HEADS_BOARD)("         %06d ", _NiosMem->stat.cond[no[i]].serialNo); term_printf("\n");	
		term_printf("alive:           "); PRINTF(MAX_HEADS_BOARD)("         %06d ", RX_NiosStat.cond[no[i]].alive); term_printf("\n");

		term_printf("error:            ");
		for (i=0; i<MAX_HEADS_BOARD; i++)
		{
			term_printf("    ");
			switch(cond_err_level())
			{
			case LOG_TYPE_LOG:			term_color(GREY); break;
			case LOG_TYPE_WARN:			term_color(YELLOW); break;
			case LOG_TYPE_ERROR_CONT:
			case LOG_TYPE_ERROR_STOP:
			case LOG_TYPE_ERROR_ABORT:	term_color(RED);    break;
			default:
				if (RX_NiosStat.cond[no[i]].error)	term_color(GREY);
				else								term_color(BLACK);  
				break;
			}
			term_printf("0x%08x", RX_NiosStat.cond[no[i]].error);
			term_color(BLACK);
			term_printf("  ");
		}
		term_printf("\n");
		
		if (status)
		{			
			for (j = 0; j < SIZEOF(_ErrorStrings); j++)
			{				
				// darkgrey background for every even line 
				if (alternate++ % 2 == 0)
					term_printf("\033[48;5;237m");
				
				term_printf("  %s", _ErrorStrings[j]);
				PRINTF(MAX_HEADS_BOARD)("            %c   ", ERR[(RX_NiosStat.cond[no[i]].error >> j) & 0x01]);
				term_color(BLACK);
				term_printf("\n");
			}
		}
		term_printf("Mode:             ");
		for (i = 0; i < MAX_HEADS_BOARD; i++)
		{
			if (RX_NiosStat.cond[no[i]].info.connected) sprintf(str, "%s(%s)", FluidCtrlModeStr(RX_NiosStat.cond[no[i]].mode), FluidCtrlModeStr(cond_getCtrlMode(no[i])));
			else                              sprintf(str, " x(%s)", FluidCtrlModeStr(cond_getCtrlMode(no[i])));
			term_printf("%14s  ", str);		
		}
		term_printf("\n");
		
		term_printf("Pressure in:     "); 
    	{
			int i, l;
			for (i=0; i<MAX_HEADS_BOARD; i++)
			{
				l   = sprintf(str, "%4s ", value_str1(RX_HBStatus->head[no[i]].presIn));
				if(RX_HBStatus->head[no[i]].presIn_max == INVALID_VALUE)
					l  += sprintf(&str[l], "~%4s", value_str1(RX_HBStatus->head[no[i]].presIn_diff));
				else
					l  += sprintf(&str[l], "^%4s", value_str1(RX_HBStatus->head[no[i]].presIn_max));
				term_printf("%15s ", str);
			}
			term_printf("\n");
    	}
		term_printf("Pressure in2:     "); 
    	{
			int i, l;
			for (i=0; i<MAX_HEADS_BOARD; i++)
			{
				l   = sprintf(str, "%4s ", value_str1(_NiosMem->stat.cond[no[i]].pressure_in2));
				term_printf("%15s ", str);
			}
			term_printf("\n");
    	}
		
		term_printf("meniscus flowRes: "); PRINTF(MAX_HEADS_BOARD)("     %d.%02d (%d.%02d)", _NiosMem->stat.cond[no[i]].flowResistance/100, _NiosMem->stat.cond[no[i]].flowResistance%100, _NiosMem->cfg.cond[no[i]].flowResistance/100, _NiosMem->cfg.cond[no[i]].flowResistance%100); term_printf("\n");
		term_printf("meniscus setpoint:"); PRINTF(MAX_HEADS_BOARD)(" %14s ", value_str1(RX_NiosStat.cond[no[i]].meniscus_setpoint)); term_printf("\n");
		
    	term_printf("Pressure out:    ");
    	{
        	int i, l;
        	for (i = 0; i < MAX_HEADS_BOARD; i++)
        	{
            	l = sprintf(str, "%4s ", value_str1(RX_HBStatus->head[no[i]].presOut));
	        	l  += sprintf(&str[l], "~%4s", value_str1(RX_HBStatus->head[no[i]].presOut_diff));
            	term_printf(" %14s ", str);
        	}
        	term_printf("\n");
    	}
    	
    	term_printf("Pressure diff:   ");
		{
			int i;
			for (i = 0; i < MAX_HEADS_BOARD; i++)
			{
				if (!valid(RX_HBStatus->head[no[i]].presIn) || !valid(RX_HBStatus->head[no[i]].presOut))
					 term_printf("       %8s ", value_str1(INVALID_VALUE));
				else term_printf("       %8s ", value_str1(RX_HBStatus->head[no[i]].presIn - RX_HBStatus->head[no[i]].presOut)); 
			}
			term_printf("\n");
		}
		
		term_printf("Meniscus:        ");
		{
			int i, l;
			for (i=0; i<MAX_HEADS_BOARD; i++)
			{
				l   = sprintf(str, "%4s ", value_str1(RX_HBStatus->head[no[i]].meniscus));						
				l  += sprintf(&str[l], "~%4s", value_str1(RX_HBStatus->head[no[i]].meniscus_diff));
				term_printf(" %14s", str);
				if (RX_NiosStat.cond[no[i]].error&COND_ERR_meniscus) term_printf("E"); else term_printf(" ");
			}
			term_printf("\n");
		}
		
//		PRINTF(MAX_HEADS_BOARD)("     %3s(%3s)%c ", value_str1(RX_HBStatus->head[i].meniscus), value_str1(_NiosMem->cfg.cond[i].pressure_out), ERR[_NiosMem->stat.cond[i].error.meniscus]); term_printf("\n");
    	term_printf("Valve:           "); PRINTF(MAX_HEADS_BOARD)("        f=%d i=%d ", RX_NiosStat.cond[no[i]].info.valve_flush, RX_NiosStat.cond[no[i]].info.valve_ink); term_printf("\n");
    	term_printf("Printed [ml/min]:"); PRINTF(MAX_HEADS_BOARD)("       %8s ", value_str3(_NiosMem->cfg.cond[no[i]].volume_printed * 60)); term_printf("\n");			
		term_printf("Pump [ml/min]:   "); PRINTF(MAX_HEADS_BOARD)("    %5s(%4d) ", value_str1(RX_HBStatus->head[no[i]].pumpFeedback), RX_NiosStat.cond[no[i]].pump); term_printf("\n");
		term_printf("Flow Factor:     "); PRINTF(MAX_HEADS_BOARD)("       %8s ", value_str(RX_HBStatus->head[no[i]].flowFactor)); term_printf("\n");

		term_printf("Printing [h:m:s]: ");
		for (i = 0; i < MAX_HEADS_BOARD; i++)
		{
			if (RX_NiosStat.cond[no[i]].pumptime == INVALID_VALUE)	term_printf("   ----x/x----");
			else 
			{
				int h, m, s;
				s = RX_NiosStat.cond[no[i]].pumptime%60;
				h = RX_NiosStat.cond[no[i]].pumptime/3600;
				m = (RX_NiosStat.cond[no[i]].pumptime%3600)/60;					
				term_printf("   %5d:%02d:%02d  ", h, m, s);
			}
		}
		term_printf("\n");

		for (i = 0; i < MAX_HEADS_BOARD; i++) if (RX_NiosStat.cond[no[i]].error&COND_ERR_temp_head_overheat) str[i]='E'; else str[i]=' ';
		term_printf("Temp Head [C]:    "); PRINTF(MAX_HEADS_BOARD)("   %s(%02d<%02d)%c ", value_str_temp(_NiosCfg->cond[no[i]].tempHead), _NiosCfg->cond[no[i]].temp / 1000, _NiosCfg->cond[no[i]].tempMax / 1000, str[no[i]]); term_printf("\n"); 
		term_printf("Temp Ready:       "); PRINTF(MAX_HEADS_BOARD)("           %d %d ", RX_HBStatus->head[no[i]].info.temp_ready, RX_HBStatus->head[no[i]].info.flowFactor_ok); term_printf("\n"); 		
		term_printf("Temp Inlet [C]:   "); PRINTF(MAX_HEADS_BOARD)("%14s  ",  value_str_temp(RX_NiosStat.cond[no[i]].tempIn)); term_printf("\n");
		term_printf("Temp Heater [C]:  "); PRINTF(MAX_HEADS_BOARD)("%14s  ", value_str_temp(RX_NiosStat.cond[no[i]].tempHeater)); term_printf("\n");
		term_printf("Heater:           "); PRINTF(MAX_HEADS_BOARD)("          %3d%%  ", RX_NiosStat.cond[no[i]].heater_percent); term_printf("\n");
		term_printf("PCB rev:          "); PRINTF(MAX_HEADS_BOARD)("            #%c  ", RX_NiosStat.cond[no[i]].pcb_rev); term_printf("\n");	

    	term_printf("Fluid Pressure:   ");
    	for (i = 0; i < MAX_HEADS_BOARD; i++)
    	{
        	term_printf("    %4s", value_str(RX_FluidStat[no[i]].cylinderPressure));
        	term_printf("(%4s)  ", value_str(RX_FluidStat[no[i]].cylinderPressureSet));
    	}
    	term_printf("\n"); 
    	
    	//term_printf("Fluid Errors:     "); PRINTF(MAX_HEADS_BOARD)("        0x%04x  ", RX_FluidStat[no[i]].fluidErr & 0xffff); term_printf("\n");
		
		term_printf("Meniscus Check:   "); PRINTF(MAX_HEADS_BOARD)("%14s  ", STATUS_STRING[_NiosMem->cfg.cond[no[i]].cmd.disable_meniscus_check]); term_printf("\n"); 
		term_printf("PID sum:          "); PRINTF(MAX_HEADS_BOARD)("%14d  ", _NiosMem->stat.cond[no[i]].pid_sum); term_printf("\n"); 
		term_printf("PID: ");			   PRINTF(MAX_HEADS_BOARD)("(%4d %4d) ", RX_NiosStat.cond[no[i]].pid_P, RX_NiosStat.cond[no[i]].pid_I); term_printf("\n");
							
		if (status)	{term_printf("Heater   pg/flg:   "); PRINTF(MAX_HEADS_BOARD)("            %d/%d ", RX_NiosStat.cond[no[i]].gpio_state.heater_pg, RX_NiosStat.cond[no[i]].gpio_state.heater_flg); term_printf("\n");}
		if (status)	{term_printf("24V      pg/flg:   "); PRINTF(MAX_HEADS_BOARD)("            %d/%d ", RX_NiosStat.cond[no[i]].gpio_state.u_24v_pg, RX_NiosStat.cond[no[i]].gpio_state.u_24v_flg); term_printf("\n");}
		if (status)	{term_printf("Solenoid/Watchdog: "); PRINTF(MAX_HEADS_BOARD)("            %d/%d ", RX_NiosStat.cond[no[i]].info.valve_ink, RX_NiosStat.cond[no[i]].gpio_state.watchdog_reset); term_printf("\n");}
		
//		if (status)	{term_printf("--- LOW LEVEL ERRORS ---\n"); }
//		if (status)	{term_printf("baud rate change err:");term_printf("       %04d", RX_NiosStat.baud_rate_change_error); term_printf("\n"); }
//		if (status)	{term_printf("parity err:        "); PRINTF(MAX_HEADS_BOARD)("         %04d", RX_NiosStat.cond[no[i]].error.parity_error); term_printf("\n"); }
//		if (status)	{term_printf("framin/break Error:"); PRINTF(MAX_HEADS_BOARD)("    %04d/%04d", RX_NiosStat.cond[no[i]].error.framing_error, RX_NiosStat.cond[no[i]].error.break_detected); term_printf("\n"); }
//		if (status)	{term_printf("rx/tx Error:       "); PRINTF(MAX_HEADS_BOARD)("    %04d/%04d", RX_NiosStat.cond[no[i]].error.receive_overrun, RX_NiosStat.cond[no[i]].error.transmit_overrun); term_printf("\n"); }
//		if (status)	{term_printf("--- HIGH LEVEL ERRORS ---\n"); }
//		if (status)	{term_printf("read/write Error:  "); PRINTF(MAX_HEADS_BOARD)("    %04d/%04d", RX_NiosStat.cond[no[i]].error.uart_read_error, RX_NiosStat.cond[no[i]].error.uart_write_error); term_printf("\n");}
//		if (status)	{term_printf("crc/buffer oveflow "); PRINTF(MAX_HEADS_BOARD)("    %04d/%04d", RX_NiosStat.cond[no[i]].error.checksum_error, RX_NiosStat.cond[no[i]].error.buffer_overflow); term_printf("\n"); }
//		if (status)	{term_printf("p_in/p_out Error:  "); PRINTF(MAX_HEADS_BOARD)("    %04d/%04d", RX_NiosStat.cond[no[i]].mcu.p_in_reset_cnt, RX_NiosStat.cond[no[i]].mcu.p_out_reset_cnt); term_printf("\n");}
//		if (status)	{term_printf("p in/out to high:        "); PRINTF(MAX_HEADS_BOARD)("   %01d/%01d       ", RX_NiosStat.cond[no[i]].mcu.cond_error.bitset.p_in_too_high, RX_NiosStat.cond[no[i]].mcu.cond_error.bitset.p_out_too_high); term_printf("\n");}
//		if (status)	{term_printf("p_sensor damaged in/out: "); PRINTF(MAX_HEADS_BOARD)("   %01d/%01d       ", RX_NiosStat.cond[no[i]].mcu.cond_error.bitset.p_in_damage, RX_NiosStat.cond[no[i]].mcu.cond_error.bitset.p_out_damage); term_printf("\n");}
//		if (status)	{term_printf("flash Error/p_purge_high:"); PRINTF(MAX_HEADS_BOARD)("   %01d/%01d       ", RX_NiosStat.cond[no[i]].mcu.cond_error.bitset.flash_write_error, RX_NiosStat.cond[no[i]].mcu.cond_error.bitset.p_in_too_high); term_printf("\n");}
//		if (status)	{term_printf("pump_time_overflow:"); PRINTF(MAX_HEADS_BOARD)("           %01d ", RX_NiosStat.cond[no[i]].mcu.cond_error.bitset.pump_time_count_overflow); term_printf("\n");}
//		if (status)	{term_printf("temp >90 / <10 C: "); PRINTF(MAX_HEADS_BOARD)("          %01d/%01d", RX_NiosStat.cond[no[i]].mcu.cond_error.bitset.thermistor_too_hot, RX_NiosStat.cond[no[i]].mcu.cond_error.bitset.thermistor_too_cold); term_printf("\n");}
//		if (status)	{term_printf("termistor test done/error"); PRINTF(MAX_HEADS_BOARD)("   %01d/%01d       ", RX_NiosStat.cond[no[i]].mcu.info.thermistor_test_done, RX_NiosStat.cond[no[i]].mcu.cond_error.bitset.thermistor_not_close_to_heater); term_printf("\n");}
//		if (status)	{term_printf("\n");}
//		if (status)	{term_printf("unique_id_0        "); PRINTF(MAX_HEADS_BOARD)("   %9d  ", RX_NiosStat.cond[no[i]].unique_id[0]); term_printf("\n");}
//		if (status)	{term_printf("unique_id_1        "); PRINTF(MAX_HEADS_BOARD)("        %4d  ", RX_NiosStat.cond[no[i]].unique_id[1]); term_printf("\n");}
				
		if (status) {
    		term_printf("\n--- Power Status ------------------------------------------------\n"); 
			term_printf("+2.5V: %6s err:%d\n", value_str_u(RX_NiosStat.u_plus_2v5),  RX_NiosStat.error.u_plus_2v5);
			term_printf("+3.3V:        err:%d\n",                                    RX_NiosStat.error.u_plus_3v3);		
			term_printf("+5V:   %6s err:%d\n", value_str_u(RX_NiosStat.u_plus_5v),   RX_NiosStat.error.u_plus_5v);	
			term_printf("-5V:   %6s err:%d\n", value_str_u(RX_NiosStat.u_minus_5v),  RX_NiosStat.error.u_minus_5v);
			if(_NiosStat->info.u_firepulse_48V)
				term_printf("-48V:  %6s err:%d\n", value_str_u(RX_NiosStat.u_firepulse), RX_NiosStat.error.u_firepulse); 
			else
				term_printf("-36V:  %6s err:%d\n", value_str_u(RX_NiosStat.u_firepulse), RX_NiosStat.error.u_firepulse);
			term_printf("\n");
		}
		
		if (status) 
		{
			term_printf("FP AC: err. 0x%01x ", RX_NiosStat.fp_ac_error);
			
			term_printf("FP DC: err. 0x%01x ", RX_NiosStat.fp_dc_error); 

			term_printf("Head EEPROM:"); 
			if (RX_NiosStat.error.head_eeprom_read) term_printf(" ERROR\n"); 
			else                                    term_printf(" OK\n"); 

			term_printf("ADC err. cnt: %d\n", RX_NiosStat.voltage_error); 

			term_printf("   Watchdog in ");
			if (RX_NiosStat.info.watchdog_dbg_mode)	term_printf("Debug Mode");
			else
			{
				term_printf("Normal Mode, "); 
				if (RX_NiosStat.info.watchdog_catch_fp_err) term_printf("catch fp error");
				else										term_printf("ignore fp error");
			}
			term_printf("\n");
		}
	}
}