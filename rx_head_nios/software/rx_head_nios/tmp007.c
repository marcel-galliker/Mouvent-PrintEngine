#include <stdio.h>
#include "alt_types.h"
#include "sys/alt_irq.h"
#include "altera_avalon_timer.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_pio_regs.h"
#include <unistd.h>
#include "sys/alt_stdio.h"
#include "system.h"
#include "io.h"
#include "sys/alt_alarm.h"
#include "rx_ink_common.h"
#include "nios_def_head.h"
#include "tmp007.h"
#include "i2c_master.h"
#include "i2c.h"

//--- defines ----------------------------------------------------

#define TMP007_U15_ADR				0x40
//#define TMP007_U16_ADR				0x41
//#define TMP007_U17_ADR				0x42
//#define TMP007_U18_ADR				0x43
//#define TMP007_U19_ADR				0x44
//#define TMP007_U20_ADR				0x45
//#define TMP007_U21_ADR				0x46
//#define TMP007_U22_ADR				0x47

#define TMP007_VSENSOR_VOLTAGE		0x00
#define TMP007_TDIE_LOCAL_TEMP		0x01
#define TMP007_TOBJ_TEMP			0x03
#define TMP007_DEVICE_ID_ADDR		0x1F
#define TMP007_DEVICE_ID_VALUE		0x0078

static int	_SensorNo=0;

//--- tmp_init --------------------------------------------------
void tmp_init(void)
{
	int i;
	int i2c_adr;
	UINT32 id;
	static const int RETRY = 3;
	int retry = RETRY;		// number of retries for HW initialization

	for (i=0; i<IR_SENSOR_CONT; i++)
	{
		i2c_adr = TMP007_U15_ADR +i;

		pRX_Status->ir_sensor[i].tempObj = INVALID_VALUE;
		pRX_Status->ir_sensor[i].tempDie = INVALID_VALUE;
		pRX_Status->ir_sensor[i].voltage = INVALID_VALUE;

		do {
			id = 0;
			if (I2C_start(I2C_MASTER_1_BASE,(i2c_adr),WRITE)==I2C_ACK)
			{
				I2C_write(I2C_MASTER_1_BASE,TMP007_DEVICE_ID_ADDR, LAST_BYTE);  	// set address
				I2C_start(I2C_MASTER_1_BASE,(i2c_adr),READ); 			// set chip address in read mode
				id  = (I2C_read(I2C_MASTER_1_BASE,0) <<8)+ (I2C_read(I2C_MASTER_1_BASE,LAST_BYTE));
			}
		} while ((id != TMP007_DEVICE_ID_VALUE) && --retry);

		if (id != TMP007_DEVICE_ID_VALUE)
			pRX_Status->error.tmp007=TRUE;
	}

	_SensorNo = 0;
}

//--- tmp_tick_10ms ----------------------------------------------
void tmp_tick_10ms(void)
{
	// IR Temp. Sensors (conversion time minimal 250m, default 1.01s (average over 4 measurements on chip))
	// dez.   hex       deg.
	// 4800 = 0x12C0 => 150°C
	// 4000 = 0x0FA0 => 125°C
	// 3200 = 0x0C80 => 100°C
	// 2560 = 0x0A00 =>  80°C
	// 2400 = 0x0960 =>  75°C
	// 1600 = 0x0640 =>  50°C
	//  800 = 0x0320 =>  25°C
	//    1	= 0x0001 =>   0.03125°C
	//    0 = 0x0000 =>   0°C
	//65535 = 0xFFFF =>  -0.03125°C
	//63775 = 0xF91F => -55°C

	int i2c_adr;
	alt_16 temp;
	alt_u8 data[2];

	if (_SensorNo >= IR_SENSOR_CONT)
		_SensorNo=0;

	i2c_adr = TMP007_U15_ADR + _SensorNo;

	// sensor voltage result
	if (pRX_Status->error.tmp007)
	{
		pRX_Status->ir_sensor[_SensorNo].voltage = INVALID_VALUE;
		pRX_Status->ir_sensor[_SensorNo].tempObj = INVALID_VALUE;
		pRX_Status->ir_sensor[_SensorNo].tempDie = INVALID_VALUE;
	}
	else
	{
		if(I2C_start(I2C_MASTER_1_BASE,(i2c_adr),WRITE)==I2C_ACK) 	// set chip address and set to write/
		{
			I2C_write(I2C_MASTER_1_BASE, TMP007_VSENSOR_VOLTAGE, LAST_BYTE);  		// set internal reg. adr.
			I2C_start(I2C_MASTER_1_BASE, i2c_adr, READ); 	// set chip address in read mode
			data[0] = I2C_read(I2C_MASTER_1_BASE,!LAST_BYTE);
			data[1] = I2C_read(I2C_MASTER_1_BASE,LAST_BYTE);

			temp = (alt_16)((data[0] << 8) + data[1]);

			pRX_Status->ir_sensor[_SensorNo].voltage = (INT32)temp;
		}
		else pRX_Status->ir_sensor[_SensorNo].voltage = INVALID_VALUE;

		// object temp
		if (I2C_start(I2C_MASTER_1_BASE, i2c_adr, WRITE)==I2C_ACK) 		// set chip address and set to write/
		{
			I2C_write(I2C_MASTER_1_BASE, TMP007_TOBJ_TEMP, LAST_BYTE); 	// set internal reg. adr.
			I2C_start(I2C_MASTER_1_BASE, i2c_adr, READ); 				// set chip address in read mode

			data[0] = I2C_read(I2C_MASTER_1_BASE,!LAST_BYTE);
			data[1] = I2C_read(I2C_MASTER_1_BASE,LAST_BYTE);

			if((data[1] & 0x01)==0) //temp is valid
			{
				temp = (alt_16)(((data[0] << 8) + (data[1] & 0xFC))) / 4;
				pRX_Status->ir_sensor[_SensorNo].tempObj = ((INT32)temp*3125)/100;	// *31.25
			}
		}
		else
		{
			pRX_Status->ir_sensor[_SensorNo].tempObj = INVALID_VALUE;
		}

		// local die temp
		if (I2C_start(I2C_MASTER_1_BASE, i2c_adr, WRITE)==I2C_ACK) 	// set chip address and set to write/
		{
			I2C_write(I2C_MASTER_1_BASE, TMP007_TDIE_LOCAL_TEMP, LAST_BYTE);  				// set internal reg. adr.
			I2C_start(I2C_MASTER_1_BASE, i2c_adr, READ); 			// set chip address in read mode
			data[0] = I2C_read(I2C_MASTER_1_BASE,!LAST_BYTE);
			data[1] = I2C_read(I2C_MASTER_1_BASE,LAST_BYTE);

			temp = (alt_16)((data[0] << 8) + (data[1] & 0xFC)) / 4;

			pRX_Status->ir_sensor[_SensorNo].tempDie = ((INT32)temp*3125)/100;	// *31.25
		}
		else pRX_Status->ir_sensor[_SensorNo].tempDie = INVALID_VALUE;
	}

	_SensorNo++;
}
