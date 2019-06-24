/*
 * system.h - SOPC Builder system and BSP software package information
 *
 * Machine generated for CPU 'nios2_qsys_0' in SOPC Builder design 'fluid_board_soc'
 * SOPC Builder design path: ../../fluid_board_soc.sopcinfo
 *
 * Generated: Mon Sep 03 10:26:15 CEST 2018
 */

/*
 * DO NOT MODIFY THIS FILE
 *
 * Changing this file will have subtle consequences
 * which will almost certainly lead to a nonfunctioning
 * system. If you do modify this file, be aware that your
 * changes will be overwritten and lost when this file
 * is generated again.
 *
 * DO NOT MODIFY THIS FILE
 */

/*
 * License Agreement
 *
 * Copyright (c) 2008
 * Altera Corporation, San Jose, California, USA.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This agreement shall be governed in all respects by the laws of the State
 * of California and by the laws of the United States of America.
 */

#ifndef __SYSTEM_H_
#define __SYSTEM_H_

/* Include definitions from linker script generator */
#include "linker.h"


/*
 * CPU configuration
 *
 */

#define ALT_CPU_ARCHITECTURE "altera_nios2_gen2"
#define ALT_CPU_BIG_ENDIAN 0
#define ALT_CPU_BREAK_ADDR 0x00001020
#define ALT_CPU_CPU_ARCH_NIOS2_R1
#define ALT_CPU_CPU_FREQ 50000000u
#define ALT_CPU_CPU_ID_SIZE 1
#define ALT_CPU_CPU_ID_VALUE 0x00000000
#define ALT_CPU_CPU_IMPLEMENTATION "fast"
#define ALT_CPU_DATA_ADDR_WIDTH 0x13
#define ALT_CPU_DCACHE_LINE_SIZE 0
#define ALT_CPU_DCACHE_LINE_SIZE_LOG2 0
#define ALT_CPU_DCACHE_SIZE 0
#define ALT_CPU_EXCEPTION_ADDR 0x00010020
#define ALT_CPU_FLASH_ACCELERATOR_LINES 0
#define ALT_CPU_FLASH_ACCELERATOR_LINE_SIZE 0
#define ALT_CPU_FLUSHDA_SUPPORTED
#define ALT_CPU_FREQ 50000000
#define ALT_CPU_HARDWARE_DIVIDE_PRESENT 0
#define ALT_CPU_HARDWARE_MULTIPLY_PRESENT 1
#define ALT_CPU_HARDWARE_MULX_PRESENT 0
#define ALT_CPU_HAS_DEBUG_CORE 1
#define ALT_CPU_HAS_DEBUG_STUB
#define ALT_CPU_HAS_EXTRA_EXCEPTION_INFO
#define ALT_CPU_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define ALT_CPU_HAS_JMPI_INSTRUCTION
#define ALT_CPU_ICACHE_LINE_SIZE 32
#define ALT_CPU_ICACHE_LINE_SIZE_LOG2 5
#define ALT_CPU_ICACHE_SIZE 512
#define ALT_CPU_INST_ADDR_WIDTH 0x11
#define ALT_CPU_NAME "nios2_qsys_0"
#define ALT_CPU_NUM_OF_SHADOW_REG_SETS 0
#define ALT_CPU_OCI_VERSION 1
#define ALT_CPU_RESET_ADDR 0x00010000


/*
 * CPU configuration (with legacy prefix - don't use these anymore)
 *
 */

#define NIOS2_BIG_ENDIAN 0
#define NIOS2_BREAK_ADDR 0x00001020
#define NIOS2_CPU_ARCH_NIOS2_R1
#define NIOS2_CPU_FREQ 50000000u
#define NIOS2_CPU_ID_SIZE 1
#define NIOS2_CPU_ID_VALUE 0x00000000
#define NIOS2_CPU_IMPLEMENTATION "fast"
#define NIOS2_DATA_ADDR_WIDTH 0x13
#define NIOS2_DCACHE_LINE_SIZE 0
#define NIOS2_DCACHE_LINE_SIZE_LOG2 0
#define NIOS2_DCACHE_SIZE 0
#define NIOS2_EXCEPTION_ADDR 0x00010020
#define NIOS2_FLASH_ACCELERATOR_LINES 0
#define NIOS2_FLASH_ACCELERATOR_LINE_SIZE 0
#define NIOS2_FLUSHDA_SUPPORTED
#define NIOS2_HARDWARE_DIVIDE_PRESENT 0
#define NIOS2_HARDWARE_MULTIPLY_PRESENT 1
#define NIOS2_HARDWARE_MULX_PRESENT 0
#define NIOS2_HAS_DEBUG_CORE 1
#define NIOS2_HAS_DEBUG_STUB
#define NIOS2_HAS_EXTRA_EXCEPTION_INFO
#define NIOS2_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define NIOS2_HAS_JMPI_INSTRUCTION
#define NIOS2_ICACHE_LINE_SIZE 32
#define NIOS2_ICACHE_LINE_SIZE_LOG2 5
#define NIOS2_ICACHE_SIZE 512
#define NIOS2_INST_ADDR_WIDTH 0x11
#define NIOS2_NUM_OF_SHADOW_REG_SETS 0
#define NIOS2_OCI_VERSION 1
#define NIOS2_RESET_ADDR 0x00010000


