/*
 * system.h - SOPC Builder system and BSP software package information
 *
 * Machine generated for CPU 'nios_cpu' in SOPC Builder design 'soc_system'
 * SOPC Builder design path: ../../soc_system.sopcinfo
 *
 * Generated: Tue Jan 28 14:58:16 CET 2020
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
 * AMC7891_0 configuration
 *
 */

#define ALT_MODULE_CLASS_AMC7891_0 AMC7891
#define AMC7891_0_BASE 0x10100
#define AMC7891_0_IRQ -1
#define AMC7891_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AMC7891_0_NAME "/dev/AMC7891_0"
#define AMC7891_0_SPAN 256
#define AMC7891_0_TYPE "AMC7891"


/*
 * CPU configuration
 *
 */

#define ALT_CPU_ARCHITECTURE "altera_nios2_gen2"
#define ALT_CPU_BIG_ENDIAN 0
#define ALT_CPU_BREAK_ADDR 0x00011820
#define ALT_CPU_CPU_ARCH_NIOS2_R1
#define ALT_CPU_CPU_FREQ 50000000u
#define ALT_CPU_CPU_ID_SIZE 1
#define ALT_CPU_CPU_ID_VALUE 0x00000000
#define ALT_CPU_CPU_IMPLEMENTATION "fast"
#define ALT_CPU_DATA_ADDR_WIDTH 0x13
#define ALT_CPU_DCACHE_LINE_SIZE 0
#define ALT_CPU_DCACHE_LINE_SIZE_LOG2 0
#define ALT_CPU_DCACHE_SIZE 0
#define ALT_CPU_EXCEPTION_ADDR 0x00000020
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
#define ALT_CPU_INST_ADDR_WIDTH 0x12
#define ALT_CPU_NAME "nios_cpu"
#define ALT_CPU_NUM_OF_SHADOW_REG_SETS 0
#define ALT_CPU_OCI_VERSION 1
#define ALT_CPU_RESET_ADDR 0x00000000


/*
 * CPU configuration (with legacy prefix - don't use these anymore)
 *
 */

#define NIOS2_BIG_ENDIAN 0
#define NIOS2_BREAK_ADDR 0x00011820
#define NIOS2_CPU_ARCH_NIOS2_R1
#define NIOS2_CPU_FREQ 50000000u
#define NIOS2_CPU_ID_SIZE 1
#define NIOS2_CPU_ID_VALUE 0x00000000
#define NIOS2_CPU_IMPLEMENTATION "fast"
#define NIOS2_DATA_ADDR_WIDTH 0x13
#define NIOS2_DCACHE_LINE_SIZE 0
#define NIOS2_DCACHE_LINE_SIZE_LOG2 0
#define NIOS2_DCACHE_SIZE 0
#define NIOS2_EXCEPTION_ADDR 0x00000020
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
#define NIOS2_INST_ADDR_WIDTH 0x12
#define NIOS2_NUM_OF_SHADOW_REG_SETS 0
#define NIOS2_OCI_VERSION 1
#define NIOS2_RESET_ADDR 0x00000000


/*
 * Define for each module class mastered by the CPU
 *
 */

#define __ALTERA_AVALON_JTAG_UART
#define __ALTERA_AVALON_ONCHIP_MEMORY2
#define __ALTERA_AVALON_PIO
#define __ALTERA_AVALON_SYSID_QSYS
#define __ALTERA_AVALON_TIMER
#define __ALTERA_AVALON_UART
#define __ALTERA_NIOS2_GEN2
#define __AMC7891
#define __AVALON_SPI_AD9102
#define __AVALON_TSE_MAC
#define __I2C_MASTER


/*
 * I2C_Master_0 configuration
 *
 */

#define ALT_MODULE_CLASS_I2C_Master_0 i2c_master
#define I2C_MASTER_0_BASE 0x10210
#define I2C_MASTER_0_IRQ -1
#define I2C_MASTER_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define I2C_MASTER_0_NAME "/dev/I2C_Master_0"
#define I2C_MASTER_0_SPAN 8
#define I2C_MASTER_0_TYPE "i2c_master"


/*
 * I2C_Master_1 configuration
 *
 */

#define ALT_MODULE_CLASS_I2C_Master_1 i2c_master
#define I2C_MASTER_1_BASE 0x10008
#define I2C_MASTER_1_IRQ -1
#define I2C_MASTER_1_IRQ_INTERRUPT_CONTROLLER_ID -1
#define I2C_MASTER_1_NAME "/dev/I2C_Master_1"
#define I2C_MASTER_1_SPAN 8
#define I2C_MASTER_1_TYPE "i2c_master"


/*
 * System configuration
 *
 */

#define ALT_DEVICE_FAMILY "Arria V"
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
#define ALT_STDERR_BASE 0x20000
#define ALT_STDERR_DEV jtag_uart
#define ALT_STDERR_IS_JTAG_UART
#define ALT_STDERR_PRESENT
#define ALT_STDERR_TYPE "altera_avalon_jtag_uart"
#define ALT_STDIN "/dev/jtag_uart"
#define ALT_STDIN_BASE 0x20000
#define ALT_STDIN_DEV jtag_uart
#define ALT_STDIN_IS_JTAG_UART
#define ALT_STDIN_PRESENT
#define ALT_STDIN_TYPE "altera_avalon_jtag_uart"
#define ALT_STDOUT "/dev/jtag_uart"
#define ALT_STDOUT_BASE 0x20000
#define ALT_STDOUT_DEV jtag_uart
#define ALT_STDOUT_IS_JTAG_UART
#define ALT_STDOUT_PRESENT
#define ALT_STDOUT_TYPE "altera_avalon_jtag_uart"
#define ALT_SYSTEM_NAME "soc_system"


/*
 * avalon_spi_AD9102_0 configuration
 *
 */

#define ALT_MODULE_CLASS_avalon_spi_AD9102_0 avalon_spi_AD9102
#define AVALON_SPI_AD9102_0_BASE 0x70000
#define AVALON_SPI_AD9102_0_IRQ -1
#define AVALON_SPI_AD9102_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AVALON_SPI_AD9102_0_NAME "/dev/avalon_spi_AD9102_0"
#define AVALON_SPI_AD9102_0_SPAN 65536
#define AVALON_SPI_AD9102_0_TYPE "avalon_spi_AD9102"


