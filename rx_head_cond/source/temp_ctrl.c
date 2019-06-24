/**************************************************************************/ /**
 \file temp_ctrl.c

 Temperature Controlling for head-tank and jetting assembly.

 Read the thermistor value of the conditioner board and control the
 integrated heater depending on the measured temperature from the head.

 \author      Cyril Andreatta
 \author      Stefan Weber

 \copyright   2017 by radex AG, Switzerland. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "temp_ctrl.h"
#include "IOMux.h"
#include "average.h"
#include "cond_def_head.h"
#include "debug_printf.h"
#include "watchdog.h"
#include "main.h"
#include "pid.h"

//@{
/** defines *******************************************************************/

// Production test software for Walter which heats up without a connected head

//#define LOG_TEMP    TRUE
#define TEMP_MAX		 50000			// maximal valid conditioner temperature
#define TEMP_ERROR       TEMP_MAX+5000  // in 1/1000 °C = 90 °C -> compare to thermistor value directly

#define VALUE_BUF_SIZE		10

#define ADC_INVL	0x1000
#define ADC_DATA	0xFFF0
#define ADC_CHAN	0x001F 

#define OFF 0

#define TANK_TEMP_TIMEOUT           40000   // * 50 ms
#define TANK_TEMP_FALLING_TIMEOUT   2000    // * 50 ms
#define HEAD_TEMP_TIMEOUT           18000   // * 50 ms -> 15min
#define RANGE_TIMEOUT               30000   // * 50 ms

#define ABSOLUTE_MIN_INK_TEMP       10000     // in 0.1 °C = 10 °C
//#define HEATER_DELAY                6000   // do not start heating right away, 1 minute delay
#define TEMP_TANK_THRESHOLD         1       // in 0.1 °C
#define TEMP_HEAD_THRESHOLD         1       // in 0.1 °C
#define TEMP_HEAD_TARGET_RANGE      0.05    // +/-% of target temperature
#define TEMP_HEAD_OVERSHOOT         0.25    // +%  of target temperature


#define GRAD_MAX_INDEX 10
// measured if thermistor is unconnected

#define _TEMP_RAMPUMP				60000	//in 10mS, 10 minutes

//@}

//@{
/** private types *************************************************************/

typedef struct
{
    UINT32 x;
    UINT32 y;
}dsp_lookup_f_t;  