/*
 * Define for each module class mastered by the CPU
 *
 */

#define __ALTERA_AVALON_JTAG_UART
#define __ALTERA_AVALON_ONCHIP_MEMORY2
#define __ALTERA_AVALON_PIO
#define __ALTERA_AVALON_SYSID_QSYS
#define __ALTERA_AVALON_TIMER
#define __ALTERA_NIOS2_GEN2
#define __AVALON2FPGA_SLAVE
#define __AVALON_SPI_AMC7891
#define __AVALON_SPI_MAX31865
#define __AXI_LW_SLAVE_REGISTER
#define __I2C_MASTER


/*
 * System configuration
 *
 */

#define ALT_DEVICE_FAMILY "Cyclone V"
#define ALT_ENHANCED_INTERRUPT_API_PRESENT
#define ALT_IRQ_BASE NULL
#define ALT_LOG_PORT "/dev/null"
#define ALT_LOG_PORT_BASE 0x0
#define ALT_LOG_PORT_DEV null
#define ALT_LOG_PORT_TYPE ""
#define ALT_NUM_EXTERNAL_INTERRUPT_CONTROLLERS 0
#define ALT_NUM_INTERNAL_INTERRUPT_CONTROLLERS 1
#define ALT_NUM_INTERRUPT_CONTROLLERS 1
#define ALT_STDERR "/dev/jtag_uart"
#define ALT_STDERR_BASE 0x0
#define ALT_STDERR_DEV jtag_uart
#define ALT_STDERR_IS_JTAG_UART
#define ALT_STDERR_PRESENT
#define ALT_STDERR_TYPE "altera_avalon_jtag_uart"
#define ALT_STDIN "/dev/jtag_uart"
#define ALT_STDIN_BASE 0x0
#define ALT_STDIN_DEV jtag_uart
#define ALT_STDIN_IS_JTAG_UART
#define ALT_STDIN_PRESENT
#define ALT_STDIN_TYPE "altera_avalon_jtag_uart"
#define ALT_STDOUT "/dev/jtag_uart"
#define ALT_STDOUT_BASE 0x0
#define ALT_STDOUT_DEV jtag_uart
#define ALT_STDOUT_IS_JTAG_UART
#define ALT_STDOUT_PRESENT
#define ALT_STDOUT_TYPE "altera_avalon_jtag_uart"
#define ALT_SYSTEM_NAME "fluid_board_soc"


/*
 * avalon2fpga_slave_0 configuration
 *
 */

#define ALT_MODULE_CLASS_avalon2fpga_slave_0 avalon2fpga_slave
#define AVALON2FPGA_SLAVE_0_BASE 0x200
#define AVALON2FPGA_SLAVE_0_IRQ -1
#define AVALON2FPGA_SLAVE_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AVALON2FPGA_SLAVE_0_NAME "/dev/avalon2fpga_slave_0"
#define AVALON2FPGA_SLAVE_0_SPAN 256
#define AVALON2FPGA_SLAVE_0_TYPE "avalon2fpga_slave"


/*
 * avalon_spi_amc7891_1 configuration
 *
 */

#define ALT_MODULE_CLASS_avalon_spi_amc7891_1 avalon_spi_amc7891
#define AVALON_SPI_AMC7891_1_BASE 0x100
#define AVALON_SPI_AMC7891_1_IRQ -1
#define AVALON_SPI_AMC7891_1_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AVALON_SPI_AMC7891_1_NAME "/dev/avalon_spi_amc7891_1"
#define AVALON_SPI_AMC7891_1_SPAN 256
#define AVALON_SPI_AMC7891_1_TYPE "avalon_spi_amc7891"


/*
 * avalon_spi_max31865_0 configuration
 *
 */

