#ifndef _TW8_REG_H_
#define _TW8_REG_H_

#include <unistd.h>

#include "rx_common.h"

#ifdef TW8
	typedef UINT8  __int8_t;
	typedef UINT16 __uint16_t;
#endif
// Datatypes
// Disable padding
#pragma pack(push, 1)

//--- memory allocation ------------------
// 0x0000 – 07FFF NU Not Used 	
// 0x8000 – 0x8041 LUT Lookup Table Data 
#define ADDR_CFG	0x8042	// 0x8042 – 0x8067 CFG Configuration Data 
#define ADDR_PAR_NOW	0x806A  // 0x8068	// 0x8068 – 0x8093 PAR Signal Path Parameters current parameters
#define ADDR_PAR_BAK	0x8078  // 0x8078 – 0x8085 PAR Signal Path Parameters Backup Values at powerup
//#define ADDR_PAR_BASE	0x8086  // 0x8086 – 0x8093 PAR Signal Path Parameters  baseline parameter values -> factory default
#define ADDR_VAR	0x809A  // 0x8094	// 0x8094 – 0x9FFF VAR Working Variables 
#define ADDR_RB		0xA000	// 0xA000 – 0xA0FF RB Register Banks 
#define ADDR_WM		0xA100	// 0xA100 – 0xFFFF WM Working Memory 
#define ADDR_POS	0xA18E	// 

//--- see TW8_PR_datasheet_B4en.pdf Page PR8/39 
typedef struct STW8_PAR_Regs
{
	//				0x806A – 0x8077 PAR_NOW Current Values 
	//				0x8078 – 0x8085 PAR_BAK Backup Values
	//				0x8086 – 0x8093 PAR_BASE Baseline Values 
	UINT16	CKSUM;	// 0x806A / 00
	UINT16	DGAIN;	// 0x806C / 02	
	UINT16	DOFFS;	// 0x806E / 04
	UINT16	DOFFC;	// 0x8070 / 06
	UINT16	DPH;	// 0x8072 / 08
	UINT8	AGAIN;	// 0x8074 / 0A
	UINT8	AOFFS;	// 0x8075 / 0B
	UINT8	AOFFC;	// 0x8076 / 0C
	UINT8	res;    // 0x8077 / 0C
} STW8_PAR_Regs;

typedef struct STW8_VAR_Regs
{
	//UINT16	STAT_SP;	// 0x809A
	union				//	0x809A
	{
		UINT16	reg;	
		struct
		{
			UINT8 adcof : 1;
			UINT8 adcuf : 1;
			UINT8 adapt : 1;
			UINT8 vtop : 1;
			UINT8 ospeed : 1;
			UINT8 fatal : 1;
			UINT8 lag : 1;
			UINT8 ab : 1;
			UINT8 res_8 : 1;
			UINT8 one_wire : 1;
			UINT8 res_10 : 1;
			UINT8 xtal : 1;
			UINT8 res_12 : 1;
			UINT8 res_13 : 1;
			UINT8 res_14 : 1;
			UINT8 res_15 : 1;
		};
	} STAT_SP;
	//UINT16	STAT_EE;	// 0x809C
	union				//	0x809C
	{
		UINT16	reg;	
		struct
		{
			UINT8 timeout : 1;
			UINT8 stuck : 1;
			UINT8 id : 1;
			UINT8 cfg : 1;
			UINT8 par0 : 1;
			UINT8 par1 : 1;
			UINT8 base : 1;
			UINT8 lut : 1;
			UINT8 mode : 1;
			UINT8 res_9 : 1;
			UINT8 res_10 : 1;
			UINT8 res_11 : 1;
			UINT8 res_12 : 1;
			UINT8 res_13 : 1;
			UINT8 res_14 : 1;
			UINT8 res_15 : 1;
		};
	} STAT_EE;
	UINT16	res_9e;		// 0x809E
	UINT16	res_a0;		// 0x80A0
	UINT16	res_a2;		// 0x80A2	
	UINT16	ADC_SIN;	// 0x80A4
	UINT16	ADC_COS;	// 0x80A6
	UINT8	ADC_AMP;	// 0x80A8
	UINT8	res_a9;
	UINT8	res_aa;
	UINT8	res_ab;
	UINT8	res_ac;
	UINT8	res_ad;
	UINT8	QM_ADC;		// 0x80AE
	UINT8	QM_ADAPT;	// 0x80AF
	UINT8	CMD;		// 0x80B0
	UINT8	res_var[0xA000-0x80B1];
} STW8_VAR_Regs;

typedef union
{
	UINT8	reg;
	struct
	{
		UINT8	limit :4;
		UINT8	th:4;
	};
} SMON_CFG;