const dsp_lookup_f_t _temp_table[] =
{					
{ .x=	3624,	.y=	10000	},
{ .x=	3603,	.y=	11000	},
{ .x=	3581,	.y=	12000	},
{ .x=	3558,	.y=	13000	},
{ .x=	3534,	.y=	14000	},
{ .x=	3509,	.y=	15000	},
{ .x=	3484,	.y=	16000	},
{ .x=	3458,	.y=	17000	},
{ .x=	3432,	.y=	18000	},
{ .x=	3404,	.y=	19000	},
{ .x=	3376,	.y=	20000	},
{ .x=	3347,	.y=	21000	},
{ .x=	3318,	.y=	22000	},
{ .x=	3287,	.y=	23000	},
{ .x=	3256,	.y=	24000	},
{ .x=	3225,	.y=	25000	},
{ .x=	3192,	.y=	26000	},
{ .x=	3159,	.y=	27000	},
{ .x=	3126,	.y=	28000	},
{ .x=	3091,	.y=	29000	},
{ .x=	3056,	.y=	30000	},
{ .x=	3021,	.y=	31000	},
{ .x=	2985,	.y=	32000	},
{ .x=	2948,	.y=	33000	},
{ .x=	2911,	.y=	34000	},
{ .x=	2873,	.y=	35000	},
{ .x=	2835,	.y=	36000	},
{ .x=	2797,	.y=	37000	},
{ .x=	2758,	.y=	38000	},
{ .x=	2718,	.y=	39000	},
{ .x=	2679,	.y=	40000	},
{ .x=	2639,	.y=	41000	},
{ .x=	2598,	.y=	42000	},
{ .x=	2558,	.y=	43000	},
{ .x=	2517,	.y=	44000	},
{ .x=	2476,	.y=	45000	},
{ .x=	2435,	.y=	46000	},
{ .x=	2394,	.y=	47000	},
{ .x=	2353,	.y=	48000	},
{ .x=	2312,	.y=	49000	},
{ .x=	2271,	.y=	50000	},
{ .x=	2229,	.y=	51000	},
{ .x=	2186,	.y=	52000	},
{ .x=	2147,	.y=	53000	},
{ .x=	2106,	.y=	54000	},
{ .x=	2066,	.y=	55000	},
{ .x=	2025,	.y=	56000	},
{ .x=	1985,	.y=	57000	},
{ .x=	1945,	.y=	58000	},
{ .x=	1905,	.y=	59000	},
{ .x=	1866,	.y=	60000	},
{ .x=	1826,	.y=	61000	},
{ .x=	1788,	.y=	62000	},
{ .x=	1749,	.y=	63000	},
{ .x=	1711,	.y=	64000	},
{ .x=	1674,	.y=	65000	},
{ .x=	1637,	.y=	66000	},
{ .x=	1600,	.y=	67000	},
{ .x=	1564,	.y=	68000	},
{ .x=	1529,	.y=	69000	},
{ .x=	1493,	.y=	70000	},
{ .x=	1459,	.y=	71000	},
{ .x=	1425,	.y=	72000	},
{ .x=	1391,	.y=	73000	},
{ .x=	1358,	.y=	74000	},
{ .x=	1326,	.y=	75000	},
{ .x=	1294,	.y=	76000	},
{ .x=	1263,	.y=	77000	},
{ .x=	1232,	.y=	78000	},
{ .x=	1202,	.y=	79000	},
{ .x=	1173,	.y=	80000	},
{ .x=	1144,	.y=	81000	},
{ .x=	1115,	.y=	82000	},
{ .x=	1088,	.y=	83000	},
{ .x=	1060,	.y=	84000	},
{ .x=	1034,	.y=	85000	},
{ .x=	1008,	.y=	86000	},
{ .x=	982	,	.y=	87000	},
{ .x=	957	,	.y=	88000	},
{ .x=	933	,	.y=	89000	},
{ .x=	909	,	.y=	90000	},
{ .x=	886	,	.y=	91000	},
{ .x=	863	,	.y=	92000	},
{ .x=	841	,	.y=	93000	},
{ .x=	819	,	.y=	94000	},
{ .x=	798	,	.y=	95000	},
{ .x=	778	,	.y=	96000	},
{ .x=	758	,	.y=	97000	},
{ .x=	738	,	.y=	98000	},
{ .x=	719	,	.y=	99000	},
{ .x=	700	,	.y=	100000	},
{ .x=	682	,	.y=	101000	},
{ .x=	664	,	.y=	102000	},
{ .x=	647	,	.y=	103000	},
{ .x=	630	,	.y=	104000	},
{ .x=	614	,	.y=	105000	},
{ .x=	598	,	.y=	106000	},
{ .x=	583	,	.y=	107000	},
{ .x=	569	,	.y=	108000	},
{ .x=	553	,	.y=	109000	},
{ .x=	538	,	.y=	110000	},
{ .x=	524	,	.y=	111000	},
{ .x=	511	,	.y=	112000	},
{ .x=	498	,	.y=	113000	},
{ .x=	485	,	.y=	114000	},
{ .x=	472	,	.y=	115000	},
{ .x=	460	,	.y=	116000	},
{ .x=	448	,	.y=	117000	},
{ .x=	437	,	.y=	118000	},
{ .x=	426	,	.y=	119000	},
{ .x=	415	,	.y=	120000	},
{ .x=	404	,	.y=	121000	},
{ .x=	394	,	.y=	122000	},
{ .x=	384	,	.y=	123000	},
{ .x=	374	,	.y=	124000	},
{ .x=	364	,	.y=	125000	},
{ .x=	355	,	.y=	126000	},
{ .x=	346	,	.y=	127000	},
{ .x=	337	,	.y=	128000	},
{ .x=	329	,	.y=	129000	},
};					

