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
#include "IOMux.h" 
#include "temp_ctrl.h"

//--- defines -----------------------------
#define TABLE_SIZE 81

#define VALUE_BUF_SIZE		100

#define SENSOR_NR					3
			
//--- prototypes ---------------------------------------------------
void _handle_temp_val(void);

//--- temp_init ----------------------------------------------------------
void temp_init(int channel)
{
	// Disable ADC in any case first
	FM4_ADC0->ADCEN_f.ENBL = 0u;

	// Clear all relevant registers
	FM4_ADC0->ADCR = 0u;
	FM4_ADC0->ADSR = 0u;
	FM4_ADC0->SCCR = 0u;

	// Set Scan Conversion Input Selection Register
	switch (channel)
	{
		case 0 :	FM4_ADC0->SCIS01_f.AN1=1; break;
		case 1 : 	FM4_ADC0->SCIS01_f.AN2=1; break;
		case 2 :	FM4_ADC0->SCIS01_f.AN5=1; break;
		default: break;
	}
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
	
//	// Conversion mode continous multiple channel
	FM4_ADC0->SCCR_f.RPT=0;

	// Set enable time
	FM4_ADC0->ADCEN_f.ENBLTIME = 10u;

	// Set Interrupt Configuration
	FM4_ADC0->SFNS  = 0;    // Set FIFO Stage Count Interrupt (if there is 1 values in fifo)
	FM4_ADC0->ADCR_f.SCIE=0x01; // Enable Scan conversion interrupt

	// ADC0 interrupt settings 
	NVIC_ClearPendingIRQ(ADC0_IRQn);
	NVIC_EnableIRQ(ADC0_IRQn);
	NVIC_SetPriority(ADC0_IRQn, 10u);

	// Enable ADC
	FM4_ADC0->ADCEN_f.ENBL = 1u;
	
	{ // Polling for readiness
		uint32_t time = 1000000u;        ///< Time-out counter value for ADC ready bit polling
		for (time=1000000u; time&&(FM4_ADC0->ADCEN_f.READY!=1); time--)
		{ 
		}
	}
}

/**
 ******************************************************************************
 ** \brief  adc irq handler (occurs if ad conversion is over and new data ready)
 ******************************************************************************/
void ADC0_IRQHandler(void)
{
	if (FM4_ADC0->SCCR_f.SOVR)   // Error case (FIFO overflow)
	{
		FM4_ADC0->SCCR_f.SOVR = 0u;
	}

	if (FM4_ADC0->PCCR_f.POVR)  // Error case (Priority FIFO overflow)
	{
		FM4_ADC0->PCCR_f.POVR = 0u;
	}

	if (FM4_ADC0->ADCR_f.PCIF)  // Priority Conversion
	{
		FM4_ADC0->ADCR_f.PCIF = 0u;
	}

	if (FM4_ADC0->ADCR_f.SCIF)   // Scan conversion interrupt request?
	{
		_handle_temp_val();
		FM4_ADC0->ADCR_f.SCIF = 0u;
	}

	if (FM4_ADC0->ADCR_f.CMPIF)  // Compare result interrupt request?
	{
		FM4_ADC0->ADCR_f.CMPIF = 0u;
	}

	if (FM4_ADC0->WCMRCIF_f.RCINT)  // Range result interrupt request?
	{
		FM4_ADC0->WCMRCIF_f.RCINT = 0u;
	}	

	NVIC_ClearPendingIRQ(ADC0_IRQn);
}

//--- _handle_temp_val -----------------
void _handle_temp_val(void)
{
	int channel=0;
	int data=0;
	
	if(FM4_ADC0->SCFD_f.INVL==0)	//check validity of register
	{
		channel=FM4_ADC0->SCFDL&ADC_CH;
		data=FM4_ADC0->SCFDH&ADC_DATA;
		if(channel==2) channel=0;
		else if(channel==3) channel=1;
		else if(channel==5) channel=2;
		weight[channel]=data;
		ADC_BUFFER_WR(&adc_buffer.buf[channel], data);
		adc_readout+=1;
	}
}

