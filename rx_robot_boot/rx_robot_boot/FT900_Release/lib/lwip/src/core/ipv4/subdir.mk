################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lib/lwip/src/core/ipv4/autoip.c \
../lib/lwip/src/core/ipv4/dhcp.c \
../lib/lwip/src/core/ipv4/etharp.c \
../lib/lwip/src/core/ipv4/icmp.c \
../lib/lwip/src/core/ipv4/igmp.c \
../lib/lwip/src/core/ipv4/ip4.c \
../lib/lwip/src/core/ipv4/ip4_addr.c \
../lib/lwip/src/core/ipv4/ip4_frag.c 

OBJS += \
./lib/lwip/src/core/ipv4/autoip.o \
./lib/lwip/src/core/ipv4/dhcp.o \
./lib/lwip/src/core/ipv4/etharp.o \
./lib/lwip/src/core/ipv4/icmp.o \
./lib/lwip/src/core/ipv4/igmp.o \
./lib/lwip/src/core/ipv4/ip4.o \
./lib/lwip/src/core/ipv4/ip4_addr.o \
./lib/lwip/src/core/ipv4/ip4_frag.o 

C_DEPS += \
./lib/lwip/src/core/ipv4/autoip.d \
./lib/lwip/src/core/ipv4/dhcp.d \
./lib/lwip/src/core/ipv4/etharp.d \
./lib/lwip/src/core/ipv4/icmp.d \
./lib/lwip/src/core/ipv4/igmp.d \
./lib/lwip/src/core/ipv4/ip4.d \
./lib/lwip/src/core/ipv4/ip4_addr.d \
./lib/lwip/src/core/ipv4/ip4_frag.d 


# Each subdirectory must supply rules for building sources it contributes
lib/lwip/src/core/ipv4/%.o: ../lib/lwip/src/core/ipv4/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: FT9xx GCC Compiler'
	ft32-elf-gcc -D__FT900__ -DRELEASE -I"C:/Program Files (x86)/Bridgetek/FT9xx Toolchain/Toolchain/hardware/include" -I"../Includes" -I"../lib" -I"../lib/tinyprintf" -I"../lib/fatfs" -I"../lib/lwip/src/arch" -I"../lib/lwip/src/include" -I../../../rx_robot_ctrl/includes -Os -Wall -c -fmessage-length=0 -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