/*
 * avalon_spi_AD9102_1 configuration
 *
 */

#define ALT_MODULE_CLASS_avalon_spi_AD9102_1 avalon_spi_AD9102
#define AVALON_SPI_AD9102_1_BASE 0x50000
#define AVALON_SPI_AD9102_1_IRQ -1
#define AVALON_SPI_AD9102_1_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AVALON_SPI_AD9102_1_NAME "/dev/avalon_spi_AD9102_1"
#define AVALON_SPI_AD9102_1_SPAN 65536
#define AVALON_SPI_AD9102_1_TYPE "avalon_spi_AD9102"


/*
 * avalon_spi_AD9102_2 configuration
 *
 */

#define ALT_MODULE_CLASS_avalon_spi_AD9102_2 avalon_spi_AD9102
#define AVALON_SPI_AD9102_2_BASE 0x40000
#define AVALON_SPI_AD9102_2_IRQ -1
#define AVALON_SPI_AD9102_2_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AVALON_SPI_AD9102_2_NAME "/dev/avalon_spi_AD9102_2"
#define AVALON_SPI_AD9102_2_SPAN 65536
#define AVALON_SPI_AD9102_2_TYPE "avalon_spi_AD9102"


/*
 * avalon_spi_AD9102_3 configuration
 *
 */

#define ALT_MODULE_CLASS_avalon_spi_AD9102_3 avalon_spi_AD9102
#define AVALON_SPI_AD9102_3_BASE 0x30000
#define AVALON_SPI_AD9102_3_IRQ -1
#define AVALON_SPI_AD9102_3_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AVALON_SPI_AD9102_3_NAME "/dev/avalon_spi_AD9102_3"
#define AVALON_SPI_AD9102_3_SPAN 65536
#define AVALON_SPI_AD9102_3_TYPE "avalon_spi_AD9102"


/*
 * avalon_tse_mac_0 configuration
 *
 */

#define ALT_MODULE_CLASS_avalon_tse_mac_0 avalon_tse_mac
#define AVALON_TSE_MAC_0_BASE 0x10800
#define AVALON_TSE_MAC_0_IRQ -1
#define AVALON_TSE_MAC_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AVALON_TSE_MAC_0_NAME "/dev/avalon_tse_mac_0"
#define AVALON_TSE_MAC_0_SPAN 1024
#define AVALON_TSE_MAC_0_TYPE "avalon_tse_mac"


/*
 * avalon_tse_mac_1 configuration
 *
 */

#define ALT_MODULE_CLASS_avalon_tse_mac_1 avalon_tse_mac
#define AVALON_TSE_MAC_1_BASE 0x10400
#define AVALON_TSE_MAC_1_IRQ -1
#define AVALON_TSE_MAC_1_IRQ_INTERRUPT_CONTROLLER_ID -1
#define AVALON_TSE_MAC_1_NAME "/dev/avalon_tse_mac_1"
#define AVALON_TSE_MAC_1_SPAN 1024
#define AVALON_TSE_MAC_1_TYPE "avalon_tse_mac"


/*
 * cond_pio_0 configuration
 *
 */

#define ALT_MODULE_CLASS_cond_pio_0 altera_avalon_pio
#define COND_PIO_0_BASE 0x10280
#define COND_PIO_0_BIT_CLEARING_EDGE_REGISTER 0
#define COND_PIO_0_BIT_MODIFYING_OUTPUT_REGISTER 1
#define COND_PIO_0_CAPTURE 0
#define COND_PIO_0_DATA_WIDTH 4
#define COND_PIO_0_DO_TEST_BENCH_WIRING 0
#define COND_PIO_0_DRIVEN_SIM_VALUE 0
#define COND_PIO_0_EDGE_TYPE "NONE"
#define COND_PIO_0_FREQ 50000000
#define COND_PIO_0_HAS_IN 0
#define COND_PIO_0_HAS_OUT 1
#define COND_PIO_0_HAS_TRI 0
#define COND_PIO_0_IRQ -1
#define COND_PIO_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define COND_PIO_0_IRQ_TYPE "NONE"
#define COND_PIO_0_NAME "/dev/cond_pio_0"
#define COND_PIO_0_RESET_VALUE 1
#define COND_PIO_0_SPAN 32
#define COND_PIO_0_TYPE "altera_avalon_pio"


/*
 * cond_pio_1 configuration
 *
 */

#define ALT_MODULE_CLASS_cond_pio_1 altera_avalon_pio
#define COND_PIO_1_BASE 0x10260
#define COND_PIO_1_BIT_CLEARING_EDGE_REGISTER 0
#define COND_PIO_1_BIT_MODIFYING_OUTPUT_REGISTER 1
#define COND_PIO_1_CAPTURE 0
#define COND_PIO_1_DATA_WIDTH 4
#define COND_PIO_1_DO_TEST_BENCH_WIRING 0
#define COND_PIO_1_DRIVEN_SIM_VALUE 0
#define COND_PIO_1_EDGE_TYPE "NONE"
#define COND_PIO_1_FREQ 50000000
#define COND_PIO_1_HAS_IN 0
#define COND_PIO_1_HAS_OUT 1
#define COND_PIO_1_HAS_TRI 0
#define COND_PIO_1_IRQ -1
#define COND_PIO_1_IRQ_INTERRUPT_CONTROLLER_ID -1
#define COND_PIO_1_IRQ_TYPE "NONE"
#define COND_PIO_1_NAME "/dev/cond_pio_1"
#define COND_PIO_1_RESET_VALUE 1
#define COND_PIO_1_SPAN 32
#define COND_PIO_1_TYPE "altera_avalon_pio"


/*
 * cond_pio_2 configuration
 *
 */

#define ALT_MODULE_CLASS_cond_pio_2 altera_avalon_pio
#define COND_PIO_2_BASE 0x10240
#define COND_PIO_2_BIT_CLEARING_EDGE_REGISTER 0
#define COND_PIO_2_BIT_MODIFYING_OUTPUT_REGISTER 1
#define COND_PIO_2_CAPTURE 0
#define COND_PIO_2_DATA_WIDTH 4
#define COND_PIO_2_DO_TEST_BENCH_WIRING 0
#define COND_PIO_2_DRIVEN_SIM_VALUE 0
#define COND_PIO_2_EDGE_TYPE "NONE"
#define COND_PIO_2_FREQ 50000000
#define COND_PIO_2_HAS_IN 0
#define COND_PIO_2_HAS_OUT 1
#define COND_PIO_2_HAS_TRI 0
#define COND_PIO_2_IRQ -1
#define COND_PIO_2_IRQ_INTERRUPT_CONTROLLER_ID -1
#define COND_PIO_2_IRQ_TYPE "NONE"
#define COND_PIO_2_NAME "/dev/cond_pio_2"
#define COND_PIO_2_RESET_VALUE 1
#define COND_PIO_2_SPAN 32
#define COND_PIO_2_TYPE "altera_avalon_pio"