#define ALT_MODULE_CLASS_avalon_spi_max31865_0 avalon_spi_max31865
#define AVALON_SPI_MAX31865_0_BASE 0x300
#define AVALON_SPI_MAX31865_0_IRQ -1
#define AVALON_SPI_MAX31865_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AVALON_SPI_MAX31865_0_NAME "/dev/avalon_spi_max31865_0"
#define AVALON_SPI_MAX31865_0_SPAN 16
#define AVALON_SPI_MAX31865_0_TYPE "avalon_spi_max31865"


/*
 * avalon_spi_max31865_1 configuration
 *
 */

#define ALT_MODULE_CLASS_avalon_spi_max31865_1 avalon_spi_max31865
#define AVALON_SPI_MAX31865_1_BASE 0x310
#define AVALON_SPI_MAX31865_1_IRQ -1
#define AVALON_SPI_MAX31865_1_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AVALON_SPI_MAX31865_1_NAME "/dev/avalon_spi_max31865_1"
#define AVALON_SPI_MAX31865_1_SPAN 16
#define AVALON_SPI_MAX31865_1_TYPE "avalon_spi_max31865"


/*
 * avalon_spi_max31865_2 configuration
 *
 */

#define ALT_MODULE_CLASS_avalon_spi_max31865_2 avalon_spi_max31865
#define AVALON_SPI_MAX31865_2_BASE 0x320
#define AVALON_SPI_MAX31865_2_IRQ -1
#define AVALON_SPI_MAX31865_2_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AVALON_SPI_MAX31865_2_NAME "/dev/avalon_spi_max31865_2"
#define AVALON_SPI_MAX31865_2_SPAN 16
#define AVALON_SPI_MAX31865_2_TYPE "avalon_spi_max31865"


/*
 * avalon_spi_max31865_3 configuration
 *
 */

#define ALT_MODULE_CLASS_avalon_spi_max31865_3 avalon_spi_max31865
#define AVALON_SPI_MAX31865_3_BASE 0x330
#define AVALON_SPI_MAX31865_3_IRQ -1
#define AVALON_SPI_MAX31865_3_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AVALON_SPI_MAX31865_3_NAME "/dev/avalon_spi_max31865_3"
#define AVALON_SPI_MAX31865_3_SPAN 16
#define AVALON_SPI_MAX31865_3_TYPE "avalon_spi_max31865"


/*
 * avalon_spi_max31865_4 configuration
 *
 */

#define ALT_MODULE_CLASS_avalon_spi_max31865_4 avalon_spi_max31865
#define AVALON_SPI_MAX31865_4_BASE 0x340
#define AVALON_SPI_MAX31865_4_IRQ -1
#define AVALON_SPI_MAX31865_4_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AVALON_SPI_MAX31865_4_NAME "/dev/avalon_spi_max31865_4"
#define AVALON_SPI_MAX31865_4_SPAN 16
#define AVALON_SPI_MAX31865_4_TYPE "avalon_spi_max31865"


/*
 * avalon_spi_max31865_5 configuration
 *
 */

#define ALT_MODULE_CLASS_avalon_spi_max31865_5 avalon_spi_max31865
#define AVALON_SPI_MAX31865_5_BASE 0x350
#define AVALON_SPI_MAX31865_5_IRQ -1
#define AVALON_SPI_MAX31865_5_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AVALON_SPI_MAX31865_5_NAME "/dev/avalon_spi_max31865_5"
#define AVALON_SPI_MAX31865_5_SPAN 16
#define AVALON_SPI_MAX31865_5_TYPE "avalon_spi_max31865"


/*
 * axi_lw_slave_register_0 configuration
 *
 */

#define ALT_MODULE_CLASS_axi_lw_slave_register_0 axi_lw_slave_register
#define AXI_LW_SLAVE_REGISTER_0_BASE 0x20000
#define AXI_LW_SLAVE_REGISTER_0_IRQ -1
#define AXI_LW_SLAVE_REGISTER_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AXI_LW_SLAVE_REGISTER_0_NAME "/dev/axi_lw_slave_register_0"
#define AXI_LW_SLAVE_REGISTER_0_SPAN 65536
#define AXI_LW_SLAVE_REGISTER_0_TYPE "axi_lw_slave_register"


/*
 * flush_pump_pwm_duty_cycle configuration
 *
 */

