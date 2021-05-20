################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lib/lwip/src/arch/net.c \
../lib/lwip/src/arch/netif_ft900.c \
../lib/lwip/src/arch/sys_arch.c 

OBJS += \
./lib/lwip/src/arch/net.o \
./lib/lwip/src/arch/netif_ft900.o \
./lib/lwip/src/arch/sys_arch.o 

C_DEPS += \
./lib/lwip/src/arch/net.d \
./lib/lwip/src/arch/netif_ft900.d \
./lib/lwip/src/arch/sys_arch.d 


# Each subdirectory must supply rules for building sources it contributes
lib/lwip/src/arch/%.o: ../lib/lwip/src/arch/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: FT9xx GCC Compiler'
	ft32-elf-gcc -D__FT900__ -DRELEASE -I"C:/Program Files (x86)/Bridgetek/FT9xx Toolchain/Toolchain/hardware/include" -I"../Includes" -I"../lib" -I"../lib/tinyprintf" -I"../lib/fatfs" -I"../lib/lwip/src/arch" -I"../lib/lwip/src/include" -I../../../rx_robot_ctrl/includes -Os -Wall -c -fmessage-length=0 -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