/*
 * cond_pio_3 configuration
 *
 */

#define ALT_MODULE_CLASS_cond_pio_3 altera_avalon_pio
#define COND_PIO_3_BASE 0x10220
#define COND_PIO_3_BIT_CLEARING_EDGE_REGISTER 0
#define COND_PIO_3_BIT_MODIFYING_OUTPUT_REGISTER 1
#define COND_PIO_3_CAPTURE 0
#define COND_PIO_3_DATA_WIDTH 4
#define COND_PIO_3_DO_TEST_BENCH_WIRING 0
#define COND_PIO_3_DRIVEN_SIM_VALUE 0
#define COND_PIO_3_EDGE_TYPE "NONE"
#define COND_PIO_3_FREQ 50000000
#define COND_PIO_3_HAS_IN 0
#define COND_PIO_3_HAS_OUT 1
#define COND_PIO_3_HAS_TRI 0
#define COND_PIO_3_IRQ -1
#define COND_PIO_3_IRQ_INTERRUPT_CONTROLLER_ID -1
#define COND_PIO_3_IRQ_TYPE "NONE"
#define COND_PIO_3_NAME "/dev/cond_pio_3"
#define COND_PIO_3_RESET_VALUE 1
#define COND_PIO_3_SPAN 32
#define COND_PIO_3_TYPE "altera_avalon_pio"


/*
 * cpu_timer_0 configuration
 *
 */

#define ALT_MODULE_CLASS_cpu_timer_0 altera_avalon_timer
#define CPU_TIMER_0_ALWAYS_RUN 0
#define CPU_TIMER_0_BASE 0x10080
#define CPU_TIMER_0_COUNTER_SIZE 32
#define CPU_TIMER_0_FIXED_PERIOD 0
#define CPU_TIMER_0_FREQ 50000000
#define CPU_TIMER_0_IRQ 0
#define CPU_TIMER_0_IRQ_INTERRUPT_CONTROLLER_ID 0
#define CPU_TIMER_0_LOAD_VALUE 49999
#define CPU_TIMER_0_MULT 0.001
#define CPU_TIMER_0_NAME "/dev/cpu_timer_0"
#define CPU_TIMER_0_PERIOD 1
#define CPU_TIMER_0_PERIOD_UNITS "ms"
#define CPU_TIMER_0_RESET_OUTPUT 0
#define CPU_TIMER_0_SNAPSHOT 1
#define CPU_TIMER_0_SPAN 32
#define CPU_TIMER_0_TICKS_PER_SEC 1000
#define CPU_TIMER_0_TIMEOUT_PULSE_OUTPUT 0
#define CPU_TIMER_0_TYPE "altera_avalon_timer"


/*
 * cpu_timer_1 configuration
 *
 */

#define ALT_MODULE_CLASS_cpu_timer_1 altera_avalon_timer
#define CPU_TIMER_1_ALWAYS_RUN 0
#define CPU_TIMER_1_BASE 0x10040
#define CPU_TIMER_1_COUNTER_SIZE 32
#define CPU_TIMER_1_FIXED_PERIOD 0
#define CPU_TIMER_1_FREQ 50000000
#define CPU_TIMER_1_IRQ 1
#define CPU_TIMER_1_IRQ_INTERRUPT_CONTROLLER_ID 0
#define CPU_TIMER_1_LOAD_VALUE 49999
#define CPU_TIMER_1_MULT 0.001
#define CPU_TIMER_1_NAME "/dev/cpu_timer_1"
#define CPU_TIMER_1_PERIOD 1
#define CPU_TIMER_1_PERIOD_UNITS "ms"
#define CPU_TIMER_1_RESET_OUTPUT 0
#define CPU_TIMER_1_SNAPSHOT 1
#define CPU_TIMER_1_SPAN 32
#define CPU_TIMER_1_TICKS_PER_SEC 1000
#define CPU_TIMER_1_TIMEOUT_PULSE_OUTPUT 0
#define CPU_TIMER_1_TYPE "altera_avalon_timer"


/*
 * cpu_timer_2 configuration
 *
 */

#define ALT_MODULE_CLASS_cpu_timer_2 altera_avalon_timer
#define CPU_TIMER_2_ALWAYS_RUN 0
#define CPU_TIMER_2_BASE 0x10020
#define CPU_TIMER_2_COUNTER_SIZE 32
#define CPU_TIMER_2_FIXED_PERIOD 0
#define CPU_TIMER_2_FREQ 50000000
#define CPU_TIMER_2_IRQ 2
#define CPU_TIMER_2_IRQ_INTERRUPT_CONTROLLER_ID 0
#define CPU_TIMER_2_LOAD_VALUE 49999
#define CPU_TIMER_2_MULT 0.001
#define CPU_TIMER_2_NAME "/dev/cpu_timer_2"
#define CPU_TIMER_2_PERIOD 1
#define CPU_TIMER_2_PERIOD_UNITS "ms"
#define CPU_TIMER_2_RESET_OUTPUT 0
#define CPU_TIMER_2_SNAPSHOT 1
#define CPU_TIMER_2_SPAN 32
#define CPU_TIMER_2_TICKS_PER_SEC 1000
#define CPU_TIMER_2_TIMEOUT_PULSE_OUTPUT 0
#define CPU_TIMER_2_TYPE "altera_avalon_timer"


/*
 * dp_ram_nios_arm configuration
 *
 */

