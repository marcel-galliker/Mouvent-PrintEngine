// stepper chip documentation: https://www.trinamic.com/fileadmin/assets/Products/ICs_Documents/TMC5130_datasheet_Rev1.17.pdf


// uses the screwer motor to identify board

#include "rx_robot_def.h"
#include "rx_robot_tcpip.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys/types.h"

#include <ft900.h>
#include <gpio.h>
#include <motor.h>

#include "network.h"
#include "rx_trace.h"


#define MOTOR_SCREW	2

#define TMC_RAMPSTAT_POS_REACHED_FLAG	0x80
#define TMC_RAMPSTAT_STOP_SG_FLAG		0x40

#define TMC_STATUS_RESET_FLAG				0X01
#define TMC_STATUS_DRIVER_ERROR_FLAG		0x02
#define TMC_STATUS_SG_FLAG					0x04
#define TMC_STATUS_STANDSTILL_FLAG			0x08
#define TMC_STATUS_VELOCITY_REACHED_FLAG	0x10
#define TMC_STATUS_POSITION_REACHED_FLAG	0x20

#define TMC_GCONF_REG			0x00
#define TMC_GSTAT_REG			0x01
#define TMC_IFCNT_REG			0x02
#define TMC_SLAVECONF_REG		0x03
#define TMC_IOIN_REG			0x04
#define TMC_OUTPUT_REG			0x04
#define TMC_XCOMPARE_REG		0x05
#define TMC_IHOLDIRUN_REG		0x10
#define TMC_TPOWERDOWN_REG		0x11
#define TMC_TSTEP_REG			0x12
#define TMC_TPWMTHRS_REG		0x13
#define TMC_TCOOLTHRS_REG		0x14
#define TMC_THIGH_REG			0x15
#define TMC_RAMPMODE_REG		0x20
#define TMC_XACTUAL_REG			0x21
#define TMC_VACTUAL_REG			0x22
#define TMC_VSTART_REG			0x23
#define TMC_A1_REG				0x24
#define TMC_V1_REG				0x25
#define TMC_AMAX_REG			0x26
#define TMC_VMAX_REG			0x27
#define TMC_DMAX_REG			0x28
#define TMC_D1_REG				0x2A
#define TMC_VSTOP_REG			0x2B
#define TMC_TZEROWAIT_REG		0x2C
#define TMC_XTARGET_REG			0x2D
#define TMC_VDCMIN_REG			0x33
#define TMC_SWMODE_REG			0x34
#define TMC_RAMPSTAT_REG		0x35
#define TMC_XLATCH_REG			0x36
#define TMC_ENCMODE_REG			0x38
#define TMC_XENC_REG			0x39
#define TMC_ENCCONST_REG		0x3A
#define TMC_ENCSTATUS_REG		0x3B
#define TMC_ENCLATCH_REG		0x3C
#define TMC_MSLUT0_REG			0x60
#define TMC_MSLUT1_REG			0x61
#define TMC_MSLUT2_REG			0x62
#define TMC_MSLUT3_REG			0x63
#define TMC_MSLUT4_REG			0x64
#define TMC_MSLUT5_REG			0x65
#define TMC_MSLUT6_REG			0x66
#define TMC_MSLUT7_REG			0x67
#define TMC_MSLUTSEL_REG		0x68
#define TMC_MSLUTSTART_REG		0x69
#define TMC_MSCNT_REG			0x6A
#define TMC_MSCURACT_REG		0x6B
#define TMC_CHOPCONF_REG		0x6C
#define TMC_COOLCONF_REG		0x6D
#define TMC_DCCTRL_REG			0x6E
#define TMC_DRVSTATUS_REG		0x6F
#define TMC_PWMCONF_REG			0x70
#define TMC_PWMSCALE_REG		0x71
#define TMC_ENCMCTRL_REG		0x72
#define TMC_LOSTTEPS_REG		0x73

// Motor settings
#define MOTOR_UPDATE_INTERVAL	(10)

// Task settings
#define TASK_MOTOR_STACK_SIZE	(500)
#define TASK_MOTOR_PRIORITY		(9)

#define SPI_CLOCK_DIVIDER		(64)
#define SPI_FIFO_SIZE			(64)
#define SPI_FIFO_ENABLED		(true)
#define SPI_FIFO_TRIGGER_LEVEL	(1)

#pragma pack(1)
typedef struct SpiTxDatagram {
	uint8_t addr;		// 7 bit address + 1bit RW
	uint32_t data;		// 32 bit data
}SpiTxDatagram_t;

typedef struct SpiRxDatagram {
	uint8_t status;		// 8 bit status
	uint32_t data;		// 32 bit data
} SpiRxDatagram_t;
#pragma pack()

static void _motor_init(void);
// static int32_t _spi_read_register(uint8_t reg, uint8_t motor);
static void _spi_write_register(uint8_t reg, uint32_t data, uint8_t motor);


static int _running=FALSE;