#define ALT_MODULE_CLASS_flush_pump_pwm_duty_cycle altera_avalon_pio
#define FLUSH_PUMP_PWM_DUTY_CYCLE_BASE 0xd0
#define FLUSH_PUMP_PWM_DUTY_CYCLE_BIT_CLEARING_EDGE_REGISTER 0
#define FLUSH_PUMP_PWM_DUTY_CYCLE_BIT_MODIFYING_OUTPUT_REGISTER 0
#define FLUSH_PUMP_PWM_DUTY_CYCLE_CAPTURE 0
#define FLUSH_PUMP_PWM_DUTY_CYCLE_DATA_WIDTH 32
#define FLUSH_PUMP_PWM_DUTY_CYCLE_DO_TEST_BENCH_WIRING 0
#define FLUSH_PUMP_PWM_DUTY_CYCLE_DRIVEN_SIM_VALUE 0
#define FLUSH_PUMP_PWM_DUTY_CYCLE_EDGE_TYPE "NONE"
#define FLUSH_PUMP_PWM_DUTY_CYCLE_FREQ 50000000
#define FLUSH_PUMP_PWM_DUTY_CYCLE_HAS_IN 0
#define FLUSH_PUMP_PWM_DUTY_CYCLE_HAS_OUT 1
#define FLUSH_PUMP_PWM_DUTY_CYCLE_HAS_TRI 0
#define FLUSH_PUMP_PWM_DUTY_CYCLE_IRQ -1
#define FLUSH_PUMP_PWM_DUTY_CYCLE_IRQ_INTERRUPT_CONTROLLER_ID -1
#define FLUSH_PUMP_PWM_DUTY_CYCLE_IRQ_TYPE "NONE"
#define FLUSH_PUMP_PWM_DUTY_CYCLE_NAME "/dev/flush_pump_pwm_duty_cycle"
#define FLUSH_PUMP_PWM_DUTY_CYCLE_RESET_VALUE 0
#define FLUSH_PUMP_PWM_DUTY_CYCLE_SPAN 32
#define FLUSH_PUMP_PWM_DUTY_CYCLE_TYPE "altera_avalon_pio"


/*
 * flush_pump_pwm_freq configuration
 *
 */

#define ALT_MODULE_CLASS_flush_pump_pwm_freq altera_avalon_pio
#define FLUSH_PUMP_PWM_FREQ_BASE 0xc0
#define FLUSH_PUMP_PWM_FREQ_BIT_CLEARING_EDGE_REGISTER 0
#define FLUSH_PUMP_PWM_FREQ_BIT_MODIFYING_OUTPUT_REGISTER 0
#define FLUSH_PUMP_PWM_FREQ_CAPTURE 0
#define FLUSH_PUMP_PWM_FREQ_DATA_WIDTH 32
#define FLUSH_PUMP_PWM_FREQ_DO_TEST_BENCH_WIRING 0
#define FLUSH_PUMP_PWM_FREQ_DRIVEN_SIM_VALUE 0
#define FLUSH_PUMP_PWM_FREQ_EDGE_TYPE "NONE"
#define FLUSH_PUMP_PWM_FREQ_FREQ 50000000
#define FLUSH_PUMP_PWM_FREQ_HAS_IN 0
#define FLUSH_PUMP_PWM_FREQ_HAS_OUT 1
#define FLUSH_PUMP_PWM_FREQ_HAS_TRI 0
#define FLUSH_PUMP_PWM_FREQ_IRQ -1
#define FLUSH_PUMP_PWM_FREQ_IRQ_INTERRUPT_CONTROLLER_ID -1
#define FLUSH_PUMP_PWM_FREQ_IRQ_TYPE "NONE"
#define FLUSH_PUMP_PWM_FREQ_NAME "/dev/flush_pump_pwm_freq"
#define FLUSH_PUMP_PWM_FREQ_RESET_VALUE 0
#define FLUSH_PUMP_PWM_FREQ_SPAN 32
#define FLUSH_PUMP_PWM_FREQ_TYPE "altera_avalon_pio"


/*
 * hal configuration
 *
 */

#define ALT_INCLUDE_INSTRUCTION_RELATED_EXCEPTION_API
#define ALT_MAX_FD 4
#define ALT_SYS_CLK TIMER_0
#define ALT_TIMESTAMP_CLK none


/*
 * i2c_master_d configuration
 *
 */

#define ALT_MODULE_CLASS_i2c_master_d i2c_master
#define I2C_MASTER_D_BASE 0x90
#define I2C_MASTER_D_IRQ -1
#define I2C_MASTER_D_IRQ_INTERRUPT_CONTROLLER_ID -1
#define I2C_MASTER_D_NAME "/dev/i2c_master_d"
#define I2C_MASTER_D_SPAN 8
#define I2C_MASTER_D_TYPE "i2c_master"


/*
 * i2c_master_f configuration
 *
 */