#define ALT_MODULE_CLASS_dp_ram_nios_arm altera_avalon_onchip_memory2
#define DP_RAM_NIOS_ARM_ALLOW_IN_SYSTEM_MEMORY_CONTENT_EDITOR 0
#define DP_RAM_NIOS_ARM_ALLOW_MRAM_SIM_CONTENTS_ONLY_FILE 0
#define DP_RAM_NIOS_ARM_BASE 0x60000
#define DP_RAM_NIOS_ARM_CONTENTS_INFO ""
#define DP_RAM_NIOS_ARM_DUAL_PORT 1
#define DP_RAM_NIOS_ARM_GUI_RAM_BLOCK_TYPE "AUTO"
#define DP_RAM_NIOS_ARM_INIT_CONTENTS_FILE "soc_system_dp_ram_nios_arm"
#define DP_RAM_NIOS_ARM_INIT_MEM_CONTENT 1
#define DP_RAM_NIOS_ARM_INSTANCE_ID "NONE"
#define DP_RAM_NIOS_ARM_IRQ -1
#define DP_RAM_NIOS_ARM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define DP_RAM_NIOS_ARM_NAME "/dev/dp_ram_nios_arm"
#define DP_RAM_NIOS_ARM_NON_DEFAULT_INIT_FILE_ENABLED 0
#define DP_RAM_NIOS_ARM_RAM_BLOCK_TYPE "AUTO"
#define DP_RAM_NIOS_ARM_READ_DURING_WRITE_MODE "DONT_CARE"
#define DP_RAM_NIOS_ARM_SINGLE_CLOCK_OP 1
#define DP_RAM_NIOS_ARM_SIZE_MULTIPLE 1
#define DP_RAM_NIOS_ARM_SIZE_VALUE 32768
#define DP_RAM_NIOS_ARM_SPAN 32768
#define DP_RAM_NIOS_ARM_TYPE "altera_avalon_onchip_memory2"
#define DP_RAM_NIOS_ARM_WRITABLE 1


/*
 * firepuls_ac_err_in configuration
 *
 */

#define ALT_MODULE_CLASS_firepuls_ac_err_in altera_avalon_pio
#define FIREPULS_AC_ERR_IN_BASE 0x10330
#define FIREPULS_AC_ERR_IN_BIT_CLEARING_EDGE_REGISTER 0
#define FIREPULS_AC_ERR_IN_BIT_MODIFYING_OUTPUT_REGISTER 0
#define FIREPULS_AC_ERR_IN_CAPTURE 0
#define FIREPULS_AC_ERR_IN_DATA_WIDTH 4
#define FIREPULS_AC_ERR_IN_DO_TEST_BENCH_WIRING 0
#define FIREPULS_AC_ERR_IN_DRIVEN_SIM_VALUE 0
#define FIREPULS_AC_ERR_IN_EDGE_TYPE "NONE"
#define FIREPULS_AC_ERR_IN_FREQ 50000000
#define FIREPULS_AC_ERR_IN_HAS_IN 1
#define FIREPULS_AC_ERR_IN_HAS_OUT 0
#define FIREPULS_AC_ERR_IN_HAS_TRI 0
#define FIREPULS_AC_ERR_IN_IRQ -1
#define FIREPULS_AC_ERR_IN_IRQ_INTERRUPT_CONTROLLER_ID -1
#define FIREPULS_AC_ERR_IN_IRQ_TYPE "NONE"
#define FIREPULS_AC_ERR_IN_NAME "/dev/firepuls_ac_err_in"
#define FIREPULS_AC_ERR_IN_RESET_VALUE 0
#define FIREPULS_AC_ERR_IN_SPAN 16
#define FIREPULS_AC_ERR_IN_TYPE "altera_avalon_pio"


/*
 * firepuls_dc_err_in configuration
 *
 */

#define ALT_MODULE_CLASS_firepuls_dc_err_in altera_avalon_pio
#define FIREPULS_DC_ERR_IN_BASE 0x10340
#define FIREPULS_DC_ERR_IN_BIT_CLEARING_EDGE_REGISTER 0
#define FIREPULS_DC_ERR_IN_BIT_MODIFYING_OUTPUT_REGISTER 0
#define FIREPULS_DC_ERR_IN_CAPTURE 0
#define FIREPULS_DC_ERR_IN_DATA_WIDTH 4
#define FIREPULS_DC_ERR_IN_DO_TEST_BENCH_WIRING 0
#define FIREPULS_DC_ERR_IN_DRIVEN_SIM_VALUE 0
#define FIREPULS_DC_ERR_IN_EDGE_TYPE "NONE"
#define FIREPULS_DC_ERR_IN_FREQ 50000000
#define FIREPULS_DC_ERR_IN_HAS_IN 1
#define FIREPULS_DC_ERR_IN_HAS_OUT 0
#define FIREPULS_DC_ERR_IN_HAS_TRI 0
#define FIREPULS_DC_ERR_IN_IRQ -1
#define FIREPULS_DC_ERR_IN_IRQ_INTERRUPT_CONTROLLER_ID -1
#define FIREPULS_DC_ERR_IN_IRQ_TYPE "NONE"
#define FIREPULS_DC_ERR_IN_NAME "/dev/firepuls_dc_err_in"
#define FIREPULS_DC_ERR_IN_RESET_VALUE 0
#define FIREPULS_DC_ERR_IN_SPAN 16
#define FIREPULS_DC_ERR_IN_TYPE "altera_avalon_pio"


/*
 * firepuls_gain_out configuration
 *
 */

#define ALT_MODULE_CLASS_firepuls_gain_out altera_avalon_pio
#define FIREPULS_GAIN_OUT_BASE 0x10320
#define FIREPULS_GAIN_OUT_BIT_CLEARING_EDGE_REGISTER 0
#define FIREPULS_GAIN_OUT_BIT_MODIFYING_OUTPUT_REGISTER 0
#define FIREPULS_GAIN_OUT_CAPTURE 0
#define FIREPULS_GAIN_OUT_DATA_WIDTH 4
#define FIREPULS_GAIN_OUT_DO_TEST_BENCH_WIRING 0
#define FIREPULS_GAIN_OUT_DRIVEN_SIM_VALUE 0
#define FIREPULS_GAIN_OUT_EDGE_TYPE "NONE"
#define FIREPULS_GAIN_OUT_FREQ 50000000
#define FIREPULS_GAIN_OUT_HAS_IN 0
#define FIREPULS_GAIN_OUT_HAS_OUT 1
#define FIREPULS_GAIN_OUT_HAS_TRI 0
#define FIREPULS_GAIN_OUT_IRQ -1
#define FIREPULS_GAIN_OUT_IRQ_INTERRUPT_CONTROLLER_ID -1
#define FIREPULS_GAIN_OUT_IRQ_TYPE "NONE"
#define FIREPULS_GAIN_OUT_NAME "/dev/firepuls_gain_out"
#define FIREPULS_GAIN_OUT_RESET_VALUE 0
#define FIREPULS_GAIN_OUT_SPAN 16
#define FIREPULS_GAIN_OUT_TYPE "altera_avalon_pio"


