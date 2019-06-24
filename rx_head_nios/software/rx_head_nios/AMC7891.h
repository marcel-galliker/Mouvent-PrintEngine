
#ifndef __AMC7891_H__
#define __AMC7891_H__

int init_AMC7891(void);

#define AMC7891_TEMP_DATA		(2 * 0x00)
#define AMC7891_TEMP_CONFIG		(2 * 0x0A)
#define AMC7891_TEMP_RATE		(2 * 0x0B)
#define AMC7891_ADC0_DATA		(2 * 0x23)
#define AMC7891_ADC1_DATA		(2 * 0x24)
#define AMC7891_ADC2_DATA		(2 * 0x25)
#define AMC7891_ADC3_DATA		(2 * 0x26)
#define AMC7891_ADC4_DATA		(2 * 0x27)
#define AMC7891_ADC5_DATA		(2 * 0x28)
#define AMC7891_ADC6_DATA		(2 * 0x29)
#define AMC7891_ADC7_DATA		(2 * 0x2A)
#define AMC7891_DAC0_DATA		(2 * 0x2B)
#define AMC7891_DAC1_DATA		(2 * 0x2C)
#define AMC7891_DAC2_DATA		(2 * 0x2D)
#define AMC7891_DAC3_DATA		(2 * 0x2E)
#define AMC7891_DAC0_CLEAR		(2 * 0x2F)
#define AMC7891_DAC1_CLEAR		(2 * 0x30)
#define AMC7891_DAC2_CLEAR		(2 * 0x31)
#define AMC7891_DAC3_CLEAR		(2 * 0x32)
#define AMC7891_GPIO_CONFIG		(2 * 0x33)
#define AMC7891_GPIO_OUT		(2 * 0x34)
#define AMC7891_GPIO_IN			(2 * 0x35)
#define AMC7891_AMC_CONFIG		(2 * 0x36)
#define AMC7891_ADC_ENABLE		(2 * 0x37)
#define AMC7891_ADC_GAIN		(2 * 0x38)
#define AMC7891_DAC_CLEAR		(2 * 0x39)
#define AMC7891_DAC_SYNC		(2 * 0x3A)
#define AMC7891_AMC_POWER		(2 * 0x3B)
#define AMC7891_AMC_RESET		(2 * 0x3C)
#define AMC7891_AMC_ID			(2 * 0x40)

#define AMC_ID_VALUE			0x0044


#define _0V				0
#define _3V3			676

#endif /* __AMC7891_H__ */
