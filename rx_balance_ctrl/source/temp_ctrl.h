/******************************************************************************/
/** temp_ctrl.h
 ** 
 ** uart functions to communicate between fpga and conditioner board
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/
 
#ifndef __temp_ctrl_h__
#define __temp_ctrl_h__
	
#define ADC_INVL	0x1000
#define ADC_CH		0x1F
#define ADC_DATA	0xFFF0

#define ADC_BUFFER_SIZE 10

typedef struct
{
	int size;
	int wrIdx;
	int rdIdx;
	int data[ADC_BUFFER_SIZE];
}ADC_BUF;

typedef struct
{
	ADC_BUF buf[3];
}WEIGHT_ADC_BUF;

extern volatile uint32_t weight[3];
extern uint32_t adc_toogle;
extern uint32_t adc_readout;
extern volatile uint32_t adc_channel_sel;

extern WEIGHT_ADC_BUF adc_buffer;

// Prototype

void temp_init(int channel);
void temp_tick_10ms (void);
void temp_tick_50ms (void);
void ADC_BUFFER_RESET(ADC_BUF * adc_buf);
int ADC_BUFFER_EMPTY(ADC_BUF * adc_buf);
int ADC_BUFFER_FULL(ADC_BUF * adc_buf);
int ADC_BUFFER_COUNT(ADC_BUF * adc_buf);
void ADC_BUFFER_WR(ADC_BUF * adc_buf, int dataIn);
int ADC_BUFFER_RD(ADC_BUF * adc_buf);
int ADC_BUFFER_RD_MEAN(ADC_BUF * adc_buf);

#endif /* __temp_ctrl_h__ */