/*
 * hal configuration
 *
 */

#define ALT_MAX_FD 32
#define ALT_SYS_CLK CPU_TIMER_0
#define ALT_TIMESTAMP_CLK none


/*
 * head_con_err_in configuration
 *
 */

#define ALT_MODULE_CLASS_head_con_err_in altera_avalon_pio
#define HEAD_CON_ERR_IN_BASE 0x10350
#define HEAD_CON_ERR_IN_BIT_CLEARING_EDGE_REGISTER 0
#define HEAD_CON_ERR_IN_BIT_MODIFYING_OUTPUT_REGISTER 0
#define HEAD_CON_ERR_IN_CAPTURE 0
#define HEAD_CON_ERR_IN_DATA_WIDTH 4
#define HEAD_CON_ERR_IN_DO_TEST_BENCH_WIRING 0
#define HEAD_CON_ERR_IN_DRIVEN_SIM_VALUE 0
#define HEAD_CON_ERR_IN_EDGE_TYPE "NONE"
#define HEAD_CON_ERR_IN_FREQ 50000000
#define HEAD_CON_ERR_IN_HAS_IN 1
#define HEAD_CON_ERR_IN_HAS_OUT 0
#define HEAD_CON_ERR_IN_HAS_TRI 0
#define HEAD_CON_ERR_IN_IRQ -1
#define HEAD_CON_ERR_IN_IRQ_INTERRUPT_CONTROLLER_ID -1
#define HEAD_CON_ERR_IN_IRQ_TYPE "NONE"
#define HEAD_CON_ERR_IN_NAME "/dev/head_con_err_in"
#define HEAD_CON_ERR_IN_RESET_VALUE 0
#define HEAD_CON_ERR_IN_SPAN 16
#define HEAD_CON_ERR_IN_TYPE "altera_avalon_pio"


/*
 * jtag_uart configuration
 *
 */

#define ALT_MODULE_CLASS_jtag_uart altera_avalon_jtag_uart
#define JTAG_UART_BASE 0x20000
#define JTAG_UART_IRQ 7
#define JTAG_UART_IRQ_INTERRUPT_CONTROLLER_ID 0
#define JTAG_UART_NAME "/dev/jtag_uart"
#define JTAG_UART_READ_DEPTH 64
#define JTAG_UART_READ_THRESHOLD 8
#define JTAG_UART_SPAN 8
#define JTAG_UART_TYPE "altera_avalon_jtag_uart"
#define JTAG_UART_WRITE_DEPTH 64
#define JTAG_UART_WRITE_THRESHOLD 8


/*
 * mac0_high configuration
 *
 */

#define ALT_MODULE_CLASS_mac0_high altera_avalon_pio
#define MAC0_HIGH_BASE 0x100f0
#define MAC0_HIGH_BIT_CLEARING_EDGE_REGISTER 0
#define MAC0_HIGH_BIT_MODIFYING_OUTPUT_REGISTER 0
#define MAC0_HIGH_CAPTURE 0
#define MAC0_HIGH_DATA_WIDTH 16
#define MAC0_HIGH_DO_TEST_BENCH_WIRING 0
#define MAC0_HIGH_DRIVEN_SIM_VALUE 0
#define MAC0_HIGH_EDGE_TYPE "NONE"
#define MAC0_HIGH_FREQ 50000000
#define MAC0_HIGH_HAS_IN 1
#define MAC0_HIGH_HAS_OUT 0
#define MAC0_HIGH_HAS_TRI 0
#define MAC0_HIGH_IRQ -1
#define MAC0_HIGH_IRQ_INTERRUPT_CONTROLLER_ID -1
#define MAC0_HIGH_IRQ_TYPE "NONE"
#define MAC0_HIGH_NAME "/dev/mac0_high"
#define MAC0_HIGH_RESET_VALUE 0
#define MAC0_HIGH_SPAN 16
#define MAC0_HIGH_TYPE "altera_avalon_pio"


/*
 * mac0_low configuration
 *
 */

#define ALT_MODULE_CLASS_mac0_low altera_avalon_pio
#define MAC0_LOW_BASE 0x10200
#define MAC0_LOW_BIT_CLEARING_EDGE_REGISTER 0
#define MAC0_LOW_BIT_MODIFYING_OUTPUT_REGISTER 0
#define MAC0_LOW_CAPTURE 0
#define MAC0_LOW_DATA_WIDTH 32
#define MAC0_LOW_DO_TEST_BENCH_WIRING 0
#define MAC0_LOW_DRIVEN_SIM_VALUE 0
#define MAC0_LOW_EDGE_TYPE "NONE"
#define MAC0_LOW_FREQ 50000000
#define MAC0_LOW_HAS_IN 1
#define MAC0_LOW_HAS_OUT 0
#define MAC0_LOW_HAS_TRI 0
#define MAC0_LOW_IRQ -1
#define MAC0_LOW_IRQ_INTERRUPT_CONTROLLER_ID -1
#define MAC0_LOW_IRQ_TYPE "NONE"
#define MAC0_LOW_NAME "/dev/mac0_low"
#define MAC0_LOW_RESET_VALUE 0
#define MAC0_LOW_SPAN 16
#define MAC0_LOW_TYPE "altera_avalon_pio"


/*
 * mac1_high configuration
 *
 */

#define ALT_MODULE_CLASS_mac1_high altera_avalon_pio
#define MAC1_HIGH_BASE 0x10010
#define MAC1_HIGH_BIT_CLEARING_EDGE_REGISTER 0
#define MAC1_HIGH_BIT_MODIFYING_OUTPUT_REGISTER 0
#define MAC1_HIGH_CAPTURE 0
#define MAC1_HIGH_DATA_WIDTH 16
#define MAC1_HIGH_DO_TEST_BENCH_WIRING 0
#define MAC1_HIGH_DRIVEN_SIM_VALUE 0
#define MAC1_HIGH_EDGE_TYPE "NONE"
#define MAC1_HIGH_FREQ 50000000
#define MAC1_HIGH_HAS_IN 1
#define MAC1_HIGH_HAS_OUT 0
#define MAC1_HIGH_HAS_TRI 0
#define MAC1_HIGH_IRQ -1
#define MAC1_HIGH_IRQ_INTERRUPT_CONTROLLER_ID -1
#define MAC1_HIGH_IRQ_TYPE "NONE"
#define MAC1_HIGH_NAME "/dev/mac1_high"
#define MAC1_HIGH_RESET_VALUE 0
#define MAC1_HIGH_SPAN 16
#define MAC1_HIGH_TYPE "altera_avalon_pio"


