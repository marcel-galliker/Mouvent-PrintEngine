
#include "alt_types.h"
#include "i2c_master_regs.h"
#include "i2c_master.h"

/* these functions are polled only.  */
/* all functions wait until the I2C is done before exiting */

/****************************************************************
int I2C_init
            This function inititlizes the prescalor for the scl
            and then enables the core. This must be run before
            any other i2c code is executed
inputs
      base = the base address of the component
      clk = freuqency of the clock driving this component  ( in Hz)
      speed = SCL speed ie 100K, 400K ...            (in Hz)
15-OCT-07 initial release
*****************************************************************/
void I2C_init(alt_u32 base,alt_u32 clk,alt_u32 speed)
{
  alt_u32 prescale = (clk/( 5 * speed))-1;

  IOWR_I2C_MASTER_CTR(base, 0x00); /* turn off the core*/

  IOWR_I2C_MASTER_CR(base, I2C_MASTER_CR_IACK_MSK); /* clearn any pening IRQ*/

  IOWR_I2C_MASTER_PRERLO(base, (0xff & prescale));  /* load low presacle bit*/

  IOWR_I2C_MASTER_PRERHI(base, (0xff & (prescale>>8)));  /* load upper prescale bit */

  IOWR_I2C_MASTER_CTR(base, I2C_MASTER_CTR_EN_MSK); /* turn on the core*/

}

/****************************************************************
int I2C_start
            Sets the start bit and then sends the first byte which
            is the address of the device + the write bit.
inputs
      base = the base address of the component
      add = address of I2C device
      read =  1== read    0== write
return value
       0 if address is acknowledged
       1 if address was not acknowledged
15-OCT-07 initial release
*****************************************************************/
int I2C_start(alt_u32 base, alt_u32 add, alt_u32 read)
{
			/* transmit the address shifted by one and the read/write bit*/
	IOWR_I2C_MASTER_TXR(base, ((add<<1) + (0x1 & read)));

			/* set start and write  bits which will start the transaction*/
	IOWR_I2C_MASTER_CR(base, I2C_MASTER_CR_STA_MSK | I2C_MASTER_CR_WR_MSK );

    /* wait for the trnasaction to be over.*/
  
	int count;
	for (count=0; IORD_I2C_MASTER_SR(base) & I2C_MASTER_SR_TIP_MSK; count++)
	{
		if(count > 0x1ff) return (I2C_NOACK);
	}

         /* now check to see if the address was acknowledged */
   if(IORD_I2C_MASTER_SR(base) & I2C_MASTER_SR_RXNACK_MSK)
        return (I2C_NOACK);
   else
       return (I2C_ACK);
}

/****************************************************************
int I2C_read
            assumes that any addressing and start
            has already been done.
            reads one byte of data from the slave.  on the last read
            we don't acknowldge and set the stop bit.
inputs
      base = the base address of the component
      last = on the last read there must not be a ack

return value
       byte read back.
15-OCT-07 initial release
*****************************************************************/
alt_u32 I2C_read(alt_u32 base,alt_u32 last)
{
	static alt_u32 count;

	if( last)
	{
		/* start a read and no ack and stop bit*/
		IOWR_I2C_MASTER_CR(base, I2C_MASTER_CR_RD_MSK |
		I2C_MASTER_CR_NACK_MSK | I2C_MASTER_CR_STO_MSK);
	}
	else
	{
		/* start read*/
		IOWR_I2C_MASTER_CR(base, I2C_MASTER_CR_RD_MSK );
	}
     
	/* wait for the trnasaction to be over.*/
	count = 0;
	
	while( IORD_I2C_MASTER_SR(base) & I2C_MASTER_SR_TIP_MSK)
	{
		if(count++ > 0x1ff)
			return 0;
	}

	/* now read the data */
	return (IORD_I2C_MASTER_RXR(base));

}

/****************************************************************
int I2C_write
            assumes that any addressing and start
            has already been done.
            writes one byte of data from the slave.  
            If last is set the stop bit set.
inputs
      base = the base address of the component
      data = byte to write
      last = on the last read there must not be a ack

return value
       0 if address is acknowledged
       1 if address was not acknowledged
15-OCT-07 initial release
*****************************************************************/
alt_u32 I2C_write(alt_u32 base,alt_u8 data, alt_u32 last)
{
	static alt_u32 count;

	/* transmit the data*/
	IOWR_I2C_MASTER_TXR(base, data);

	if( last)
	{
		/* start a read and no ack and stop bit*/
		IOWR_I2C_MASTER_CR(base, I2C_MASTER_CR_WR_MSK |
		I2C_MASTER_CR_STO_MSK);
	}
	else
	{
		/* start read*/
		IOWR_I2C_MASTER_CR(base, I2C_MASTER_CR_WR_MSK );
	}
    
	/* wait for the trnasaction to be over.*/
	count = 0;
	while( IORD_I2C_MASTER_SR(base) & I2C_MASTER_SR_TIP_MSK)
	{
		if(count++ > 0x7ff)
			return (I2C_NOACK);
	}

	/* now check to see if the address was acknowledged */
	if(IORD_I2C_MASTER_SR(base) & I2C_MASTER_SR_RXNACK_MSK)
		return (I2C_NOACK);
	else
       return (I2C_ACK);
}
