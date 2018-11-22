// ****************************************************************************
//
//	DIGITAL PRINTING - daisy_chain.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2013 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#ifdef linux
	#include <termios.h>
	#include <sys/stat.h> 
	#include <fcntl.h>
#endif

#include "rx_def.h"
#include "rx_error.h"
#include "rx_term.h"
#include "rx_threads.h"
#include "daisy_chain.h"
#include "fpga_fluid.h"
#include "comm.h"


//--- defines -----------------------------------------------------------------
#define SERIAL_PORT	"/dev/ttyAL0"

#define FIFO_SIZE			128

#define MAX_DEVICE_CNT		64

#define LOADCELL_MAX_PCB	8

//--- golbals -----------------------------------------------------------------
static FILE *_daisy_chain_out;
static FILE *_daisy_chain_in;

static SCommandMsg		_TxFifo[FIFO_SIZE];
static int				_FifoInIdx;
static int				_FifoOutIdx;

static uint8_t			_device_id [MAX_DEVICE_CNT];
static uint8_t			_device_cnt[DEVICE_CNT+1];
static uint8_t			_device_count = 0;

static SAdcConfigRegs	_AdcConfigRegs[LOADCELL_MAX_PCB];
static SDevice_Adc		_Adc[LOADCELL_MAX_PCB];

//---pprototypes --------------------------------------------------------------
static void *_daisy_chain_thread(void *ppar);
static void _display_status(void);
static void _display_status_adc(void);
static void	*_daisy_chain_detect_devices(void *ppar);
static int _daisy_chain_send_command(SCommandMsg *command);
static int _daisy_chain_read(void);
static int _daisy_chain_request_dispatcher(SAnswerMsg* answer);
static int _daisy_chain_adc_request_handler(SAnswerMsg* answer);
static void _comm_set_baud(int comm, int baud);
static int _devIdx(int devNo, int devId);

//--- daisy_chain_init --------------------------------------------------------
int daisy_chain_init(void)
{
	if (!fpga_is_init()) return REPLY_ERROR;
	
	// Serial Port setup
	system("stty -F " SERIAL_PORT " raw cs8 cread -ixon -cstopb -parenb -echo 115200");
	
	// Open Serial port output
	_daisy_chain_out = fopen(SERIAL_PORT, "w");
	if(_daisy_chain_out == NULL)
		return Error(ERR_CONT, 0, "Could not open serial port >>%s<< with write access.", SERIAL_PORT);
	
	// Open serial port input
	_daisy_chain_in  = fopen(SERIAL_PORT, "r");
	if(_daisy_chain_in == NULL)
		return Error(ERR_CONT, 0, "Could not open serial port >>%s<< with read access.", SERIAL_PORT);
	
	// Setup non blocking mode
	int fd = fileno(_daisy_chain_in);
	int flags = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	
	// Config Structur init
	memset(_device_id, 0, sizeof(_device_id));
	memset(_device_cnt, 0, sizeof(_device_cnt));
	memset(&_AdcConfigRegs, 0, sizeof(_AdcConfigRegs));
	
	comm_init(0);
	
	// TX-Fifo init
	memset(_TxFifo, 0, sizeof(_TxFifo));
	_FifoInIdx  = 0;
	_FifoOutIdx = 0;
	
	// Scan for devices
	rx_thread_start(_daisy_chain_detect_devices, NULL, 0, "DaisyChain Device detect");
	
	return REPLY_OK;
}

//--- daisy_chain_end ---------------------------------------------------------
int daisy_chain_end(void)
{
	return REPLY_OK;		
}