/*
 * mac1_low configuration
 *
 */

#define ALT_MODULE_CLASS_mac1_low altera_avalon_pio
#define MAC1_LOW_BASE 0x100e0
#define MAC1_LOW_BIT_CLEARING_EDGE_REGISTER 0
#define MAC1_LOW_BIT_MODIFYING_OUTPUT_REGISTER 0
#define MAC1_LOW_CAPTURE 0
#define MAC1_LOW_DATA_WIDTH 32
#define MAC1_LOW_DO_TEST_BENCH_WIRING 0
#define MAC1_LOW_DRIVEN_SIM_VALUE 0
#define MAC1_LOW_EDGE_TYPE "NONE"
#define MAC1_LOW_FREQ 50000000
#define MAC1_LOW_HAS_IN 1
#define MAC1_LOW_HAS_OUT 0
#define MAC1_LOW_HAS_TRI 0
#define MAC1_LOW_IRQ -1
#define MAC1_LOW_IRQ_INTERRUPT_CONTROLLER_ID -1
#define MAC1_LOW_IRQ_TYPE "NONE"
#define MAC1_LOW_NAME "/dev/mac1_low"
#define MAC1_LOW_RESET_VALUE 0
#define MAC1_LOW_SPAN 16
#define MAC1_LOW_TYPE "altera_avalon_pio"


/*
 * onchip_memory_nios_cpu configuration
 *
 */

#define ALT_MODULE_CLASS_onchip_memory_nios_cpu altera_avalon_onchip_memory2
#define ONCHIP_MEMORY_NIOS_CPU_ALLOW_IN_SYSTEM_MEMORY_CONTENT_EDITOR 0
#define ONCHIP_MEMORY_NIOS_CPU_ALLOW_MRAM_SIM_CONTENTS_ONLY_FILE 0
#define ONCHIP_MEMORY_NIOS_CPU_BASE 0x0
#define ONCHIP_MEMORY_NIOS_CPU_CONTENTS_INFO ""
#define ONCHIP_MEMORY_NIOS_CPU_DUAL_PORT 1
#define ONCHIP_MEMORY_NIOS_CPU_GUI_RAM_BLOCK_TYPE "AUTO"
#define ONCHIP_MEMORY_NIOS_CPU_INIT_CONTENTS_FILE "soc_system_onchip_memory_nios_cpu"
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
 * pio_all_on_en configuration
 *
 */

#define ALT_MODULE_CLASS_pio_all_on_en altera_avalon_pio
#define PIO_ALL_ON_EN_BASE 0x10360
#define PIO_ALL_ON_EN_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_ALL_ON_EN_BIT_MODIFYING_OUTPUT_REGISTER 0
#define PIO_ALL_ON_EN_CAPTURE 0
#define PIO_ALL_ON_EN_DATA_WIDTH 1
#define PIO_ALL_ON_EN_DO_TEST_BENCH_WIRING 0
#define PIO_ALL_ON_EN_DRIVEN_SIM_VALUE 0
#define PIO_ALL_ON_EN_EDGE_TYPE "NONE"
#define PIO_ALL_ON_EN_FREQ 50000000
#define PIO_ALL_ON_EN_HAS_IN 0
#define PIO_ALL_ON_EN_HAS_OUT 1
#define PIO_ALL_ON_EN_HAS_TRI 0
#define PIO_ALL_ON_EN_IRQ -1
#define PIO_ALL_ON_EN_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PIO_ALL_ON_EN_IRQ_TYPE "NONE"
#define PIO_ALL_ON_EN_NAME "/dev/pio_all_on_en"
#define PIO_ALL_ON_EN_RESET_VALUE 0
#define PIO_ALL_ON_EN_SPAN 16
#define PIO_ALL_ON_EN_TYPE "altera_avalon_pio"


/*
 * pio_enc_msg_shift configuration
 *
 */

#define ALT_MODULE_CLASS_pio_enc_msg_shift altera_avalon_pio
#define PIO_ENC_MSG_SHIFT_BASE 0x10370
#define PIO_ENC_MSG_SHIFT_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_ENC_MSG_SHIFT_BIT_MODIFYING_OUTPUT_REGISTER 0
#define PIO_ENC_MSG_SHIFT_CAPTURE 0
#define PIO_ENC_MSG_SHIFT_DATA_WIDTH 2
#define PIO_ENC_MSG_SHIFT_DO_TEST_BENCH_WIRING 0
#define PIO_ENC_MSG_SHIFT_DRIVEN_SIM_VALUE 0
#define PIO_ENC_MSG_SHIFT_EDGE_TYPE "NONE"
#define PIO_ENC_MSG_SHIFT_FREQ 50000000
#define PIO_ENC_MSG_SHIFT_HAS_IN 0
#define PIO_ENC_MSG_SHIFT_HAS_OUT 1
#define PIO_ENC_MSG_SHIFT_HAS_TRI 0
#define PIO_ENC_MSG_SHIFT_IRQ -1
#define PIO_ENC_MSG_SHIFT_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PIO_ENC_MSG_SHIFT_IRQ_TYPE "NONE"
#define PIO_ENC_MSG_SHIFT_NAME "/dev/pio_enc_msg_shift"
#define PIO_ENC_MSG_SHIFT_RESET_VALUE 0
#define PIO_ENC_MSG_SHIFT_SPAN 16
#define PIO_ENC_MSG_SHIFT_TYPE "altera_avalon_pio"


/*
 * pio_fp_power configuration
 *
 */

