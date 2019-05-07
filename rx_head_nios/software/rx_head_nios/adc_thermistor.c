/******************************************************************************/
/** \file adc_thermistor.c
 **
 ** read the thermistor value of the conditioner board
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/

#include "rx_ink_common.h"
#include "adc_thermistor.h"


typedef struct
{
	alt_u16 x;
	alt_u32 y;
}dsp_lookup_f_t;

// Saving memory:
//	- reduce further array points
//  - use function "_convert" for both conversions / change tables!

static const dsp_lookup_f_t _temp_head_table[]=
{
{ .x=	483	,	.y=	15000	},
//{ .x=	487	,	.y=	15500	},
{ .x=	490	,	.y=	16000	},
//{ .x=	494	,	.y=	16500	},
{ .x=	498	,	.y=	17000	},
//{ .x=	502	,	.y=	17500	},
{ .x=	506	,	.y=	18000	},
//{ .x=	510	,	.y=	18500	},
{ .x=	514	,	.y=	19000	},
//{ .x=	518	,	.y=	19500	},
{ .x=	522	,	.y=	20000	},
//{ .x=	526	,	.y=	20500	},
{ .x=	530	,	.y=	21000	},
//{ .x=	534	,	.y=	21500	},
{ .x=	538	,	.y=	22000	},
//{ .x=	543	,	.y=	22500	},
{ .x=	547	,	.y=	23000	},
//{ .x=	551	,	.y=	23500	},
{ .x=	555	,	.y=	24000	},
//{ .x=	560	,	.y=	24500	},
{ .x=	564	,	.y=	25000	},
//{ .x=	568	,	.y=	25500	},
{ .x=	573	,	.y=	26000	},
//{ .x=	577	,	.y=	26500	},
{ .x=	581	,	.y=	27000	},
//{ .x=	586	,	.y=	27500	},
{ .x=	590	,	.y=	28000	},
//{ .x=	594	,	.y=	28500	},
{ .x=	599	,	.y=	29000	},
//{ .x=	603	,	.y=	29500	},
{ .x=	608	,	.y=	30000	},
//{ .x=	612	,	.y=	30500	},
{ .x=	617	,	.y=	31000	},
//{ .x=	621	,	.y=	31500	},
{ .x=	626	,	.y=	32000	},
//{ .x=	630	,	.y=	32500	},
{ .x=	635	,	.y=	33000	},
//{ .x=	639	,	.y=	33500	},
{ .x=	644	,	.y=	34000	},
//{ .x=	648	,	.y=	34500	},
{ .x=	653	,	.y=	35000	},
//{ .x=	657	,	.y=	35500	},
{ .x=	662	,	.y=	36000	},
//{ .x=	667	,	.y=	36500	},
{ .x=	671	,	.y=	37000	},
//{ .x=	676	,	.y=	37500	},
{ .x=	680	,	.y=	38000	},
//{ .x=	685	,	.y=	38500	},
{ .x=	689	,	.y=	39000	},
//{ .x=	694	,	.y=	39500	},
{ .x=	699	,	.y=	40000	},
//{ .x=	703	,	.y=	40500	},
{ .x=	708	,	.y=	41000	},
//{ .x=	712	,	.y=	41500	},
{ .x=	717	,	.y=	42000	},
//{ .x=	721	,	.y=	42500	},
{ .x=	726	,	.y=	43000	},
//{ .x=	730	,	.y=	43500	},
{ .x=	735	,	.y=	44000	},
//{ .x=	740	,	.y=	44500	},
{ .x=	744	,	.y=	45000	},
//{ .x=	749	,	.y=	45500	},
{ .x=	753	,	.y=	46000	},
//{ .x=	758	,	.y=	46500	},
{ .x=	762	,	.y=	47000	},
//{ .x=	767	,	.y=	47500	},
{ .x=	771	,	.y=	48000	},
//{ .x=	776	,	.y=	48500	},
{ .x=	780	,	.y=	49000	},
//{ .x=	784	,	.y=	49500	},
{ .x=	789	,	.y=	50000	},
//{ .x=	793	,	.y=	50500	},
{ .x=	798	,	.y=	51000	},
//{ .x=	802	,	.y=	51500	},
{ .x=	806	,	.y=	52000	},
//{ .x=	811	,	.y=	52500	},
{ .x=	815	,	.y=	53000	},
//{ .x=	819	,	.y=	53500	},
{ .x=	824	,	.y=	54000	},
//{ .x=	828	,	.y=	54500	},
{ .x=	832	,	.y=	55000	},
//{ .x=	836	,	.y=	55500	},
{ .x=	841	,	.y=	56000	},
//{ .x=	845	,	.y=	56500	},
{ .x=	849	,	.y=	57000	},
//{ .x=	853	,	.y=	57500	},
{ .x=	857	,	.y=	58000	},
//{ .x=	861	,	.y=	58500	},
{ .x=	866	,	.y=	59000	},
//{ .x=	870	,	.y=	59500	},
{ .x=	874	,	.y=	60000	},
//{ .x=	878	,	.y=	60500	},
{ .x=	882	,	.y=	61000	},
//{ .x=	886	,	.y=	61500	},
{ .x=	890	,	.y=	62000	},
//{ .x=	894	,	.y=	62500	},
{ .x=	898	,	.y=	63000	},
//{ .x=	901	,	.y=	63500	},
{ .x=	905	,	.y=	64000	},
//{ .x=	909	,	.y=	64500	},
{ .x=	913	,	.y=	65000	},
//{ .x=	917	,	.y=	65500	},
{ .x=	921	,	.y=	66000	},
//{ .x=	924	,	.y=	66500	},
{ .x=	928	,	.y=	67000	},
//{ .x=	932	,	.y=	67500	},
{ .x=	935	,	.y=	68000	},
//{ .x=	939	,	.y=	68500	},
{ .x=	943	,	.y=	69000	},
//{ .x=	946	,	.y=	69500	},
{ .x=	950	,	.y=	70000	},
//{ .x=	953	,	.y=	70500	},
{ .x=	957	,	.y=	71000	},
//{ .x=	960	,	.y=	71500	},
{ .x=	964	,	.y=	72000	},
//{ .x=	967	,	.y=	72500	},
{ .x=	971	,	.y=	73000	},
//{ .x=	974	,	.y=	73500	},
{ .x=	977	,	.y=	74000	},
//{ .x=	981	,	.y=	74500	},
{ .x=	984	,	.y=	75000	},
//{ .x=	987	,	.y=	75500	},
{ .x=	991	,	.y=	76000	},
//{ .x=	994	,	.y=	76500	},
{ .x=	997	,	.y=	77000	},
//{ .x=	1000,	.y=	77500	},
{ .x=	1003,	.y=	78000	},
//{ .x=	1006,	.y=	78500	},
{ .x=	1010,	.y=	79000	},
//{ .x=	1013,	.y=	79500	},
{ .x=	1016,	.y=	80000	},
};

static const dsp_lookup_f_t _temp_pt100[] =
{
{ .x=	3624,	.y=	10000	},
//{ .x=	3603,	.y=	11000	},
{ .x=	3581,	.y=	12000	},
//{ .x=	3558,	.y=	13000	},
{ .x=	3534,	.y=	14000	},
//{ .x=	3509,	.y=	15000	},
{ .x=	3484,	.y=	16000	},
//{ .x=	3458,	.y=	17000	},
{ .x=	3432,	.y=	18000	},
//{ .x=	3404,	.y=	19000	},
{ .x=	3376,	.y=	20000	},
//{ .x=	3347,	.y=	21000	},
{ .x=	3318,	.y=	22000	},
//{ .x=	3287,	.y=	23000	},
{ .x=	3256,	.y=	24000	},
//{ .x=	3225,	.y=	25000	},
{ .x=	3192,	.y=	26000	},
//{ .x=	3159,	.y=	27000	},
{ .x=	3126,	.y=	28000	},
//{ .x=	3091,	.y=	29000	},
{ .x=	3056,	.y=	30000	},
//{ .x=	3021,	.y=	31000	},
{ .x=	2985,	.y=	32000	},
//{ .x=	2948,	.y=	33000	},
{ .x=	2911,	.y=	34000	},
//{ .x=	2873,	.y=	35000	},
{ .x=	2835,	.y=	36000	},
//{ .x=	2797,	.y=	37000	},
{ .x=	2758,	.y=	38000	},
//{ .x=	2718,	.y=	39000	},
{ .x=	2679,	.y=	40000	},
//{ .x=	2639,	.y=	41000	},
{ .x=	2598,	.y=	42000	},
//{ .x=	2558,	.y=	43000	},
{ .x=	2517,	.y=	44000	},
//{ .x=	2476,	.y=	45000	},
{ .x=	2435,	.y=	46000	},
//{ .x=	2394,	.y=	47000	},
{ .x=	2353,	.y=	48000	},
//{ .x=	2312,	.y=	49000	},
{ .x=	2271,	.y=	50000	},
//{ .x=	2229,	.y=	51000	},
{ .x=	2186,	.y=	52000	},
//{ .x=	2147,	.y=	53000	},
{ .x=	2106,	.y=	54000	},
//{ .x=	2066,	.y=	55000	},
{ .x=	2025,	.y=	56000	},
//{ .x=	1985,	.y=	57000	},
{ .x=	1945,	.y=	58000	},
//{ .x=	1905,	.y=	59000	},
{ .x=	1866,	.y=	60000	},
//{ .x=	1826,	.y=	61000	},
{ .x=	1788,	.y=	62000	},
//{ .x=	1749,	.y=	63000	},
{ .x=	1711,	.y=	64000	},
//{ .x=	1674,	.y=	65000	},
{ .x=	1637,	.y=	66000	},
//{ .x=	1600,	.y=	67000	},
{ .x=	1564,	.y=	68000	},
//{ .x=	1529,	.y=	69000	},
{ .x=	1493,	.y=	70000	},
//{ .x=	1459,	.y=	71000	},
{ .x=	1425,	.y=	72000	},
//{ .x=	1391,	.y=	73000	},
{ .x=	1358,	.y=	74000	},
//{ .x=	1326,	.y=	75000	},
{ .x=	1294,	.y=	76000	},
//{ .x=	1263,	.y=	77000	},
{ .x=	1232,	.y=	78000	},
//{ .x=	1202,	.y=	79000	},
{ .x=	1173,	.y=	80000	},
//{ .x=	1144,	.y=	81000	},
{ .x=	1115,	.y=	82000	},
//{ .x=	1088,	.y=	83000	},
{ .x=	1060,	.y=	84000	},
//{ .x=	1034,	.y=	85000	},
{ .x=	1008,	.y=	86000	},
//{ .x=	982	,	.y=	87000	},
{ .x=	957	,	.y=	88000	},
//{ .x=	933	,	.y=	89000	},
{ .x=	909	,	.y=	90000	},
//{ .x=	886	,	.y=	91000	},
{ .x=	863	,	.y=	92000	},
//{ .x=	841	,	.y=	93000	},
{ .x=	819	,	.y=	94000	},
//{ .x=	798	,	.y=	95000	},
{ .x=	778	,	.y=	96000	},
//{ .x=	758	,	.y=	97000	},
{ .x=	738	,	.y=	98000	},
//{ .x=	719	,	.y=	99000	},
{ .x=	700	,	.y=	100000	},
//{ .x=	682	,	.y=	101000	},
{ .x=	664	,	.y=	102000	},
//{ .x=	647	,	.y=	103000	},
{ .x=	630	,	.y=	104000	},
//{ .x=	614	,	.y=	105000	},
{ .x=	598	,	.y=	106000	},
//{ .x=	583	,	.y=	107000	},
{ .x=	569	,	.y=	108000	},
//{ .x=	553	,	.y=	109000	},
{ .x=	538	,	.y=	110000	},
//{ .x=	524	,	.y=	111000	},
{ .x=	511	,	.y=	112000	},
//{ .x=	498	,	.y=	113000	},
{ .x=	485	,	.y=	114000	},
//{ .x=	472	,	.y=	115000	},
{ .x=	460	,	.y=	116000	},
//{ .x=	448	,	.y=	117000	},
{ .x=	437	,	.y=	118000	},
//{ .x=	426	,	.y=	119000	},
{ .x=	415	,	.y=	120000	},
//{ .x=	404	,	.y=	121000	},
{ .x=	394	,	.y=	122000	},
//{ .x=	384	,	.y=	123000	},
{ .x=	374	,	.y=	124000	},
//{ .x=	364	,	.y=	125000	},
{ .x=	355	,	.y=	126000	},
//{ .x=	346	,	.y=	127000	},
{ .x=	337	,	.y=	128000	},
//{ .x=	329	,	.y=	129000	},
};

//static UINT32 _temp_head_x[]={1,2,3,4,5};
//static UINT32 _temp_pt100_x[]={1,2,3,4,5};

//---prototypes --------------------------------------
static alt_u32 _dsp_lookup_f(const dsp_lookup_f_t * table, alt_u32 x, alt_u32 min_y, alt_u32 max_y);
//static UINT32 _convert(UINT32 val, UINT32 *x, int size, UINT32 yMin, UINT32 dy);
static UINT32 _handle_temp_val(UINT32 rowVal);

/**
 ******************************************************************************
 ** \brief  ADC0 single conversion start
 **
 ** \return  uint32_t   ADC result
 ******************************************************************************/