//@}

//@{
/** private variables *********************************************************/
static INT32	_ValueBuf1[VALUE_BUF_SIZE]	= {0};
static INT32	_ValueBuf2[VALUE_BUF_SIZE]	= {0};
static int		_tank_temp_timeout   = TANK_TEMP_TIMEOUT;
static int		_head_temp_timeout   = HEAD_TEMP_TIMEOUT;
static int		_head_target_timeout = RANGE_TIMEOUT;
static BOOL		_heater_running      = FALSE;
//static int		_heater_delay		 = 0;
static INT32	_DutyPercent		 = 0;    // duty cycle in [%]
static INT32	_DeltaTempSumming_mK = 0;
static INT32	_DiffTemp_mK		 = 0;
static INT32	_DiffRampUp			 = 0;
//@}

//@{
/** private functions *********************************************************/
static void _heater_ctrl (UINT32 percent);
static int  _safety_net  (void);
static UINT32 _calc_heater_dutycycle(const INT32 t_jettingplate, const INT32 t_jettingplate_target);

//@}

/**
 * \brief Initialize ADC for measuring temperature in tank
 **/
void temp_init(void)
{
	// Disable ADC in any case first
	FM4_ADC0->ADCEN_f.ENBL = 0u;

	// Clear all relevant registers
	FM4_ADC0->ADCR = 0u;
	FM4_ADC0->ADSR = 0u;
	FM4_ADC0->SCCR = 0u;

	// Set Scan Conversion Input Selection Register
	FM4_ADC0->SCIS01_f.AN2=1;
	FM4_ADC0->SCIS01_f.AN5=1;
    
	// Set Sampling Time Selection Register
	FM4_ADC0->ADSS01 = 4u;
	FM4_ADC0->ADSS23 = 0u;

	// Set Sampling Times Config. 0
	FM4_ADC0->ADST0_f.STX0 = 4u;
	FM4_ADC0->ADST0_f.ST0 = 0x1E;

	// Set Sampling Times Config. 1
	FM4_ADC0->ADST1_f.STX1 = 4u;
	FM4_ADC0->ADST1_f.ST1 = 0x1E;

	// Frequency Division for ADC Instance
	FM4_ADC0->ADCT = 4u;

	// MSB, LSB alignment
	FM4_ADC0->ADSR_f.FDAS = 1u;

	// Conversion mode single
	FM4_ADC0->SCCR_f.RPT=0U;

	// Scan Conversion FIFO Depth
	FM4_ADC0->SFNS = 0u;

	// Set enable time
	FM4_ADC0->ADCEN_f.ENBLTIME = 10u;

	// Set Interrupt Configuration
    // Enable Scan conversion interrupt
    FM4_ADC0->ADCR_f.SCIE = 0x01;

	// ADC0 interrupt settings 
	NVIC_ClearPendingIRQ(ADC0_IRQn);
	NVIC_EnableIRQ(ADC0_IRQn);
    NVIC_SetPriority(ADC0_IRQn, 13u);

	// Enable ADC
	FM4_ADC0->ADCEN_f.ENBL = 1u;
		
    _heater_ctrl(OFF);

	// Polling for readiness
    // Time-out counter value for ADC ready bit polling
    for (UINT32 time = 1000000u; time && (!FM4_ADC0->ADCEN_f.READY); time--)
        ;
}

/**
 * \brief Turn `_heater_running` flag on or off
 *
 * Function `temp_tick_10ms` controls the heater's duty-cycle.
 **/
