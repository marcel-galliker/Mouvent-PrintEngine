#ifndef I2C_BITBANG_H
#define I2C_BITBANG_H

#include "rx_ink_common.h"

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/
int i2c_bb_init(void);
BYTE i2c_bb_read_current_address(const BYTE ack);
BYTE i2c_bb_read_random_address(const UINT32u address);
BYTE i2c_bb_read_sequential_address(const UINT32u address, BYTE *ret_values,
                                    BYTE len);
BYTE i2c_bb_write_address(const UINT32u address, const BYTE data);
BYTE i2c_bb_write_page(const UINT32u address, const BYTE *data, BYTE len);
void i2c_bb_reset(void);
BOOL i2c_bb_read_ack(void);

#endif // I2C_BITBANG_H