#define ALT_MODULE_CLASS_pio_fp_power altera_avalon_pio
#define PIO_FP_POWER_BASE 0x10060
#define PIO_FP_POWER_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_FP_POWER_BIT_MODIFYING_OUTPUT_REGISTER 1
#define PIO_FP_POWER_CAPTURE 0
#define PIO_FP_POWER_DATA_WIDTH 1
#define PIO_FP_POWER_DO_TEST_BENCH_WIRING 0
#define PIO_FP_POWER_DRIVEN_SIM_VALUE 0
#define PIO_FP_POWER_EDGE_TYPE "NONE"
#define PIO_FP_POWER_FREQ 50000000
#define PIO_FP_POWER_HAS_IN 0
#define PIO_FP_POWER_HAS_OUT 1
#define PIO_FP_POWER_HAS_TRI 0
#define PIO_FP_POWER_IRQ -1
#define PIO_FP_POWER_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PIO_FP_POWER_IRQ_TYPE "NONE"
#define PIO_FP_POWER_NAME "/dev/pio_fp_power"
#define PIO_FP_POWER_RESET_VALUE 0
#define PIO_FP_POWER_SPAN 32
#define PIO_FP_POWER_TYPE "altera_avalon_pio"


/*
 * pio_fpga_shutdown configuration
 *
 */

#define ALT_MODULE_CLASS_pio_fpga_shutdown altera_avalon_pio
#define PIO_FPGA_SHUTDOWN_BASE 0x10390
#define PIO_FPGA_SHUTDOWN_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_FPGA_SHUTDOWN_BIT_MODIFYING_OUTPUT_REGISTER 0
#define PIO_FPGA_SHUTDOWN_CAPTURE 0
#define PIO_FPGA_SHUTDOWN_DATA_WIDTH 1
#define PIO_FPGA_SHUTDOWN_DO_TEST_BENCH_WIRING 0
#define PIO_FPGA_SHUTDOWN_DRIVEN_SIM_VALUE 0
#define PIO_FPGA_SHUTDOWN_EDGE_TYPE "NONE"
#define PIO_FPGA_SHUTDOWN_FREQ 50000000
#define PIO_FPGA_SHUTDOWN_HAS_IN 0
#define PIO_FPGA_SHUTDOWN_HAS_OUT 1
#define PIO_FPGA_SHUTDOWN_HAS_TRI 0
#define PIO_FPGA_SHUTDOWN_IRQ -1
#define PIO_FPGA_SHUTDOWN_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PIO_FPGA_SHUTDOWN_IRQ_TYPE "NONE"
#define PIO_FPGA_SHUTDOWN_NAME "/dev/pio_fpga_shutdown"
#define PIO_FPGA_SHUTDOWN_RESET_VALUE 0
#define PIO_FPGA_SHUTDOWN_SPAN 16
#define PIO_FPGA_SHUTDOWN_TYPE "altera_avalon_pio"


/*
 * pio_nios_led configuration
 *
 */

#define ALT_MODULE_CLASS_pio_nios_led altera_avalon_pio
#define PIO_NIOS_LED_BASE 0x100c0
#define PIO_NIOS_LED_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_NIOS_LED_BIT_MODIFYING_OUTPUT_REGISTER 1
#define PIO_NIOS_LED_CAPTURE 0
#define PIO_NIOS_LED_DATA_WIDTH 2
#define PIO_NIOS_LED_DO_TEST_BENCH_WIRING 0
#define PIO_NIOS_LED_DRIVEN_SIM_VALUE 0
#define PIO_NIOS_LED_EDGE_TYPE "NONE"
#define PIO_NIOS_LED_FREQ 50000000
#define PIO_NIOS_LED_HAS_IN 0
#define PIO_NIOS_LED_HAS_OUT 1
#define PIO_NIOS_LED_HAS_TRI 0
#define PIO_NIOS_LED_IRQ -1
#define PIO_NIOS_LED_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PIO_NIOS_LED_IRQ_TYPE "NONE"
#define PIO_NIOS_LED_NAME "/dev/pio_nios_led"
#define PIO_NIOS_LED_RESET_VALUE 0
#define PIO_NIOS_LED_SPAN 32
#define PIO_NIOS_LED_TYPE "altera_avalon_pio"


/*
 * pio_overheat configuration
 *
 */

#define ALT_MODULE_CLASS_pio_overheat altera_avalon_pio
#define PIO_OVERHEAT_BASE 0x10380
#define PIO_OVERHEAT_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_OVERHEAT_BIT_MODIFYING_OUTPUT_REGISTER 0
#define PIO_OVERHEAT_CAPTURE 1
#define PIO_OVERHEAT_DATA_WIDTH 1
#define PIO_OVERHEAT_DO_TEST_BENCH_WIRING 0
#define PIO_OVERHEAT_DRIVEN_SIM_VALUE 0
#define PIO_OVERHEAT_EDGE_TYPE "RISING"
#define PIO_OVERHEAT_FREQ 50000000
#define PIO_OVERHEAT_HAS_IN 1
#define PIO_OVERHEAT_HAS_OUT 0
#define PIO_OVERHEAT_HAS_TRI 0
#define PIO_OVERHEAT_IRQ -1
#define PIO_OVERHEAT_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PIO_OVERHEAT_IRQ_TYPE "NONE"
#define PIO_OVERHEAT_NAME "/dev/pio_overheat"
#define PIO_OVERHEAT_RESET_VALUE 0
#define PIO_OVERHEAT_SPAN 16
#define PIO_OVERHEAT_TYPE "altera_avalon_pio"


/*
 * pio_reset_dac configuration
 *
 */

#define ALT_MODULE_CLASS_pio_reset_dac altera_avalon_pio
#define PIO_RESET_DAC_BASE 0x100a0
#define PIO_RESET_DAC_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_RESET_DAC_BIT_MODIFYING_OUTPUT_REGISTER 1
#define PIO_RESET_DAC_CAPTURE 0
#define PIO_RESET_DAC_DATA_WIDTH 4
#define PIO_RESET_DAC_DO_TEST_BENCH_WIRING 0
#define PIO_RESET_DAC_DRIVEN_SIM_VALUE 0
#define PIO_RESET_DAC_EDGE_TYPE "NONE"
#define PIO_RESET_DAC_FREQ 50000000
#define PIO_RESET_DAC_HAS_IN 0
#define PIO_RESET_DAC_HAS_OUT 1
#define PIO_RESET_DAC_HAS_TRI 0
#define PIO_RESET_DAC_IRQ -1
#define PIO_RESET_DAC_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PIO_RESET_DAC_IRQ_TYPE "NONE"
#define PIO_RESET_DAC_NAME "/dev/pio_reset_dac"
#define PIO_RESET_DAC_RESET_VALUE 0
#define PIO_RESET_DAC_SPAN 32
#define PIO_RESET_DAC_TYPE "altera_avalon_pio"


