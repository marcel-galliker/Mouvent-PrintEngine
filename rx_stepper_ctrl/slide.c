// ****************************************************************************
//
//	DIGITAL PRINTING - test_table.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2016 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "rx_def.h"
#include "rx_term.h"
#include "rx_threads.h"
#include "test_table.h"
#include "fpga_stepper.h"
#include "slide.h"

//--- commands ----------------------------------------
#define SLIDE_CMD_IDLE			0x00
#define SLIDE_CMD_REFERENCE		0x01
#define SLIDE_CMD_MOVE_TABLE	0x02
#define SLIDE_CMD_MOVE_ADJUST	0x03
#define SLIDE_CMD_SCAN_LEFT		0x04
#define SLIDE_CMD_SCAN_RIGHT	0x05

#define SIGNAL_WAIT_TIME		50 // 20

//--- statics ------------------------------------------
static int _SlideSpeed = 0;
static int _SlideCmd   = SLIDE_CMD_IDLE;
static int _WaitCmd	   = 0;
static int _WaitArg	   = 0;

static int _Position   = 0;

//--- prototypes ---------------------------------------
static void _slide_checks(void);


//--- slide_init -----------------------------------------
void slide_init(void)
{
	Fpga.par->output |= SLIDE_ENABLE; 
}

//--- slide_end -------------------------------------------
void slide_end(void)
{
}

//--- slide_ref_done --------------------------------------
int slide_ref_done(void)
{
	return (Fpga.stat->input&SLIDE_REF_DONE_IN) != 0;
}

//--- slide_move_done ----------------------------------
int  slide_move_done(void)
{
	int  done = (Fpga.stat->input&SLIDE_MOVING_IN) == 0;	
	if (done) _SlideCmd = SLIDE_CMD_IDLE;
	return done;
}

//--- slide_error --------------------------------------
int  slide_error(void)
{
	return (Fpga.stat->input&SLIDE_ERROR_IN) != 0;			
}

//--- slide_get_pos -----------------------------------
int	 slide_get_pos(void)
{
	return _Position;
}

//--- slide_set_speed ---------------------------------
void slide_set_speed(int speed)
{
	_SlideSpeed = speed;
}

//--- _slide_set_cmd ------------------------------
void _slide_set_cmd(int cmd, int speed)
{
	UINT32 speedBits = 0;
	UINT32 cmdBits	 = 0;

	if (_SlideCmd!=SLIDE_CMD_IDLE)
	{
		_WaitCmd = cmd;
		_WaitArg = speed;
		return;
	}
	/*
    if (Fpga.stat->input & SLIDE_MOVING_IN) 
	{
		printf("Got command while slide still moving.");
		return; 
	}
	*/
	
	if (speed&0x01) speedBits |= SLIDE_SPEED0_OUT;
	if (speed&0x02) speedBits |= SLIDE_SPEED1_OUT;
	if (speed&0x04) speedBits |= SLIDE_SPEED2_OUT;
	if (speed&0x08) speedBits |= SLIDE_SPEED3_OUT;

	Fpga.par->output = (Fpga.par->output & ~(SLIDE_SPEED0_OUT | SLIDE_SPEED1_OUT |SLIDE_SPEED2_OUT | SLIDE_SPEED3_OUT)) | speedBits;


	if (cmd&0x01) cmdBits |= SLIDE_CMD0_OUT;
	if (cmd&0x02) cmdBits |= SLIDE_CMD1_OUT;
	if (cmd&0x04) cmdBits |= SLIDE_CMD2_OUT;
	if (cmd&0x08) cmdBits |= SLIDE_CMD3_OUT;

	Fpga.par->output = (Fpga.par->output & ~(SLIDE_CMD0_OUT | SLIDE_CMD1_OUT |SLIDE_CMD2_OUT | SLIDE_CMD3_OUT )) | cmdBits;

	if (!(Fpga.par->output & SLIDE_ENABLE))
	{
		Fpga.par->output |= SLIDE_ENABLE;
		rx_sleep(SIGNAL_WAIT_TIME);
	}

	Fpga.par->output |= SLIDE_CMD_EXEC_OUT;
	rx_sleep(SIGNAL_WAIT_TIME);
	Fpga.par->output &= ~SLIDE_CMD_EXEC_OUT;

	_SlideCmd = cmd;
}

//--- slide_start_ref --------------------------------
void slide_start_ref(void)
{
	_slide_set_cmd(SLIDE_CMD_REFERENCE, _SlideSpeed);
}

//--- slide_scan_right ----------------------------
void slide_scan_right(void)
{
	_Position = 1000;
	_slide_set_cmd(SLIDE_CMD_SCAN_RIGHT, _SlideSpeed);
}

