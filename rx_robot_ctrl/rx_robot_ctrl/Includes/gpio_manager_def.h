#ifndef INCLUDES_GPIO_MANAGER_DEF_H_
#define INCLUDES_GPIO_MANAGER_DEF_H_

#include <stdbool.h>
#include <stdint.h>

// I/O count
#define NUMBER_OF_INPUTS	7
#define NUMBER_OF_OUTPUTS	4

// Input names
#define	ENDSTOP_Z_DOWN			1
#define ENDSTOP_Z_UP			2
#define ENDSTOP_SCREW_ROTATION	0
#define ENDSTOP_X_AXIS			5
#define ENDSTOP_GARAGE			4
#define ENDSTOP_Y_AXIS			3

// GPIO pins
#define MOTOR_SD_MODE	23
#define MOTOR0_EN		50
#define MOTOR1_EN		48
#define MOTOR2_EN		46
#define MOTOR3_EN		18
#define MOTOR0_L_REF	31
#define MOTOR1_L_REF	40
#define MOTOR2_L_REF	25
#define MOTOR3_L_REF	19
#define MOTOR0_R_REF	26
#define MOTOR1_R_REF	41
#define MOTOR2_R_REF	24
#define MOTOR3_R_REF	20

#define OUTPUT_0		57
#define OUTPUT_1		61
#define OUTPUT_2		62
#define OUTPUT_3		65

#define INPUT_0			53
#define INPUT_1			54
#define INPUT_2			56
#define INPUT_3			52
#define INPUT_4			55
#define INPUT_5			11
#define INPUT_6			10

#define GPIO_SPIM_CLK	27
#define GPIO_SPIM_MISO	30
#define GPIO_SPIM_MOSI	29
#define GPIO_SPIM_SS0	28
#define GPIO_SPIM_SS1	33
#define GPIO_SPIM_SS2	34
#define GPIO_SPIM_SS3	35

#define WATCHDOG_IO		66


/* Data structures */
#pragma pack(1)

typedef struct {
	uint8_t inputs;
	uint8_t outputs;
	int32_t inputEdges[NUMBER_OF_INPUTS];
} GpioStatus_t;

#pragma pack()

#endif /* INCLUDES_GPIO_MANAGER_DEF_H_ */