UINT32 convert_head_temp(UINT32 value)
{
//	return _convert(value, _temp_head_x, SIZEOF(_temp_head_x), 15000, 1000);

	int result=0;
	int maxindex = SIZEOF(_temp_head_table)-1;

	result = _dsp_lookup_f(_temp_head_table, value, 0, maxindex);
	if((result == _temp_head_table[0].y ) || (result == _temp_head_table[maxindex].y))
		return INVALID_VALUE;
	else
		return result;
}

//--- convert_pt100 -------------------------------
UINT32 convert_pt100(UINT32 value)
{
	//	return _convert(value, _temp_pt100_x, SIZEOF(_temp_pt100_x), 128000, -2000);

	int result=0;
	int maxindex = SIZEOF(_temp_pt100)-1;

	if (value == INVALID_VALUE)
		result = INVALID_VALUE;
	else
		result = _handle_temp_val(value);

	if((result == _temp_pt100[0].y) || (result == _temp_pt100[maxindex].y))
		result = INVALID_VALUE;

	return result;
}

/**
 ******************************************************************************
 ** \brief  thermistor lookup table
 **
 ** \return  uint32_t   ADC result
 ******************************************************************************/
static alt_u32 _dsp_lookup_f(const dsp_lookup_f_t * table, alt_u32 x, alt_u32 min_y, alt_u32 max_y)
{
	alt_u32 i = 0;
	alt_u32 m, delta_x, delta_y;

	while ((x > table[i].x) && (i < max_y))
		i++;

	if (i >= max_y)
		return table[max_y].y;

	if (i <= min_y)
		return table[min_y].y;

	if(x < table[i].x) // between two table values
	{
		// y = y0 + (x - x0) * ((y1 - y0) / (x1 - x0)) where x0, x1 are nearest values of input x
		// y0, y1 are nearest values to output y
		delta_x = table[i].x - table[i-1].x;
		delta_y = (table[i].y - table[i-1].y);
		m = (delta_y / delta_x);
		m = table[i-1].y + (x - table[i-1].x) * m;
	}
	else	// exactly on a table value
	{
		m = table[i].y;
	}

	return m;
}