#define ALT_MODULE_CLASS_i2c_master_f i2c_master
#define I2C_MASTER_F_BASE 0x88
#define I2C_MASTER_F_IRQ -1
#define I2C_MASTER_F_IRQ_INTERRUPT_CONTROLLER_ID -1
#define I2C_MASTER_F_NAME "/dev/i2c_master_f"
#define I2C_MASTER_F_SPAN 8
#define I2C_MASTER_F_TYPE "i2c_master"


/*
 * i2c_master_is1 configuration
 *
 */

#define ALT_MODULE_CLASS_i2c_master_is1 i2c_master
#define I2C_MASTER_IS1_BASE 0xb0
#define I2C_MASTER_IS1_IRQ -1
#define I2C_MASTER_IS1_IRQ_INTERRUPT_CONTROLLER_ID -1
#define I2C_MASTER_IS1_NAME "/dev/i2c_master_is1"
#define I2C_MASTER_IS1_SPAN 8
#define I2C_MASTER_IS1_TYPE "i2c_master"


/*
 * i2c_master_is2 configuration
 *
 */

#define ALT_MODULE_CLASS_i2c_master_is2 i2c_master
#define I2C_MASTER_IS2_BASE 0xa8
#define I2C_MASTER_IS2_IRQ -1
#define I2C_MASTER_IS2_IRQ_INTERRUPT_CONTROLLER_ID -1
#define I2C_MASTER_IS2_NAME "/dev/i2c_master_is2"
#define I2C_MASTER_IS2_SPAN 8
#define I2C_MASTER_IS2_TYPE "i2c_master"


/*
 * i2c_master_is3 configuration
 *
 */

#define ALT_MODULE_CLASS_i2c_master_is3 i2c_master
#define I2C_MASTER_IS3_BASE 0xa0
#define I2C_MASTER_IS3_IRQ -1
#define I2C_MASTER_IS3_IRQ_INTERRUPT_CONTROLLER_ID -1
#define I2C_MASTER_IS3_NAME "/dev/i2c_master_is3"
#define I2C_MASTER_IS3_SPAN 8
#define I2C_MASTER_IS3_TYPE "i2c_master"


/*
 * i2c_master_is4 configuration
 *
 */

#define ALT_MODULE_CLASS_i2c_master_is4 i2c_master
#define I2C_MASTER_IS4_BASE 0x98
#define I2C_MASTER_IS4_IRQ -1
#define I2C_MASTER_IS4_IRQ_INTERRUPT_CONTROLLER_ID -1
#define I2C_MASTER_IS4_NAME "/dev/i2c_master_is4"
#define I2C_MASTER_IS4_SPAN 8
#define I2C_MASTER_IS4_TYPE "i2c_master"


/*
 * i2c_master_p configuration
 *
 */

#define ALT_MODULE_CLASS_i2c_master_p i2c_master
#define I2C_MASTER_P_BASE 0x80
#define I2C_MASTER_P_IRQ -1
#define I2C_MASTER_P_IRQ_INTERRUPT_CONTROLLER_ID -1
#define I2C_MASTER_P_NAME "/dev/i2c_master_p"
#define I2C_MASTER_P_SPAN 8
#define I2C_MASTER_P_TYPE "i2c_master"


/*
 * jtag_uart configuration
 *
 */

#define ALT_MODULE_CLASS_jtag_uart altera_avalon_jtag_uart
#define JTAG_UART_BASE 0x0
#define JTAG_UART_IRQ 3
#define JTAG_UART_IRQ_INTERRUPT_CONTROLLER_ID 0
#define JTAG_UART_NAME "/dev/jtag_uart"
#define JTAG_UART_READ_DEPTH 64
#define JTAG_UART_READ_THRESHOLD 8
#define JTAG_UART_SPAN 16
#define JTAG_UART_TYPE "altera_avalon_jtag_uart"
#define JTAG_UART_WRITE_DEPTH 64
#define JTAG_UART_WRITE_THRESHOLD 8


/*
 * onchip_memory_nios_arm configuration
 *
 */