//--- slide_scan_left -----------------------------
void slide_scan_left(void)
{
	_Position = 0;
	_slide_set_cmd(SLIDE_CMD_SCAN_LEFT, _SlideSpeed);
}

//--- slide_move_table ----------------------------
void slide_move_table(void)
{
	_Position = 500;
	_slide_set_cmd(SLIDE_CMD_MOVE_TABLE, _SlideSpeed);
}

//--- slide_move_adjust -------------------------------
void slide_move_adjust(int head)
{
	_Position = 100+10*head;
	_slide_set_cmd(SLIDE_CMD_MOVE_ADJUST, head);
}

//--- slide_stop ----------------------------------
void slide_stop(void)
{
	Fpga.par->output &= ~SLIDE_ENABLE;
}

//--- __level ------------------------------------------------------------------
static const char *_level(int level)
{
	static const char* _ON="*";
	static const char* _OFF="_";
	if (level) return _ON;
	else       return _OFF;
}

//--- slide_main -------------------------------------
void slide_main(int ticks, int menu)
{
	_slide_checks();
	if (menu)
	{
		term_printf("Slide Inputs ---------------------------------\n");
		term_printf("01: CAP up:       %s       07: SLIDE Running:       %s\n",	_level(Fpga.stat->input&0x0001),	_level(Fpga.stat->input&0x0040));
		term_printf("02: CAP down:     %s       08: SLIDE Error:         %s\n",	_level(Fpga.stat->input&0x0002),	_level(Fpga.stat->input&0x0080));
		term_printf("03: Z-Axis Ref:   %s       09: SLIDE Ref Done:      %s\n",	_level(Fpga.stat->input&0x0004),	_level(Fpga.stat->input&0x0100));
		term_printf("04: Head Down:    %s       10: SLIDE Moving:        %s\n",	_level(Fpga.stat->input&0x0008),	_level(Fpga.stat->input&0x0200));
		term_printf("05: Cover Closed: %s       11: TT END left:         %s\n",	_level(Fpga.stat->input&0x0010),	_level(Fpga.stat->input&0x0400));
		term_printf("06: E-STOP:       %s       12: TT END right         %s\n",	_level(Fpga.stat->input&0x0020),	_level(Fpga.stat->input&0x0800));
		term_printf("\n");

		term_printf("Slide Outputs --------------------------------\n");
		term_printf("01: Vacuum:       %s       07: SLIDE Speed3:        %s\n",	_level(Fpga.par->output&0x0001),	_level(Fpga.par->output&0x0040));
		term_printf("02: Cooling:      %s       08: SLIDE CMD0:          %s\n",	_level(Fpga.par->output&0x0002),	_level(Fpga.par->output&0x0080));
		term_printf("03: SLIDE Enable: %s       09: SLIDE CMD1:          %s\n",	_level(Fpga.par->output&0x0004),	_level(Fpga.par->output&0x0100));
		term_printf("04: SLIDE Speed0: %s       10: SLIDE CMD2:          %s\n",	_level(Fpga.par->output&0x0008),	_level(Fpga.par->output&0x0200));
		term_printf("05: SLIDE Speed1: %s       11: SLIDE CMD3:          %s\n",	_level(Fpga.par->output&0x0010),	_level(Fpga.par->output&0x0400));
		term_printf("06: SLIDE Speed2: %s       12: SLIDE CMD Execute:   %s\n",	_level(Fpga.par->output&0x0020),	_level(Fpga.par->output&0x0800));
		term_printf("speed=%d, slideCmd=%d, waitCmd=0x%08x\n", _SlideSpeed, _SlideCmd, _WaitCmd);
		term_printf("\n");

		term_flush();
	}

	//--- reset outputs ---------------
	if (Fpga.stat->input&SLIDE_MOVING_IN)
	{
	//	Fpga.par->output &= ~(SLIDE_REF_START | SLIDE_SCAN_RIGHT_OUT | SLIDE_SCAN_LEFT_OUT | SLIDE_MOVE_TABLE_OUT); 
	}
}

//--- _slide_checks --------------------------------------
static void _slide_checks(void)
{
	Fpga.par->watchdog_cnt	= 50000000;

	if (slide_move_done() && _WaitCmd)
	{
		rx_sleep(100);
		_slide_set_cmd(_WaitCmd, _WaitArg);
		_WaitCmd=0;
	}

	/*
	if (Fpga.stat->input&LD_ESTOP_IN)	Fpga.par->output  = 0;
	else                                Fpga.par->output |= SLIDE_ENABLE; 
	*/
}
