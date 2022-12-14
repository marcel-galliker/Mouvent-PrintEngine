################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/bootloader.c \
../Sources/gpio.c \
../Sources/main.c \
../Sources/motor.c \
../Sources/network.c \
../Sources/robot_flash.c \
../Sources/rx_boot.c \
../Sources/rx_timer.c \
../Sources/rx_trace.c \
../Sources/version.c 

OBJS += \
./Sources/bootloader.o \
./Sources/gpio.o \
./Sources/main.o \
./Sources/motor.o \
./Sources/network.o \
./Sources/robot_flash.o \
./Sources/rx_boot.o \
./Sources/rx_timer.o \
./Sources/rx_trace.o \
./Sources/version.o 

C_DEPS += \
./Sources/bootloader.d \
./Sources/gpio.d \
./Sources/main.d \
./Sources/motor.d \
./Sources/network.d \
./Sources/robot_flash.d \
./Sources/rx_boot.d \
./Sources/rx_timer.d \
./Sources/rx_trace.d \
./Sources/version.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/%.o: ../Sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: FT9xx GCC Compiler'
	ft32-elf-gcc -D__FT900__ -DDEBUG -I"C:/Program Files (x86)/Bridgetek/FT9xx Toolchain/Toolchain/hardware/include" -I"../Includes" -I"../lib" -I"../lib/lwip/src/arch" -I"../lib/lwip/src/include" -I../../../rx_robot_ctrl/includes -Og -g -fvar-tracking -fvar-tracking-assignments -Wall -c -fmessage-length=0 -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