#define ALT_MODULE_CLASS_onchip_memory_nios_arm altera_avalon_onchip_memory2
#define ONCHIP_MEMORY_NIOS_ARM_ALLOW_IN_SYSTEM_MEMORY_CONTENT_EDITOR 0
#define ONCHIP_MEMORY_NIOS_ARM_ALLOW_MRAM_SIM_CONTENTS_ONLY_FILE 0
#define ONCHIP_MEMORY_NIOS_ARM_BASE 0x50000
#define ONCHIP_MEMORY_NIOS_ARM_CONTENTS_INFO ""
#define ONCHIP_MEMORY_NIOS_ARM_DUAL_PORT 1
#define ONCHIP_MEMORY_NIOS_ARM_GUI_RAM_BLOCK_TYPE "AUTO"
#define ONCHIP_MEMORY_NIOS_ARM_INIT_CONTENTS_FILE "fluid_board_soc_onchip_memory_nios_arm"
#define ONCHIP_MEMORY_NIOS_ARM_INIT_MEM_CONTENT 1
#define ONCHIP_MEMORY_NIOS_ARM_INSTANCE_ID "NONE"
#define ONCHIP_MEMORY_NIOS_ARM_IRQ -1
#define ONCHIP_MEMORY_NIOS_ARM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define ONCHIP_MEMORY_NIOS_ARM_NAME "/dev/onchip_memory_nios_arm"
#define ONCHIP_MEMORY_NIOS_ARM_NON_DEFAULT_INIT_FILE_ENABLED 0
#define ONCHIP_MEMORY_NIOS_ARM_RAM_BLOCK_TYPE "AUTO"
#define ONCHIP_MEMORY_NIOS_ARM_READ_DURING_WRITE_MODE "DONT_CARE"
#define ONCHIP_MEMORY_NIOS_ARM_SINGLE_CLOCK_OP 1
#define ONCHIP_MEMORY_NIOS_ARM_SIZE_MULTIPLE 1
#define ONCHIP_MEMORY_NIOS_ARM_SIZE_VALUE 32768
#define ONCHIP_MEMORY_NIOS_ARM_SPAN 32768
#define ONCHIP_MEMORY_NIOS_ARM_TYPE "altera_avalon_onchip_memory2"
#define ONCHIP_MEMORY_NIOS_ARM_WRITABLE 1


/*
 * onchip_memory_nios_cpu configuration
 *
 */

#define ALT_MODULE_CLASS_onchip_memory_nios_cpu altera_avalon_onchip_memory2
#define ONCHIP_MEMORY_NIOS_CPU_ALLOW_IN_SYSTEM_MEMORY_CONTENT_EDITOR 0
#define ONCHIP_MEMORY_NIOS_CPU_ALLOW_MRAM_SIM_CONTENTS_ONLY_FILE 0
#define ONCHIP_MEMORY_NIOS_CPU_BASE 0x10000
#define ONCHIP_MEMORY_NIOS_CPU_CONTENTS_INFO ""
#define ONCHIP_MEMORY_NIOS_CPU_DUAL_PORT 1
#define ONCHIP_MEMORY_NIOS_CPU_GUI_RAM_BLOCK_TYPE "AUTO"
#define ONCHIP_MEMORY_NIOS_CPU_INIT_CONTENTS_FILE "fluid_board_soc_onchip_memory_nios_cpu"
#define ONCHIP_MEMORY_NIOS_CPU_INIT_MEM_CONTENT 1
#define ONCHIP_MEMORY_NIOS_CPU_INSTANCE_ID "NONE"
#define ONCHIP_MEMORY_NIOS_CPU_IRQ -1
#define ONCHIP_MEMORY_NIOS_CPU_IRQ_INTERRUPT_CONTROLLER_ID -1
#define ONCHIP_MEMORY_NIOS_CPU_NAME "/dev/onchip_memory_nios_cpu"
#define ONCHIP_MEMORY_NIOS_CPU_NON_DEFAULT_INIT_FILE_ENABLED 0
#define ONCHIP_MEMORY_NIOS_CPU_RAM_BLOCK_TYPE "AUTO"
#define ONCHIP_MEMORY_NIOS_CPU_READ_DURING_WRITE_MODE "DONT_CARE"
#define ONCHIP_MEMORY_NIOS_CPU_SINGLE_CLOCK_OP 1
#define ONCHIP_MEMORY_NIOS_CPU_SIZE_MULTIPLE 1
#define ONCHIP_MEMORY_NIOS_CPU_SIZE_VALUE 65536
#define ONCHIP_MEMORY_NIOS_CPU_SPAN 65536
#define ONCHIP_MEMORY_NIOS_CPU_TYPE "altera_avalon_onchip_memory2"
#define ONCHIP_MEMORY_NIOS_CPU_WRITABLE 1


/*
 * pio_input configuration
 *
 */

