
#ifndef __AD9102_H__
#define __AD9102_H__

int init_DAC(alt_u32 base);
int fill_wave_ram(alt_u32 base,alt_u8 fpNr);

#define FP1						15
#define FP2						16
#define FP3						17
#define FPA					 	5

#define AD9102_SPICONFIG		(2 * 0x00)
#define AD9102_POWERCONFIG		(2 * 0x01)
#define AD9102_CLOCKCONFIG		(2 * 0x02)
#define AD9102_REFADJ			(2 * 0x03)
#define AD9102_DACGAIN			(2 * 0x07)
#define AD9102_DACRANGE			(2 * 0x08)
#define AD9102_DACRSET			(2 * 0x0C)
#define AD9102_CALCONFIG		(2 * 0x0D)
#define AD9102_COMPOFFSET		(2 * 0x0E)
#define AD9102_RAMUPDATE		(2 * 0x1D)
#define AD9102_PAT_STATUS 		(2 * 0x1E)
#define AD9102_PAT_TYPE			(2 * 0x1F)
#define AD9102_PATTERN_DLY		(2 * 0x20)
#define AD9102_DACDOF			(2 * 0x25)
#define AD9102_WAV_CONFIG		(2 * 0x27)
#define AD9102_PAT_TIMEBASE		(2 * 0x28)
#define AD9102_PAT_PERIOD		(2 * 0x29)
#define AD9102_DAC_PAT			(2 * 0x2B)
#define AD9102_DOUT_START		(2 * 0x2C)
#define AD9102_DOUT_CONFIG		(2 * 0x2D)
#define AD9102_DAC_CST			(2 * 0x31)
#define AD9102_DAC_DGAIN		(2 * 0x35)
#define AD9102_SAW_CONFIG		(2 * 0x37)
#define AD9102_DDS_TW32			(2 * 0x3E)
#define AD9102_DDS_TW1			(2 * 0x3F)
#define AD9102_DDS_PW			(2 * 0x43)
#define AD9102_TRIG_TW_SEL		(2 * 0x44)
#define AD9102_DDS_CONFIG		(2 * 0x45)
#define AD9102_TW_RAM_CONFIG	(2 * 0x47)
#define AD9102_START_DLY		(2 * 0x5C)
#define AD9102_START_ADDR		(2 * 0x5D)
#define AD9102_STOP_ADDR		(2 * 0x5E)
#define AD9102_DDS_CYC			(2 * 0x5F)
#define AD9102_CFG_ERROR		(2 * 0x60)
#define AD9102_SRAM_DATA  		(2 * 0x6000)

#endif /* __AD9102_H__ */