//--- daisy_chain_detect_devices ----------------------------------------------
static void * _daisy_chain_detect_devices(void *ppar)
{
	int cnt = 0;
	int len;
	BYTE buf[1024];
	
	// Setup command
	SCommandMsg detection_command;
	detection_command.dev_id  = UNDEF_DEVICE_ID;
	detection_command.command = DEVICE_DETECT_CMD;
	detection_command.length  = 0;
	
	for (cnt  = 1; cnt < MAX_DEVICE_CNT; cnt++)
	{
		// Send request
		detection_command.dev_nr = cnt;
		if (_daisy_chain_send_command(&detection_command) == REPLY_ERROR)
			return NULL;
		
		while (_FifoOutIdx != _FifoInIdx)
		{

			len = sizeof(SCommandMsg) - sizeof(_TxFifo[_FifoOutIdx].data) + _TxFifo[_FifoOutIdx].length;
			comm_encode(&_TxFifo[_FifoOutIdx], len, buf, sizeof(buf), &len);
			fwrite(&buf, 1, len, _daisy_chain_out);
			fflush(_daisy_chain_out);
				
			_FifoOutIdx = (_FifoOutIdx + 1) % FIFO_SIZE;
			
			// Wait for answer (or timeout if no more boards are connected)
			if (_daisy_chain_read() == REPLY_ERROR)
			{
				// Start communication thread
				rx_thread_start(_daisy_chain_thread, NULL, 0, "DaisyChain Thread");
				return NULL;
			}
		}
	}
	
	// Start communication thread
	rx_thread_start(_daisy_chain_thread, NULL, 0, "DaisyChain Thread");
	return NULL;
}

//--- _daisy_chain_send_command ------------------------------------------------
static int _daisy_chain_send_command(SCommandMsg *command)
{
	int idx1 = (_FifoInIdx + 1) % FIFO_SIZE;
	if (idx1 == _FifoOutIdx) 
	{
		return Error(ERR_CONT, 0, "Daisychain FIFO overfilled.");
	}
	
	memcpy(&_TxFifo[_FifoInIdx], command, sizeof(SCommandMsg));
	
	_FifoInIdx = idx1;
	
	return REPLY_OK;		
}

//--- daisy_chain_read --------------------------------------------------------
static int _daisy_chain_read(void)
{
	int				len = 0;
	int				data = 0;
	unsigned int	timeout = 0;
	SAnswerMsg	answer;
	
	memset(&answer, 0, sizeof(answer));
	
	while (TRUE)
	{
		data = fgetc(_daisy_chain_in);
		
		if (data >= 0)
		{
			if (comm_received(0, (uint8_t)data))
			{
				if (comm_get_data(0, (BYTE*)&answer, sizeof(answer), &len))
				{
					if (len == sizeof(answer) - sizeof(answer.data) + answer.length)
					{
						if (answer.error)
						{
							return Error(ERR_CONT, 0, "DAISYCHAIN RECV: NR=%d ID=%d Command=%d, len=%d, err=%d", answer.dev_ctr, answer.dev_id, answer.command, answer.length, answer.error);
						}
					
						return _daisy_chain_request_dispatcher(&answer);
					}
				}
			}
		}
		
		timeout++;
		if (timeout > UINT16_MAX*4)
		{
			return REPLY_ERROR;
		}
	}
	
	return REPLY_OK;		
}

//--- _daisy_chain_thread -----------------------------------------------------
int daisy_chain_main(int ticks, int menu, int displayStatus)
{
	if (displayStatus && menu)
	{
		_display_status_adc();			
	}
	return REPLY_OK;
}

//--- _display_status_adc ---------------------------------------------------------
static void _display_status_adc()
{
#define FOR_ALL for (i = 0; i < _device_cnt[ADC_DEVICE_ID]; i++)
	
	int i;
	int j;
	
	// Print ADC data
	term_printf("\n"); 
	
	term_printf("--- ADC  ------"); FOR_ALL {if (_device_id[i] == ADC_DEVICE_ID) {term_printf("  ---%d---", i); }}term_printf("\n"); 	
	term_printf("Temperature:    "); FOR_ALL {term_printf("% 8d ", _Adc[i].temp);}term_printf("\n");
	for (j = 0; j < SIZEOF(_Adc[i].weight); j++) 
	{	
		term_printf("Value[%d]:       ", j); FOR_ALL {term_printf("%8s ", value_str(_Adc[i].weight[j])); } term_printf("\n"); 
	}
}