void temp_ctrl_on(int turn_on)
{
    if (turn_on != _heater_running)
	{ 
        _heater_running = turn_on;
		if (!_heater_running) {
			_heater_ctrl(OFF);
		}
		else {
			_DutyPercent = 0;
			_DeltaTempSumming_mK = 0;
			_DiffTemp_mK = 0;
			_DiffRampUp = _TEMP_RAMPUMP;
		}
//      if (_heater_running) _heater_delay = HEATER_DELAY;
//		else 				 _heater_ctrl(OFF);
    }
}

/**
 * \brief Function to set dutycycle of heater
 *
 * Called from `temp_tick_10ms` to turn heater on or off.
 *
 * \param [in] percent that the heater should be turned on
 **/
static void _heater_ctrl(UINT32 percent)
{
    /** Create Dutycycle for heater
     *
     * <pre>
     * e.g. UINT32 percent = 60;
     *    ,______,
     *  __|      |____|
     *     60% ON   40% OFF
     * </pre>
	 * function is called every 10 ms
     */

	static int _temp_timer = 0;
	static int _pg_cnt=0;

	if (RX_Status.error & (COND_ERR_meniscus))
    {
        RX_Status.heater_percent = 0;
    }
	else		
    {            
        // HW Revision >= 'h' has a second thermistor
		if (RX_Status.tempHeater == INVALID_VALUE || RX_Status.tempHeater > TEMP_MAX) 
			RX_Status.heater_percent = 0;
		else
			RX_Status.heater_percent = percent;
    }

    bFM4_GPIO_PDOR3_P3 = (_temp_timer < RX_Status.heater_percent);
	
	if (++_temp_timer >= 100) _temp_timer = 0;
	
	//--- check PG ----
	if(RX_Status.gpio_state.heater_pg == bFM4_GPIO_PDOR3_P3)
	{
		if (++_pg_cnt>10) RX_Status.error |= COND_ERR_power_heater;
	}
	else
	{
		_pg_cnt = 0;
	}
	
	if (RX_Config.mode != ctrl_test_watchdog)
		watchdog_toggle24V();
}

/**
 * \brief Handler function when new temperature is read from ADC
 *
 * This function is called from `ADC0_IRQHandler`,
 * which is triggered from `temp_tick_50ms`
 *
 * Ideal temperatures in the head, ranges from 30-35 °C for Waterbased ink,
 * 35-50 °C for UV ink.
 **/
UINT32 _handle_temp_val(UINT32 rowVal)
{    
    UINT32 i          = 0;
    UINT32 m          = 0;
    UINT32 delta_x    = 0;
    UINT32 delta_y    = 0;
	UINT32 val        = _temp_table[0].y;
	UINT32 table_size = SIZEOF(_temp_table);
    
	//--- convert value -----------------------
    // find the two points in the table to use
    for (i = 0; i < table_size; i++)
	{  
		if (rowVal > _temp_table[i].x)
		{
            /*
             * y = y0 + (x - x0) * ((y1 - y0) / (x1-x0))
             * where x0, x1 are nearest values of input x
             * y0, y1 are nearest values to output y
             */
			delta_x = _temp_table[i-1].x - _temp_table[i].x;
			delta_y = _temp_table[i].y   - _temp_table[i-1].y;
            m       = delta_y / delta_x;
			val     = _temp_table[i-1].y - (rowVal - _temp_table[i-1].x) * m;
			break;
		}
	}

    if (val < _temp_table[0].y) return INVALID_VALUE;
    if (val > _temp_table[table_size - 1].y) return INVALID_VALUE;
    return val;
}

/**
 * \brief Check if all temperatures are in range and all is well
 **/
