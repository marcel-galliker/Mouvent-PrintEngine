################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lib/lwip/src/netif/ethernet.c \
../lib/lwip/src/netif/ethernetif.c \
../lib/lwip/src/netif/lowpan6.c \
../lib/lwip/src/netif/slipif.c 

OBJS += \
./lib/lwip/src/netif/ethernet.o \
./lib/lwip/src/netif/ethernetif.o \
./lib/lwip/src/netif/lowpan6.o \
./lib/lwip/src/netif/slipif.o 

C_DEPS += \
./lib/lwip/src/netif/ethernet.d \
./lib/lwip/src/netif/ethernetif.d \
./lib/lwip/src/netif/lowpan6.d \
./lib/lwip/src/netif/slipif.d 


# Each subdirectory must supply rules for building sources it contributes
lib/lwip/src/netif/%.o: ../lib/lwip/src/netif/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: FT9xx GCC Compiler'
	ft32-elf-gcc -D__FT900__ -DDEBUG -I"C:/Program Files (x86)/Bridgetek/FT9xx Toolchain/Toolchain/hardware/include" -I"../Includes" -I"../lib" -I"../lib/lwip/src/arch" -I"../lib/lwip/src/include" -I../../../rx_robot_ctrl/includes -Og -g -fvar-tracking -fvar-tracking-assignments -Wall -c -fmessage-length=0 -ffunction-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


