

#ifndef __I2C_MASTER_REGS_H__
#define __I2C_MASTER_REGS_H__

#include <io.h>
/* prescal   clock/(5*desired_SCL) */
/* all registers are 8 bits wide but on 32 bit address boundaries.*/
/* reg definitions take from i2c_specs.pdf in the docs folder */

#define IOADDR_I2C_MASTER_PRERLO(base)           __IO_CALC_ADDRESS_NATIVE(base, 0)
#define IORD_I2C_MASTER_PRERLO(base)             IORD_8DIRECT(base, 0)
#define IOWR_I2C_MASTER_PRERLO(base, data)       IOWR_8DIRECT(base, 0, data)


#define IOADDR_I2C_MASTER_PRERHI(base)           __IO_CALC_ADDRESS_NATIVE(base, 0)
#define IORD_I2C_MASTER_PRERHI(base)             IORD_8DIRECT(base, 1)
#define IOWR_I2C_MASTER_PRERHI(base, data)       IOWR_8DIRECT(base, 1, data)


#define IOADDR_I2C_MASTER_CTR(base)      __IO_CALC_ADDRESS_NATIVE(base, 2)
#define IORD_I2C_MASTER_CTR(base)        IORD_8DIRECT(base, 2)
#define IOWR_I2C_MASTER_CTR(base, data)  IOWR_8DIRECT(base, 2, data)
/* bit definitions*/
#define I2C_MASTER_CTR_EN_MSK             (0x80)
#define I2C_MASTER_CTR_EN_OFST            (7)
#define I2C_MASTER_CTR_IEN_MSK            (0x40)
#define I2C_MASTER_CTR_IEN_OFST           (6)


#define IOADDR_I2C_MASTER_TXR(base)       __IO_CALC_ADDRESS_NATIVE(base, 3)
#define IOWR_I2C_MASTER_TXR(base, data)   IOWR_8DIRECT(base, 3, data)
/* bit definitions*/
#define I2C_MASTER_TXR_RD_MSK             (0x1)
#define I2C_MASTER_TXR_RD_OFST            (0)
#define I2C_MASTER_TXR_WR_MSK             (0x0)
#define I2C_MASTER_TXR_WR_OFST            (0)


#define IOADDR_I2C_MASTER_RXR(base)       __IO_CALC_ADDRESS_NATIVE(base, 3)
#define IORD_I2C_MASTER_RXR(base)         IORD_8DIRECT(base, 3)


#define IOADDR_I2C_MASTER_CR(base)       __IO_CALC_ADDRESS_NATIVE(base, 4)
#define IOWR_I2C_MASTER_CR(base, data)   IOWR_8DIRECT(base, 4, data)
/* bit definitions*/
#define I2C_MASTER_CR_STA_MSK             (0x80)
#define I2C_MASTER_CR_STA_OFST            (7)
#define I2C_MASTER_CR_STO_MSK             (0x40)
#define I2C_MASTER_CR_STO_OFST            (6)
#define I2C_MASTER_CR_RD_MSK              (0x20)
#define I2C_MASTER_CR_RD_OFST             (5)
#define I2C_MASTER_CR_WR_MSK              (0x10)
#define I2C_MASTER_CR_WR_OFST             (4)
#define I2C_MASTER_CR_NACK_MSK             (0x8)
#define I2C_MASTER_CR_NACK_OFST            (3)
#define I2C_MASTER_CR_IACK_MSK            (0x1)
#define I2C_MASTER_CR_IACK_OFST           (0)


#define IOADDR_I2C_MASTER_SR(base)       __IO_CALC_ADDRESS_NATIVE(base, 4)
#define IORD_I2C_MASTER_SR(base)         IORD_8DIRECT(base, 4)
/* bit definitions*/
#define I2C_MASTER_SR_RXNACK_MSK           (0x80)
#define I2C_MASTER_SR_RXNACK_OFST          (7)
#define I2C_MASTER_SR_BUSY_MSK            (0x40)
#define I2C_MASTER_SR_BUSY_OFST           (6)
#define I2C_MASTER_SR_AL_MSK              (0x20)
#define I2C_MASTER_SR_AL_OFST             (5)
#define I2C_MASTER_SR_TIP_MSK             (0x2)
#define I2C_MASTER_SR_TIP_OFST            (1)
#define I2C_MASTER_SR_IF_MSK              (0x1)
#define I2C_MASTER_SR_IF_OFST             (0)

#endif /* __I2C_MASTER_REGS_H__ */