//--- temp_tick_50ms ---------------------------------
void temp_tick_50ms (void)
{	
		adc_channel_sel+=1;
		if(adc_channel_sel>2) adc_channel_sel=0;
		temp_init(adc_channel_sel);
		FM4_ADC0->SCCR_f.SSTR = 1u;     				// Trigger AD Conversion
}

/**
 ******************************************************************************
 ** \brief  reset the adc_buf (sets pointer, size and data to 0x00)
 **
 ** \input	ADC_BUF * pointer to adc_buf
 ******************************************************************************/
void ADC_BUFFER_RESET(ADC_BUF * adc_buf)
{
	unsigned int count;

	for(count=0; count <ADC_BUFFER_SIZE; count++)
	{
		adc_buf->data[count]=0x00;
	}
	adc_buf->size=0;
	adc_buf->rdIdx = adc_buf->wrIdx = 0x00;
}

/**
 ******************************************************************************
 ** \brief  check if adc_buf is empty
 **
 ** \input	ADC_BUF * pointer to adc_buf
 **
 ** \return if adc_buf is empty 1 else 0
 ******************************************************************************/
int ADC_BUFFER_EMPTY(ADC_BUF * adc_buf)
{
	if(adc_buf->rdIdx==adc_buf->wrIdx)
	{
		return (0x1);
	}
	else return (0x0);
}

/**
 ******************************************************************************
 ** \brief  check if adc_buf is full
 **
 ** \input	ADC_BUF * pointer to adc_buf
 **
 ** \return if adc_buf is full 1 else 0
 ******************************************************************************/
int ADC_BUFFER_FULL(ADC_BUF * adc_buf)
{
	if(adc_buf->size>=ADC_BUFFER_SIZE)
	{
		return (0x1);
	}
	else return (0x0);
}
/**
 ******************************************************************************
 ** \brief  give back size of adc_buf
 **
 ** \input	ADC_BUF * pointer to adc_buf
 **
 ** \return size of adc_buf
 ******************************************************************************/
int ADC_BUFFER_COUNT(ADC_BUF * adc_buf)
{
	return adc_buf->size;
}

/**
 ******************************************************************************
 ** \brief  write data to adc_buf if he is not full, so check before!
 **
 ** \input	ADC_BUF * pointer to adc_buf, char data to write in adc_buf, overwrites old data
 ******************************************************************************/
void ADC_BUFFER_WR(ADC_BUF * adc_buf, int dataIn)
{
	if(adc_buf->wrIdx>=ADC_BUFFER_SIZE)
	{
		adc_buf->wrIdx=0x0;
	}

	adc_buf->data[adc_buf->wrIdx++] = (dataIn);

	if(!ADC_BUFFER_FULL(adc_buf))
	{
		adc_buf->size+=1;
	}
}

/**
 ******************************************************************************
 ** \brief  read from adc_buf if he is not empty
 **
 ** \input	ADC_BUF * pointer to adc_buf
 **
 ** \return char data in adc_buf or 0xff if the adc_buf is empty
 ******************************************************************************/
int ADC_BUFFER_RD(ADC_BUF * adc_buf)
{
	int dataOut=0x0;
	if(!ADC_BUFFER_EMPTY(adc_buf))
	{
		if(adc_buf->rdIdx>=ADC_BUFFER_SIZE)
		{
			adc_buf->rdIdx=0x0;
		}
		dataOut = adc_buf->data[adc_buf->rdIdx];
		adc_buf->rdIdx++;
		adc_buf->size-=1;
		return (dataOut);
	}
	return (-1);
}

/**
 ******************************************************************************
 ** \brief  read from adc_buf the mean value if he is not empty
 **
 ** \input	ADC_BUF * pointer to adc_buf
 **
 ** \return char data in adc_buf or 0xff if the adc_buf is empty
 ******************************************************************************/
int ADC_BUFFER_RD_MEAN(ADC_BUF * adc_buf)
{
	int dataOut=0x0;
	int count=0;
	if(ADC_BUFFER_FULL(adc_buf))
	{
		for(count=0;count<ADC_BUFFER_SIZE;count++)
		{
			dataOut+=adc_buf->data[count];
		}
		dataOut=dataOut/ADC_BUFFER_SIZE;
		return (dataOut);
	}
	return (0);
}
