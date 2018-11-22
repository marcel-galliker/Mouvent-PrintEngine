
#ifndef _MAX31865_H__
#define __MAX31865_H__

void init_MAX31865(void);

#define MAX31865_CONFIG				(1 * 0x0)
#define MAX31865_RTD_DATA_MSB		(1 * 0x1)
#define MAX31865_RTD_DATA_LSB		(1 * 0x2)
#define MAX31865_FAULT_HTHR_MSB		(1 * 0x3)
#define MAX31865_FAULT_HTHR_LSB		(1 * 0x4)
#define MAX31865_FAULT_LTHR_MSB		(1 * 0x5)
#define MAX31865_FAULT_LTHR_LSB		(1 * 0x6)
#define MAX31865_FAULT_STATUS		(1 * 0x7)

#define MAX_PT100					6

#endif /* __MAX31865_H__ */