//--- see TW8_PR_datasheet_B4en.pdf Page PR13/39 
typedef struct STW8_CFG_Regs
{
	UINT16	CFG_CKSUM;	//	0x8042 
	union				//	0x8044
	{
		UINT8	reg;	
		struct
		{
			UINT8 wp:1;
			UINT8 bit_1:1;
			UINT8 pull:1;
			UINT8 clkout:1;
		};
	} MAIN_CFG;
	
//	UINT8	MAIN_DSM;	//	0x8045	
	union				
	{
		UINT8	reg;	
		struct
		{
			UINT8	freq:3;
			UINT8	calmp:1;				
		};
	} MAIN_DSM;
	
	UINT16	MAIN_INTER;	//	0x8046
	UINT16	MAIN_HYST;	//	0x8048
//	UINT8	MAIN_FLTR;	//	0x804A
	union
	{
		UINT8	reg;
		struct
		{
			UINT8	fb:1;
			UINT8	p:3;
			UINT8	i:3;
		};
	} MAIN_FLTR;	
//	UINT8	MAIN_OUT;	//	0x804B
	union
	{
		UINT8	reg;
		struct
		{
			UINT8	start:4;
			UINT8	mode:2;
		};
	} MAIN_OUT;
//	UINT8	MAIN_CLOCK;	//	0x804C
	union
	{
		UINT8	reg;
		struct
		{
			UINT8	xtal:1;
			UINT8	div:2;
			UINT8	freq:4;
			UINT8	xforce:1;
		};
	} MAIN_CLOCK;
	UINT8	res_4d;		//	0x804D
	UINT16	MAIN_ZPOS;	//	0x804E
//	UINT8	MAIN_Z;		//	0x8050
	union
	{
		UINT8	reg;
		struct
		{
			UINT8	th:5;
			UINT8	reset:1;				
		};
	} MAIN_Z;
	UINT8	res_51;		//	0x8051
//	UINT8	AB_CFG0;	//	0x8052
	union
	{
		UINT8 reg;
		struct
		{
			UINT8	dir:1;
			UINT8	apol:1;
			UINT8	bpol:1;
			UINT8	zpol:1;
			UINT8	bit_4:1;
			UINT8	startmode:2;
			UINT8	bit_7:1;
		};
	} AB_CFG0;
//	UINT8	AB_CFG1;	//	0x8053
	union
	{
		UINT8 reg;
		struct
		{
			UINT8	div:5;
		};
	} AB_CFG1;
	UINT8	AB_ZWIDTH;	//	0x8054
	UINT8	AB_VTOP;	//	0x8055
	UINT8	res_56;		//	0x8056
	UINT8	res_57;		//	0x8057
//	UINT8	ADPT_CFG;	//	0x8058 
	union
	{
		UINT8	reg;
		struct
		{
			UINT8	doff:1;			
			UINT8	dgain:1;
			UINT8	dphase:1;
			UINT8	aoff:1;
			UINT8	again:1;
			UINT8	lut:1;
			UINT8	store:1;
		};
	} ADPT_CFG;
//	UINT8	ADPT_DETAIL;//	0x8059 
	union
	{
		UINT8	reg;
		struct
		{
			UINT8	flimit:3;
			UINT8	tbase:3;
			UINT8	fault:1;
		};
	} ADPT_DETAIL;
//	UINT8	ADPT_STORE;	//	0x805A
	union
	{
		UINT8	reg;
		struct
		{
			UINT8	offth:4;
			UINT8	gainth:4;
		};
	} ADPT_STORE;
//	UINT8	ADPT_CORR;	//	0x805B
	union
	{
		UINT8	reg;
		struct
		{
			UINT8	prop:2;
			UINT8	offtol:3;
			UINT8	gaintol:3;
		};
	} ADPT_CORR;
	UINT8	res_5c;		//	0x805c
//	UINT8	MON_CFG;	//	0x805D
	union
	{
		UINT8	reg;
		struct
		{
			UINT8	adc:1;
			UINT8	adapt:1;
			UINT8	pwm:1;
			UINT8	bit_4:1;
			UINT8	bit_5:1;
			UINT8	adcth:1;
		};
	} MON_CFG;
	SMON_CFG	MON_ADC;	//	0x805E
	SMON_CFG	MON_OFF;	//	0x805F
	SMON_CFG	MON_GAIN;	//	0x8060
	SMON_CFG	MON_PHASE;	//	0x8061
//	UINT8	FLT_CFG;	//	0x8062
	union
	{
		UINT8	reg;
		struct
		{
			UINT8	vwarn:1;
			UINT8	hold:2;
			UINT8	pol:1;
			UINT8	long_:1;
		};
	} FLT_CFG;
	UINT8	res_63;		//	0x8063
//	UINT8	FLT_STAT;	//	0x8064
	union
	{
		UINT8	reg;
		struct
		{
			UINT8	ee:1;
			UINT8	adc:1;
			UINT8	adapt:1;
			UINT8	vwarn:1;
			UINT8	vfatal:1;
			UINT8	bit_5:1;
			UINT8	intern:1;
		};
	} FLT_STAT;
	UINT8	res_65;		//	0x8065
//	UINT8	FLT_EN;		//	0x8066
	union
	{
		UINT8	reg;
		struct
		{
			UINT8	ee:1;
			UINT8	adc:1;
			UINT8	adapt:1;
			UINT8	vwarn:1;
			UINT8	vfatal:1;
			UINT8	bit_5:1;
			UINT8	intern:1;
		};
	} FLT_EN;
	UINT8	res_67;		//	0x8067
} STW8_CFG_Regs;

//--- see TW8_PR_datasheet_B4en.pdf Page PR9/39 
typedef struct STW8_WM_PosRegs
{
	UINT32	pos;	//	0xA18E
	INT16	speed;	//	0xA192	
} STW8_WM_PosRegs;

	// Go back to default padding
#pragma pack(pop)

#endif /* _TW8_H_ */