#define ALT_MODULE_CLASS_pio_input altera_avalon_pio
#define PIO_INPUT_BASE 0x10
#define PIO_INPUT_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_INPUT_BIT_MODIFYING_OUTPUT_REGISTER 0
#define PIO_INPUT_CAPTURE 0
#define PIO_INPUT_DATA_WIDTH 15
#define PIO_INPUT_DO_TEST_BENCH_WIRING 0
#define PIO_INPUT_DRIVEN_SIM_VALUE 0
#define PIO_INPUT_EDGE_TYPE "NONE"
#define PIO_INPUT_FREQ 50000000
#define PIO_INPUT_HAS_IN 1
#define PIO_INPUT_HAS_OUT 0
#define PIO_INPUT_HAS_TRI 0
#define PIO_INPUT_IRQ 2
#define PIO_INPUT_IRQ_INTERRUPT_CONTROLLER_ID 0
#define PIO_INPUT_IRQ_TYPE "LEVEL"
#define PIO_INPUT_NAME "/dev/pio_input"
#define PIO_INPUT_RESET_VALUE 0
#define PIO_INPUT_SPAN 32
#define PIO_INPUT_TYPE "altera_avalon_pio"


/*
 * pio_output configuration
 *
 */

#define ALT_MODULE_CLASS_pio_output altera_avalon_pio
#define PIO_OUTPUT_BASE 0x20
#define PIO_OUTPUT_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_OUTPUT_BIT_MODIFYING_OUTPUT_REGISTER 1
#define PIO_OUTPUT_CAPTURE 0
#define PIO_OUTPUT_DATA_WIDTH 16
#define PIO_OUTPUT_DO_TEST_BENCH_WIRING 0
#define PIO_OUTPUT_DRIVEN_SIM_VALUE 0
#define PIO_OUTPUT_EDGE_TYPE "NONE"
#define PIO_OUTPUT_FREQ 50000000
#define PIO_OUTPUT_HAS_IN 0
#define PIO_OUTPUT_HAS_OUT 1
#define PIO_OUTPUT_HAS_TRI 0
#define PIO_OUTPUT_IRQ -1
#define PIO_OUTPUT_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PIO_OUTPUT_IRQ_TYPE "NONE"
#define PIO_OUTPUT_NAME "/dev/pio_output"
#define PIO_OUTPUT_RESET_VALUE 0
#define PIO_OUTPUT_SPAN 64
#define PIO_OUTPUT_TYPE "altera_avalon_pio"


/*
 * pio_watchdog_cnt configuration
 *
 */

#define ALT_MODULE_CLASS_pio_watchdog_cnt altera_avalon_pio
#define PIO_WATCHDOG_CNT_BASE 0x360
#define PIO_WATCHDOG_CNT_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_WATCHDOG_CNT_BIT_MODIFYING_OUTPUT_REGISTER 0
#define PIO_WATCHDOG_CNT_CAPTURE 0
#define PIO_WATCHDOG_CNT_DATA_WIDTH 32
#define PIO_WATCHDOG_CNT_DO_TEST_BENCH_WIRING 0
#define PIO_WATCHDOG_CNT_DRIVEN_SIM_VALUE 0
#define PIO_WATCHDOG_CNT_EDGE_TYPE "NONE"
#define PIO_WATCHDOG_CNT_FREQ 50000000
#define PIO_WATCHDOG_CNT_HAS_IN 0
#define PIO_WATCHDOG_CNT_HAS_OUT 1
#define PIO_WATCHDOG_CNT_HAS_TRI 0
#define PIO_WATCHDOG_CNT_IRQ -1
#define PIO_WATCHDOG_CNT_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PIO_WATCHDOG_CNT_IRQ_TYPE "NONE"
#define PIO_WATCHDOG_CNT_NAME "/dev/pio_watchdog_cnt"
#define PIO_WATCHDOG_CNT_RESET_VALUE 0
#define PIO_WATCHDOG_CNT_SPAN 32
#define PIO_WATCHDOG_CNT_TYPE "altera_avalon_pio"


/*
 * pio_watchdog_freq configuration
 *
 */

#define ALT_MODULE_CLASS_pio_watchdog_freq altera_avalon_pio
#define PIO_WATCHDOG_FREQ_BASE 0x370
#define PIO_WATCHDOG_FREQ_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_WATCHDOG_FREQ_BIT_MODIFYING_OUTPUT_REGISTER 0
#define PIO_WATCHDOG_FREQ_CAPTURE 0
#define PIO_WATCHDOG_FREQ_DATA_WIDTH 32
#define PIO_WATCHDOG_FREQ_DO_TEST_BENCH_WIRING 0
#define PIO_WATCHDOG_FREQ_DRIVEN_SIM_VALUE 0
#define PIO_WATCHDOG_FREQ_EDGE_TYPE "NONE"
#define PIO_WATCHDOG_FREQ_FREQ 50000000
#define PIO_WATCHDOG_FREQ_HAS_IN 0
#define PIO_WATCHDOG_FREQ_HAS_OUT 1
#define PIO_WATCHDOG_FREQ_HAS_TRI 0
#define PIO_WATCHDOG_FREQ_IRQ -1
#define PIO_WATCHDOG_FREQ_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PIO_WATCHDOG_FREQ_IRQ_TYPE "NONE"
#define PIO_WATCHDOG_FREQ_NAME "/dev/pio_watchdog_freq"
#define PIO_WATCHDOG_FREQ_RESET_VALUE 0
#define PIO_WATCHDOG_FREQ_SPAN 32
#define PIO_WATCHDOG_FREQ_TYPE "altera_avalon_pio"