static int _safety_net(void)
{
    static UINT32 _head_overheat_cnt 	= 0;
	static UINT32 _in_overheat_cnt	 	= 0;
	static UINT32 _heater_overheat_cnt	= 0;
    static int _last_inlet_deg_C 		= 0;
    static int _last_head_deg_C 		= 0;

    static signed char tank_gradients[GRAD_MAX_INDEX] = { 0 };

    INT32         tank_grad    = 0;
    static BYTE   grad_index   = 0;
    static UINT32 down_counter = 0;

    static BOOL _head_temp_in_range = FALSE; // is current temp in range
         
    
    // --- Head temperature ---
    if (RX_Config.tempHead == INVALID_VALUE || RX_Config.tempHead == 0)
    {
        RX_Status.error |= COND_ERR_temp_head_hw;
    }
    else
    {        
        if (RX_Config.tempHead < TEMP_ERROR) _head_overheat_cnt=0;
		else if (++_head_overheat_cnt > 100) RX_Status.error |= COND_ERR_temp_head_overheat;
    }
    
    // --- Heater temperature ---
	if ((RX_Status.tempHeater != INVALID_VALUE))
    {
		if (RX_Status.tempHeater < (TEMP_ERROR+2000)) _heater_overheat_cnt=0;
		else if (++_heater_overheat_cnt > 100) RX_Status.error |= COND_ERR_temp_heater_overheat;
    }
                        
    // --- Inktank temperature --- 
    // Ink in tank overheat or Thermistor is not connected
    // Unconnected thermistors can either report 129.0 or 10.0 °C
        
    /// \todo sometimes measured temperature from Nios is 1024 or 256 -> Investigate on Nios    
    if (RX_Status.tempIn!=INVALID_VALUE)
    {
        if (RX_Status.tempIn > TEMP_ERROR)
        {
			if (++_in_overheat_cnt>100) RX_Status.error |= COND_ERR_temp_ink_overheat;
        }
        else if (RX_Status.tempIn < ABSOLUTE_MIN_INK_TEMP)
        {
            // Measured temperature in tank too low
            RX_Status.error |= COND_ERR_temp_tank_too_low;
        }
		else _in_overheat_cnt=0;
    }
    else
    {
		_in_overheat_cnt=0;
        return 0;
    }
    
    // calculate gradient (raising/falling) of tank temperature
    if (RX_Status.tempIn == _last_inlet_deg_C)     tank_gradients[grad_index] = 0;
    else if (RX_Status.tempIn > _last_inlet_deg_C) tank_gradients[grad_index] = 1;
    else                                           tank_gradients[grad_index] = -1;

    if (grad_index++ >= GRAD_MAX_INDEX) grad_index = 0;

    tank_grad = 0;
    for (int i = 0; i < GRAD_MAX_INDEX; i++)
        tank_grad += tank_gradients[i];
    // end gradient calculation
    

    // If heater is running, temperatures must increase and
    // eventually reach target temperature
    if (RX_Status.heater_percent)
    {
        if (tank_grad > 0)
            down_counter = 0;
        else if (!_head_temp_in_range && tank_grad < 0)
            ++down_counter;

        if (down_counter > TANK_TEMP_FALLING_TIMEOUT)
        {
            RX_Status.error |= COND_ERR_temp_tank_falling;

            down_counter = 0;
        }

        // TANK temperature is expected to increase because we are heating up
        if (RX_Status.tempIn >= (_last_inlet_deg_C + TEMP_TANK_THRESHOLD))
        {
            _tank_temp_timeout = TANK_TEMP_TIMEOUT;
        }
        else
        {
            if ((!_head_temp_in_range) && (--_tank_temp_timeout <= 0))
            {
                _tank_temp_timeout = TANK_TEMP_TIMEOUT;

                RX_Status.error |= COND_ERR_temp_tank_not_changing;
			}
		}	

        // Head does not reach target range
        if (RX_Config.tempHead >= (UINT32)((1 - TEMP_HEAD_TARGET_RANGE) * RX_Config.temp) &&
            RX_Config.tempHead <= (UINT32)((1 + TEMP_HEAD_TARGET_RANGE) * RX_Config.temp))
        {
            _head_target_timeout = RANGE_TIMEOUT;
            _head_temp_timeout   = HEAD_TEMP_TIMEOUT;
            //_tank_temp_timeout   = TANK_TEMP_TIMEOUT;
            _head_temp_in_range = TRUE;
        }
        else
        {
            _head_temp_in_range = FALSE;

            if (_head_target_timeout-- <= 0)
            {
                _head_target_timeout = RANGE_TIMEOUT;

                //RX_Status.error |= COND_ERR_temp_head_target_unreached;
            }
        }

        // HEAD temperature is expected to increase because we are heating up
        if (RX_Config.tempHead > (_last_head_deg_C + TEMP_HEAD_THRESHOLD))
        {
            _head_temp_timeout = HEAD_TEMP_TIMEOUT;
        }	
        else
        {
            if ((!_head_temp_in_range) && (_head_temp_timeout-- <= 0))
            {
                _head_temp_timeout = HEAD_TEMP_TIMEOUT;

                RX_Status.error |= COND_ERR_temp_head_not_changing;
            }
        }
    } // if (RX_Status.heater_percent)

    // store last temperatures
    _last_inlet_deg_C = RX_Status.tempIn;
    _last_head_deg_C  = RX_Config.tempHead;

    return RX_Status.error;
}

