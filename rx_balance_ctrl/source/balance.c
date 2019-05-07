/******************************************************************************/
/** \file balance.c
 ** 
 ** read the sensor of the balance 
 **
 **	Copyright 2017 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "rx_ink_common.h"
#include "IOMux.h" 
#include "balance.h"
#include "balance_def.h"

//--- defines -----------------------------

#define SENSOR_NR					3
#define ADC_CH		0x1F
#define ADC_DATA	0xFFF0

//int _adc_fifo_overflow=0;
//int _adc_priority_fifo_overflow=0;
//int _adc_priority_conversion=0;
//int _adc_scan_conversion_irq=0;
//int _adc_compare_irq=0;
//int _adc_range_irq=0;			
//int _adc_wrong_channel=0;
//int _adc_not_valid_register=0;

extern volatile UINT32 weight[3];

uint32_t adc_readout=0;
			
//--- prototypes ---------------------------------------------------
void adc_handler(void);

//--- adc_init ----------------------------------------------------------
void adc_init(int channel)
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
	NVIC_SetPriority(ADC0_IRQn, 6u);

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
//		_adc_fifo_overflow+=1;
	}

	if (FM4_ADC0->PCCR_f.POVR)  // Error case (Priority FIFO overflow)
	{
		FM4_ADC0->PCCR_f.POVR = 0u;
//		_adc_priority_fifo_overflow+=1;
	}

	if (FM4_ADC0->ADCR_f.PCIF)  // Priority Conversion
	{
		FM4_ADC0->ADCR_f.PCIF = 0u;
//		_adc_priority_conversion+=1;
	}

	if (FM4_ADC0->ADCR_f.SCIF)   // Scan conversion interrupt request?
	{
		adc_handler();
		FM4_ADC0->ADCR_f.SCIF = 0u;
//		_adc_scan_conversion_irq+=1;
	}

	if (FM4_ADC0->ADCR_f.CMPIF)  // Compare result interrupt request?
	{
		FM4_ADC0->ADCR_f.CMPIF = 0u;
//		_adc_compare_irq+=1;
	}

	if (FM4_ADC0->WCMRCIF_f.RCINT)  // Range result interrupt request?
	{
		FM4_ADC0->WCMRCIF_f.RCINT = 0u;
//		_adc_range_irq+=1;
	}	

	NVIC_ClearPendingIRQ(ADC0_IRQn);
}

//--- adc_handler -----------------
void adc_handler(void)
{
	int channel=0;
	int data=0;
	
	if(FM4_ADC0->SCFD_f.INVL==0)	//check validity of register
	{
		channel=FM4_ADC0->SCFDL&ADC_CH;
		data=FM4_ADC0->SCFDH&ADC_DATA;
        switch (channel)
        {
            case 1:
            	weight[0]=data;
 //               adc_readout+=1;
                break;
            case 2:
            	weight[1]=data;
//                adc_readout+=1;
                break;
            case 5:
            	weight[2]=data;
//                adc_readout+=1;
                break;
            default:
//                _adc_wrong_channel+=1;
                break;
        }
	}
    else
    {
    //    _adc_not_valid_register+=1;
    }
}