//--- _motor_init --------------------------------------
static void _motor_init(void)
{
	static int _init=FALSE;
	if (!_init)
	{
		_init=TRUE;

		gpio_init();

		spi_init(SPIM, spi_dir_master, spi_mode_3, SPI_CLOCK_DIVIDER);
		spi_option(SPIM, spi_option_fifo_size, SPI_FIFO_SIZE);
		spi_option(SPIM, spi_option_fifo, SPI_FIFO_ENABLED);
		spi_option(SPIM, spi_option_fifo_receive_trigger, SPI_FIFO_TRIGGER_LEVEL);


		//--- _set_config -----------------------------------------------------------
		//		parameters see rx_stepper_ctrl::robot_client.c

		#define IHOLD_IRUN(hold, run, delay)	((((delay)&0xf)<<16) | (((run)&0x1f)<<8) | ((hold)&0x1f))
		int current = IHOLD_IRUN(0, 18, 1);
		int speed   = 160000;

		_spi_write_register(TMC_GCONF_REG, 		0, 		 MOTOR_SCREW);
		_spi_write_register(TMC_SWMODE_REG,		0x03, 	 MOTOR_SCREW);
		_spi_write_register(TMC_IHOLDIRUN_REG, 	current, MOTOR_SCREW);
		_spi_write_register(TMC_TPOWERDOWN_REG, 0,		 MOTOR_SCREW);
		_spi_write_register(TMC_TPWMTHRS_REG, 	0,  	 MOTOR_SCREW);
		_spi_write_register(TMC_RAMPMODE_REG, 	0,  	 MOTOR_SCREW);
		_spi_write_register(TMC_VMAX_REG, 		speed,   MOTOR_SCREW);
		_spi_write_register(TMC_V1_REG, 		0, 		 MOTOR_SCREW);
		_spi_write_register(TMC_AMAX_REG, 		4*2932,  MOTOR_SCREW);
		_spi_write_register(TMC_DMAX_REG, 		4*2932,  MOTOR_SCREW);
		_spi_write_register(TMC_A1_REG, 		2932, 	 MOTOR_SCREW);
		_spi_write_register(TMC_D1_REG, 		2932, 	 MOTOR_SCREW);
		_spi_write_register(TMC_VSTART_REG, 	0, 	 	 MOTOR_SCREW);
		_spi_write_register(TMC_VSTOP_REG, 		10, 	 MOTOR_SCREW);
		_spi_write_register(TMC_TZEROWAIT_REG, 	0, 		 MOTOR_SCREW);
		_spi_write_register(TMC_VDCMIN_REG, 	0, 	 	 MOTOR_SCREW);
		_spi_write_register(TMC_CHOPCONF_REG, 	0x101F5, MOTOR_SCREW);
		_spi_write_register(TMC_COOLCONF_REG, 	0,  	 MOTOR_SCREW);
		_spi_write_register(TMC_PWMCONF_REG, 	0, 	 	 MOTOR_SCREW);
		_spi_write_register(TMC_TCOOLTHRS_REG, 	0,		 MOTOR_SCREW);
		_spi_write_register(TMC_THIGH_REG, 		0, 	 	 MOTOR_SCREW);
		_spi_write_register(TMC_ENCMODE_REG, 	0, 	 	 MOTOR_SCREW);
		_spi_write_register(TMC_ENCCONST_REG, 	0,  	 MOTOR_SCREW);
	}
}

//--- motor_flash_start -----------------------------------------------
void motor_flash_start(void)
{
	_motor_init();

	gpio_enable_motor(MOTOR_SCREW, TRUE);
	gpio_stop_motor(MOTOR_SCREW);
	_spi_write_register(TMC_XACTUAL_REG, 0, MOTOR_SCREW);
	_spi_write_register(TMC_XTARGET_REG, 0x7fffffff, MOTOR_SCREW);
	gpio_start_motor(MOTOR_SCREW);
	_running = TRUE;
}

//--- motor_flash_stop -------------------------------------------------------
void motor_flash_stop(void)
{
	if (_running)
	{
		gpio_stop_motor(MOTOR_SCREW);

		_spi_write_register(TMC_XTARGET_REG, 0, MOTOR_SCREW);
		_spi_write_register(TMC_XACTUAL_REG, 0, MOTOR_SCREW);

		gpio_enable_motor(MOTOR_SCREW, FALSE);
	}
	_running = FALSE;
}

/*
static int32_t _spi_read_register(uint8_t reg, uint8_t motor)
{
	int32_t data;
	SpiTxDatagram_t txDatagram;
	SpiRxDatagram_t rxDatagram;

	if(motor >= MOTOR_CNT)
		return 0;

	// Zero structs
	memset(&txDatagram, 0, sizeof(txDatagram));
	memset(&rxDatagram, 0, sizeof(rxDatagram));

	// Set Address and read bit
	txDatagram.addr = (reg & 0x7F);

	// Send reg to read from and read return value
	spi_open(SPIM, motor);
	spi_xchangen(SPIM, (uint8_t *)&rxDatagram, (uint8_t *)&txDatagram, sizeof(rxDatagram));
	spi_close(SPIM, motor);

	spi_open(SPIM, motor);
	spi_xchangen(SPIM, (uint8_t *)&rxDatagram, (uint8_t *)&txDatagram, sizeof(rxDatagram));
	spi_close(SPIM, motor);

	data = __bswap32(rxDatagram.data);

	return data;
}
*/

static void _spi_write_register(uint8_t reg, uint32_t data, uint8_t motor)
{
	SpiTxDatagram_t txDatagram;
	SpiRxDatagram_t rxDatagram;

	// Zero structs
	memset(&txDatagram, 0, sizeof(txDatagram));

	// Set Address and write bit
	txDatagram.addr = (reg | 0x80);

	// Set data
	txDatagram.data = __bswap32(data);

	spi_open(SPIM, motor);
	spi_xchangen(SPIM, (uint8_t *)&rxDatagram, (uint8_t *)&txDatagram, sizeof(rxDatagram));
	spi_close(SPIM, motor);
}
