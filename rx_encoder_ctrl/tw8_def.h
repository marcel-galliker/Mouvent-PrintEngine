#ifndef _TW8_H_
#define _TW8_H_

#include <unistd.h>

#ifdef TW8
	typedef UINT8  __int8_t;
	typedef UINT16 __uint16_t;
#endif

// Communication defines
#define TW8_WRITE_MEM 0x80
#define TW8_READ_MEM 0xC0

// Calibration (ON = Set gpio LOW, OFF = set gpio high; Calibrate is active low)
// The default state is OFF on powerup
#define TW8_CALIBRATE_ON 0xEC
#define TW8_CALIBRATE_OFF 0xDC


// Chips
#define CHIP_0 0x64
#define CHIP_1 0x65

// TW8 communication error
#define TW8_SUCCESS 0x00
#define TW8_INVALID_COMMAND 0x01
#define TW8_READ_ERROR 0x02
#define TW8_WRITE_ERROR 0x03

#define DATA_SIZE	256

// Datatypes
// Disable padding
#pragma pack(push, 1)

// Communication data structures
typedef struct _rx_tw8_ctrl_cmd {
	UINT8 chip;
	UINT8 cmd;
	UINT16 addr;
	UINT8 length;
	UINT8 data[DATA_SIZE];
} rx_tw8_ctrl_cmd;

typedef struct _rx_tw8_ctrl_answer {
	UINT8 chip;
	UINT8 err;
	UINT16 addr;
	UINT8 length;
	UINT8 data[DATA_SIZE];
} rx_tw8_ctrl_answer;
	
// Go back to default padding
#pragma pack(pop)

#endif /* _TW8_H_ */