//--- _daisy_chain_thread -----------------------------------------------------
static void *_daisy_chain_thread(void *ppar)
{	
	int len;
	int device = 0;
	BYTE buf[1024];
	SCommandMsg test_cmd;
	
	while (TRUE)
	{
		for (device = 0; device < _device_count; device++)
		{
			switch (_device_id[device])
			{
			case ADC_DEVICE_ID:
				test_cmd.dev_nr = device + 1;
				test_cmd.dev_id = ADC_DEVICE_ID;
				test_cmd.command = GET_ADC_DATA;
				test_cmd.length = 0;
				break;
				
			default:
				break;
			}
			
			_daisy_chain_send_command(&test_cmd);
		}
		
		while (_FifoOutIdx != _FifoInIdx)
		{
			len = sizeof(SCommandMsg) - sizeof(_TxFifo[_FifoOutIdx].data) + _TxFifo[_FifoOutIdx].length;
			comm_encode(&_TxFifo[_FifoOutIdx], len, buf, sizeof(buf), &len);
			fwrite(&buf, 1, len, _daisy_chain_out);
			fflush(_daisy_chain_out);
				
			_FifoOutIdx = (_FifoOutIdx + 1) % FIFO_SIZE;
			_daisy_chain_read();
		}
	}
}

//--- _daisy_chain_request_dispatcher ----------------------------------------------
static int _daisy_chain_request_dispatcher(SAnswerMsg* answer)
{
	if (answer->command == DEVICE_DETECT_CMD)
	{
		_device_id[_device_count] = answer->dev_id;
		_device_cnt[answer->dev_id]++;
		_device_count++;
		
		return REPLY_OK;
	}
	
	switch (answer->dev_id)
	{
	case UNDEF_DEVICE_ID:
		return Error(ERR_CONT, 0, "Daisychain: UNDEF_DEVICE_ID Error. Got answer from unknown device Nr. %d", answer->dev_ctr);
		
	case ADC_DEVICE_ID:
		return _daisy_chain_adc_request_handler(answer);
		
	default:
		return Error(ERR_CONT, 0, "Daisychain: UNDEF_DEVICE_ID Error. Got answer from unknown device Nr. %d", answer->dev_ctr);
	}
	
	return REPLY_OK;		
}

//--- _devIdx --------------------------------------
static int _devIdx(int devNo, int devId)
{
	int i, no=-1;
	for (i=0; i<devNo; i++)
	{
		if (_device_id[i]==devId) no++;
	}
	return no;
}

//--- _daisy_chain_handle_request ----------------------------------------------
static int _daisy_chain_adc_request_handler(SAnswerMsg* answer)
{
	int no, i;
	
	no = _devIdx(answer->dev_ctr, answer->dev_id);
	
	if (no<0) return Error(ERR_CONT, 0, "Invalid command in response: Recived command %d from device %d.", answer->command, answer->dev_ctr);
	
	switch (answer->command)
	{
	case SET_ADC_CONFIG:
		return REPLY_OK;
		
	case GET_ADC_CONFIG:
		if (answer->length != sizeof(SAdcConfigRegs))
			return Error(ERR_CONT, 0, "SAdcConfigRegs size missmatch: %d bytes recived instead of %d bytes.", answer->length, sizeof(SAdcConfigRegs));
		
		memcpy(&_AdcConfigRegs[no], answer->data, sizeof(SAdcConfigRegs));
		return REPLY_OK;
		
	case GET_ADC_DATA:
		if (answer->length != sizeof(SDevice_Adc))
			return Error(ERR_CONT, 0, "Loadcell data size missmatch: %d bytes recived instead of %d bytes.", answer->length, sizeof(SDevice_Adc));	
		
		SDevice_Adc *padc = &_Adc[no];
		memcpy(padc, answer->data, sizeof(SDevice_Adc));
		
		// Umrechnung temperatur: 
		// U [V] = Value * 0.0000001490116119384765625
		// 0.129V = 25°
		// 0.000403V = 1°C Delta
		// Temp [°C] = 25 + ((Value * 0.0000001490116119384765625) - 0.129) / 0.000403
		
		padc->temp = (INT32)(25.0 + (((0.000000149*padc->temp) - 0.129) / 0.000403));

		// Umrechnung gewicht:
		// U [V] = Value * 0.000000004656612873077392578125
		// ±39.0625 mV = +- 40kg

		for (i=0; i<SIZEOF(padc->weight); i++)
		{
			/*
			if(padc->weight[i]<0 || padc->weight[i]>1000000)
				padc->weight[i] = INVALID_VALUE;
			else
			*/
			{
				padc->weight[i] = padc->weight[i];
			}			
		}
		return REPLY_OK;
		
	default:
		return Error(ERR_CONT, 0, "Invalid command in response: Recived command %d from device %d.", answer->command, answer->dev_ctr);
	}
	
	return REPLY_OK;		
}