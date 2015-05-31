################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Si446x.c \
../src/aprs.c \
../src/ax25.c \
../src/core_cm0.c \
../src/cr_startup_lpc11xx.c \
../src/fifo.c \
../src/global.c \
../src/gps.c \
../src/i2c.c \
../src/main.c \
../src/modem.c \
../src/sensors.c \
../src/sleep.c \
../src/small_printf_support.c \
../src/small_utils.c \
../src/spi.c \
../src/target.c \
../src/uart.c 

OBJS += \
./src/Si446x.o \
./src/aprs.o \
./src/ax25.o \
./src/core_cm0.o \
./src/cr_startup_lpc11xx.o \
./src/fifo.o \
./src/global.o \
./src/gps.o \
./src/i2c.o \
./src/main.o \
./src/modem.o \
./src/sensors.o \
./src/sleep.o \
./src/small_printf_support.o \
./src/small_utils.o \
./src/spi.o \
./src/target.o \
./src/uart.o 

C_DEPS += \
./src/Si446x.d \
./src/aprs.d \
./src/ax25.d \
./src/core_cm0.d \
./src/cr_startup_lpc11xx.d \
./src/fifo.d \
./src/global.d \
./src/gps.d \
./src/i2c.d \
./src/main.d \
./src/modem.d \
./src/sensors.d \
./src/sleep.d \
./src/small_printf_support.d \
./src/small_utils.d \
./src/spi.d \
./src/target.d \
./src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DNDEBUG -D__CODE_RED -DCORE_M0 -D__USE_CMSIS=CMSIS_CORE_LPC11xx -D__USE_CMSIS_DSPLIB=CMSIS_DSPLIB_CM0 -D__LPC11XX__ -Os -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