/**
 * \brief Function called from timer IRQ to enable or disable the heater
 *
 * How often this function is called defines the granularity of the
 * temperature controller.
 **/
void temp_tick_10ms (void)
{	
	if (RX_Config.temp > TEMP_MAX)
		RX_Config.temp = TEMP_MAX;

	if (_heater_running 
   //     && !RX_Status.error 	// not for any error
        && (RX_Status.tempIn < RX_Config.tempMax))
    {        
		if (RX_Config.tempHead==INVALID_VALUE)
		{
			RX_Status.error |= COND_ERR_temp_head_hw;
			_heater_ctrl(_calc_heater_dutycycle(RX_Status.tempIn, RX_Config.temp));
		}
		else
		{
			_heater_ctrl(_calc_heater_dutycycle(RX_Config.tempHead, RX_Config.temp));
		}		
        
        #if defined (LOG_TEMP)        
        static UINT32 counter = 0;
        if (++counter % 50  == 0)
            DBG_PRINTF("%d tempHead %d setpoint %d tempInlet %d\n", counter, RX_Config.tempHead, RX_Config.temp, RX_Status.tempIn);
		#endif
	}
    else
    {
        _heater_ctrl(OFF);
    }
}

UINT32 _calc_heater_dutycycle(const INT32 t_jettingplate, const INT32 t_jettingplate_target)
{
    static const UINT32 c = 4184;   // temperature coefficient of water in [mWs/(ml*K)]
    static const UINT32 MAX_HEATER_POWER = 35000;
	static const UINT32 MAX_DUTY_CYCLE = 70;
    
    UINT32 deltatheta_mK = 0;       // temperature difference in [mK]
    UINT32 q_mW = 0;                // power in [mW]
    UINT32 m_ul = 0;                // mass in [ul] = [mg] with density of ink = 1 g/ml

    // calculate temperature difference in [mK]
	if (t_jettingplate_target > t_jettingplate)
        deltatheta_mK = t_jettingplate_target - t_jettingplate;
    else
        deltatheta_mK = 0;

	// eror correction
	if (_DiffRampUp)
	{
		_DiffRampUp--;
		_DeltaTempSumming_mK = 0;
	}
	else
	{
		if ((_DutyPercent > 0) && (_DutyPercent < MAX_DUTY_CYCLE))
			_DeltaTempSumming_mK += (t_jettingplate_target - t_jettingplate);
	}
	_DiffTemp_mK = _DeltaTempSumming_mK / 50000; // it takes 10000*0.01 sec = 100 secs to correct the value

    /* calculate mass/volume depending on pump speed and printed drops in [ul/s]
     * RX_Status.pump_measured in [ul/s]
     * RX_Config.volume_printed in [ul/s]
     * 
     * q [Ws] = [kg] * [Ws/(kg*K)] * [K]    SI-Units
     * q [1000mWs] = [1000000ul] * [1000mWs/(1000000ul*K)] * [K]     
     */
    m_ul = RX_Status.pump_measured + RX_Config.volume_printed;	
	q_mW = m_ul * c / 1000 * (deltatheta_mK + _DiffTemp_mK ) / 1000; // mK->K, ul->ml

    // calculate duty cycle in [%]
	_DutyPercent = 100 * q_mW / MAX_HEATER_POWER;
	if (_DutyPercent > MAX_DUTY_CYCLE)
		_DutyPercent = MAX_DUTY_CYCLE;
	if (_DutyPercent <0)
		_DutyPercent = 0;
    
    //DBG_PRINTF("m = %d ul/s %d ml/min printed = %d -> q = %d [temp %d - %d = %d] pump = %d duty = %d\n", m_ul, RX_Status.pump_measured * 60 / 1000, RX_Config.volume_printed, q_mW, t_jettingplate_target, t_inlet, deltatheta_mK, RX_Status.pump_measured, duty_percent);
    
	return _DutyPercent;
}