/*
 * sysid_qsys configuration
 *
 */

#define ALT_MODULE_CLASS_sysid_qsys altera_avalon_sysid_qsys
#define SYSID_QSYS_BASE 0x10000
#define SYSID_QSYS_ID -1395322108
#define SYSID_QSYS_IRQ -1
#define SYSID_QSYS_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SYSID_QSYS_NAME "/dev/sysid_qsys"
#define SYSID_QSYS_SPAN 8
#define SYSID_QSYS_TIMESTAMP 1560758923
#define SYSID_QSYS_TYPE "altera_avalon_sysid_qsys"


/*
 * uart_0 configuration
 *
 */

#define ALT_MODULE_CLASS_uart_0 altera_avalon_uart
#define UART_0_BASE 0x10300
#define UART_0_BAUD 115200
#define UART_0_DATA_BITS 8
#define UART_0_FIXED_BAUD 0
#define UART_0_FREQ 50000000
#define UART_0_IRQ 3
#define UART_0_IRQ_INTERRUPT_CONTROLLER_ID 0
#define UART_0_NAME "/dev/uart_0"
#define UART_0_PARITY 'N'
#define UART_0_SIM_CHAR_STREAM ""
#define UART_0_SIM_TRUE_BAUD 0
#define UART_0_SPAN 32
#define UART_0_STOP_BITS 1
#define UART_0_SYNC_REG_DEPTH 2
#define UART_0_TYPE "altera_avalon_uart"
#define UART_0_USE_CTS_RTS 0
#define UART_0_USE_EOP_REGISTER 0


/*
 * uart_1 configuration
 *
 */

#define ALT_MODULE_CLASS_uart_1 altera_avalon_uart
#define UART_1_BASE 0x102e0
#define UART_1_BAUD 115200
#define UART_1_DATA_BITS 8
#define UART_1_FIXED_BAUD 0
#define UART_1_FREQ 50000000
#define UART_1_IRQ 4
#define UART_1_IRQ_INTERRUPT_CONTROLLER_ID 0
#define UART_1_NAME "/dev/uart_1"
#define UART_1_PARITY 'N'
#define UART_1_SIM_CHAR_STREAM ""
#define UART_1_SIM_TRUE_BAUD 0
#define UART_1_SPAN 32
#define UART_1_STOP_BITS 1
#define UART_1_SYNC_REG_DEPTH 2
#define UART_1_TYPE "altera_avalon_uart"
#define UART_1_USE_CTS_RTS 0
#define UART_1_USE_EOP_REGISTER 0


/*
 * uart_2 configuration
 *
 */

#define ALT_MODULE_CLASS_uart_2 altera_avalon_uart
#define UART_2_BASE 0x102c0
#define UART_2_BAUD 115200
#define UART_2_DATA_BITS 8
#define UART_2_FIXED_BAUD 0
#define UART_2_FREQ 50000000
#define UART_2_IRQ 5
#define UART_2_IRQ_INTERRUPT_CONTROLLER_ID 0
#define UART_2_NAME "/dev/uart_2"
#define UART_2_PARITY 'N'
#define UART_2_SIM_CHAR_STREAM ""
#define UART_2_SIM_TRUE_BAUD 0
#define UART_2_SPAN 32
#define UART_2_STOP_BITS 1
#define UART_2_SYNC_REG_DEPTH 2
#define UART_2_TYPE "altera_avalon_uart"
#define UART_2_USE_CTS_RTS 0
#define UART_2_USE_EOP_REGISTER 0


/*
 * uart_3 configuration
 *
 */

#define ALT_MODULE_CLASS_uart_3 altera_avalon_uart
#define UART_3_BASE 0x102a0
#define UART_3_BAUD 115200
#define UART_3_DATA_BITS 8
#define UART_3_FIXED_BAUD 0
#define UART_3_FREQ 50000000
#define UART_3_IRQ 6
#define UART_3_IRQ_INTERRUPT_CONTROLLER_ID 0
#define UART_3_NAME "/dev/uart_3"
#define UART_3_PARITY 'N'
#define UART_3_SIM_CHAR_STREAM ""
#define UART_3_SIM_TRUE_BAUD 0
#define UART_3_SPAN 32
#define UART_3_STOP_BITS 1
#define UART_3_SYNC_REG_DEPTH 2
#define UART_3_TYPE "altera_avalon_uart"
#define UART_3_USE_CTS_RTS 0
#define UART_3_USE_EOP_REGISTER 0


/*
 * watchdog_cnt_pio configuration
 *
 */

#define ALT_MODULE_CLASS_watchdog_cnt_pio altera_avalon_pio
#define WATCHDOG_CNT_PIO_BASE 0x103a0
#define WATCHDOG_CNT_PIO_BIT_CLEARING_EDGE_REGISTER 0
#define WATCHDOG_CNT_PIO_BIT_MODIFYING_OUTPUT_REGISTER 1
#define WATCHDOG_CNT_PIO_CAPTURE 0
#define WATCHDOG_CNT_PIO_DATA_WIDTH 32
#define WATCHDOG_CNT_PIO_DO_TEST_BENCH_WIRING 0
#define WATCHDOG_CNT_PIO_DRIVEN_SIM_VALUE 0
#define WATCHDOG_CNT_PIO_EDGE_TYPE "NONE"
#define WATCHDOG_CNT_PIO_FREQ 50000000
#define WATCHDOG_CNT_PIO_HAS_IN 0
#define WATCHDOG_CNT_PIO_HAS_OUT 1
#define WATCHDOG_CNT_PIO_HAS_TRI 0
#define WATCHDOG_CNT_PIO_IRQ -1
#define WATCHDOG_CNT_PIO_IRQ_INTERRUPT_CONTROLLER_ID -1
#define WATCHDOG_CNT_PIO_IRQ_TYPE "NONE"
#define WATCHDOG_CNT_PIO_NAME "/dev/watchdog_cnt_pio"
#define WATCHDOG_CNT_PIO_RESET_VALUE 0
#define WATCHDOG_CNT_PIO_SPAN 32
#define WATCHDOG_CNT_PIO_TYPE "altera_avalon_pio"

#endif /* __SYSTEM_H_ */
