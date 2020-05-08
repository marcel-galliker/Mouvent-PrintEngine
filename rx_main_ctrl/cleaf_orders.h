// ****************************************************************************
//
//	cleaf_orders.h		
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------

#include "rx_common.h"
#include "rx_def.h"
#include "rx_setup_file.h"
#include "rx_sok.h"

#define PATH_CLEAF_ORDERS	PATH_HOME "orders/"

#pragma pack(1)
typedef struct SCleafFlexo	
{
	char lacCode[32];
	char anilox[32];
	char robberRoll[32];
} SCleafFlexo;

typedef struct SCleafOrder
{
	char	id[32];
	char	code[32];
	char	absCode[32];
	char	material[32];
	INT32	speed;
	INT32	length;
	double	printHeight;
	char	printFile[32];
	INT32	gloss;
	char	note[256];
	SCleafFlexo flexo[4];
} SCleafOrder;

typedef struct SCleafProduction
{
	INT32	produced;
	INT32	waste;
	INT32	inkUsed[4];
} SCleafProduction;

typedef struct SCleafRoll	
{
	INT32	no;
	double	length;
	INT32	quality;
	#define CO_QUALITY_UNDEF 0
	#define CO_QUALITY_WASTE 1
	#define CO_QUALITY_OK	 2
	char	operator[64];	
} SCleafRoll;

#pragma pack()

extern SCleafOrder		RX_CleafOrder;
extern SCleafProduction	RX_CleafProd;
extern SCleafRoll		RX_CleafRoll[256];

void co_init			();
void co_tick			(void);

int co_read_order		(char *id, SCleafOrder *porder);
int co_setup_production	(char *id, EN_setup_Action action, SCleafProduction *pprod, SCleafRoll *roll, int size, int *pcnt);

void co_set_order(char *id);
void co_set_operator(char *name);
void co_send_order(RX_SOCKET socket);
void co_set_roll(SValue* pvalue);

void co_start_printing(int pageLength);
void co_stop_printing(void);
void co_printed(void);
void co_roll_end(void);