/*
 * sysid configuration
 *
 */

#define ALT_MODULE_CLASS_sysid altera_avalon_sysid_qsys
#define SYSID_BASE 0x8
#define SYSID_ID 6
#define SYSID_IRQ -1
#define SYSID_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SYSID_NAME "/dev/sysid"
#define SYSID_SPAN 8
#define SYSID_TIMESTAMP 1535028175
#define SYSID_TYPE "altera_avalon_sysid_qsys"


/*
 * timer_0 configuration
 *
 */

#define ALT_MODULE_CLASS_timer_0 altera_avalon_timer
#define TIMER_0_ALWAYS_RUN 0
#define TIMER_0_BASE 0x60
#define TIMER_0_COUNTER_SIZE 32
#define TIMER_0_FIXED_PERIOD 0
#define TIMER_0_FREQ 50000000
#define TIMER_0_IRQ 0
#define TIMER_0_IRQ_INTERRUPT_CONTROLLER_ID 0
#define TIMER_0_LOAD_VALUE 49999
#define TIMER_0_MULT 0.001
#define TIMER_0_NAME "/dev/timer_0"
#define TIMER_0_PERIOD 1
#define TIMER_0_PERIOD_UNITS "ms"
#define TIMER_0_RESET_OUTPUT 0
#define TIMER_0_SNAPSHOT 1
#define TIMER_0_SPAN 64
#define TIMER_0_TICKS_PER_SEC 1000
#define TIMER_0_TIMEOUT_PULSE_OUTPUT 0
#define TIMER_0_TYPE "altera_avalon_timer"


/*
 * timer_1 configuration
 *
 */

#define ALT_MODULE_CLASS_timer_1 altera_avalon_timer
#define TIMER_1_ALWAYS_RUN 0
#define TIMER_1_BASE 0x40
#define TIMER_1_COUNTER_SIZE 32
#define TIMER_1_FIXED_PERIOD 0
#define TIMER_1_FREQ 50000000
#define TIMER_1_IRQ 1
#define TIMER_1_IRQ_INTERRUPT_CONTROLLER_ID 0
#define TIMER_1_LOAD_VALUE 49999
#define TIMER_1_MULT 0.001
#define TIMER_1_NAME "/dev/timer_1"
#define TIMER_1_PERIOD 1
#define TIMER_1_PERIOD_UNITS "ms"
#define TIMER_1_RESET_OUTPUT 0
#define TIMER_1_SNAPSHOT 1
#define TIMER_1_SPAN 64
#define TIMER_1_TICKS_PER_SEC 1000
#define TIMER_1_TIMEOUT_PULSE_OUTPUT 0
#define TIMER_1_TYPE "altera_avalon_timer"


/*
 * timer_2 configuration
 *
 */

#define ALT_MODULE_CLASS_timer_2 altera_avalon_timer
#define TIMER_2_ALWAYS_RUN 0
#define TIMER_2_BASE 0xe0
#define TIMER_2_COUNTER_SIZE 32
#define TIMER_2_FIXED_PERIOD 0
#define TIMER_2_FREQ 50000000
#define TIMER_2_IRQ 11
#define TIMER_2_IRQ_INTERRUPT_CONTROLLER_ID 0
#define TIMER_2_LOAD_VALUE 49999
#define TIMER_2_MULT 0.001
#define TIMER_2_NAME "/dev/timer_2"
#define TIMER_2_PERIOD 1
#define TIMER_2_PERIOD_UNITS "ms"
#define TIMER_2_RESET_OUTPUT 0
#define TIMER_2_SNAPSHOT 1
#define TIMER_2_SPAN 64
#define TIMER_2_TICKS_PER_SEC 1000
#define TIMER_2_TIMEOUT_PULSE_OUTPUT 0
#define TIMER_2_TYPE "altera_avalon_timer"

#endif /* __SYSTEM_H_ */