/**
 * \brief Timed IRQ to trigger a temperature measurement
 **/
void temp_tick_50ms (void)
{	
    // Trigger AD Conversion
    FM4_ADC0->SCCR_f.SSTR = 1u;
        
    _safety_net();    
}

/**
 * \brief ADC IRQ handler (occurs if ad conversion is over and new data
 *ready)
 **/
void ADC0_IRQHandler(void)
{
    #define ADC_CHAN_1 2
    #define ADC_CHAN_2 5
    
	static int	_ValueIdx1		= 0;
	static int	_ValueIdx2		= 0;
	static int	_ValueIdxValid1	= FALSE;
	static int	_ValueIdx2Valid = FALSE;

    INT32 channel = 0;
	INT32 val;
   
    FM4_ADC0->SCCR_f.SOVR = 0u; // Error case (FIFO overflow)
    FM4_ADC0->PCCR_f.POVR = 0u; // Error case (Priority FIFO overflow)
    FM4_ADC0->ADCR_f.PCIF = 0u; // Priority Conversion

    // Scan conversion interrupt request?
    if (FM4_ADC0->ADCR_f.SCIF)
    {
        // check validity of register
        if (FM4_ADC0->SCFD_f.INVL == 0)
        {
            channel = FM4_ADC0->SCFDL & ADC_CHAN;
            if (channel == ADC_CHAN_1)
			{
				val = _handle_temp_val(FM4_ADC0->SCFDH & ADC_DATA);
				if (val==INVALID_VALUE) RX_Status.error |= COND_ERR_temp_heater_hw;
                else
				{
					_ValueBuf1[_ValueIdx1++] = val;
					// calculate average temperature when buffer is full
					if (_ValueIdx1 >= VALUE_BUF_SIZE)
					{
						_ValueIdx1 = 0;
						_ValueIdxValid1 = TRUE;
					}
					if (_ValueIdxValid1)RX_Status.tempHeater = average(_ValueBuf1, VALUE_BUF_SIZE, 0);
				}
			}
			else if ((ADC_CHAN_2 == 5))
			{
				val = _handle_temp_val(FM4_ADC0->SCFDH & ADC_DATA);
				if (val==INVALID_VALUE) RX_Status.error |= COND_ERR_temp_inlet_hw;
				else
				{
					_ValueBuf2[_ValueIdx2++] = val;
					if (_ValueIdx2 >= VALUE_BUF_SIZE)
					{
						_ValueIdx2 = 0;
						_ValueIdx2Valid = TRUE;
					}
					// Thermistor position is switched on revision #h
					if (_ValueIdx2Valid) RX_Status.tempIn = average(_ValueBuf2, VALUE_BUF_SIZE, 0);
				}
			}
        }
        
        FM4_ADC0->ADCR_f.SCIF = 0u;
    }
	
    FM4_ADC0->ADCR_f.CMPIF    = 0u; // Compare result interrupt request?
    FM4_ADC0->WCMRCIF_f.RCINT = 0u; // Range result interrupt request?
	
    NVIC_ClearPendingIRQ(ADC0_IRQn); 
}