/**
 * \brief Handler function when new temperature is read from ADC
 **/
static UINT32 _handle_temp_val(UINT32 rowVal)
{
    UINT32 i          = 0;
    UINT32 m          = 0;
    UINT32 delta_x    = 0;
    UINT32 delta_y    = 0;
	UINT32 val = INVALID_VALUE;
	UINT32 table_size = SIZEOF(_temp_pt100);

    if (rowVal >= _temp_pt100[0].x)
        return INVALID_VALUE;

    if (rowVal <= _temp_pt100[table_size - 1].x)
    	return INVALID_VALUE;

	//--- convert value -----------------------
    // find the two points in the table to use
    for (i = 0; i < table_size; i++)
	{
		if (rowVal>_temp_pt100[i].x)
		{
            /*
             * y = y0 + (x - x0) * ((y1 - y0) / (x1-x0))
             * where x0, x1 are nearest values of input x
             * y0, y1 are nearest values to output y
             */
			delta_x = _temp_pt100[i-1].x - _temp_pt100[i].x;
			delta_y = (_temp_pt100[i].y - _temp_pt100[i-1].y);
            m       = delta_y / delta_x;
			val 	= _temp_pt100[i-1].y - (rowVal - _temp_pt100[i-1].x)*m;
			break;
		}
	}

    return val;
}

//--- convert -------------------------------------------------
/*static UINT32 _convert(UINT32 val, UINT32 *x, int size, UINT32 yMin, UINT32 dy)
{
	//	convert tables _temp_head_table and _temp_pt100 (must be ascending)

	int i;

	if (val<x[0]) 		return INVALID_VALUE;
	if (val>x[size-2])	return INVALID_VALUE; // need one value above for interploation

    for (i=0; val<x[i]; i++){};

    // now:	x[i] < val < x[i+1]

    return 	(yMin+i*dy) + dy*(val-x[i]) / (x[i+1]-x[i]);
}*/
