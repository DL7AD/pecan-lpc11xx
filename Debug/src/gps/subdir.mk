################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/gps/fifo.c \
../src/gps/uart.c 

OBJS += \
./src/gps/fifo.o \
./src/gps/uart.o 

C_DEPS += \
./src/gps/fifo.d \
./src/gps/uart.d 


# Each subdirectory must supply rules for building sources it contributes
src/gps/%.o: ../src/gps/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DDEBUG -D__CODE_RED -DCORE_M0 -D__USE_CMSIS=CMSIS_CORE_LPC11xx -D__USE_CMSIS_DSPLIB=CMSIS_DSPLIB_CM0 -D__LPC11XX__ -I"C:\Users\Sven\Documents\LPCXpresso_7.0.0_92\workspace\CMSIS_CORE_LPC11xx\inc" -I"C:\Users\Sven\Documents\LPCXpresso_7.0.0_92\workspace\pecanpico6\src\aprs" -I"C:\Users\Sven\Documents\LPCXpresso_7.0.0_92\workspace\pecanpico6\src\gps" -I"C:\Users\Sven\Documents\LPCXpresso_7.0.0_92\workspace\pecanpico6\src\radio" -I"C:\Users\Sven\Documents\LPCXpresso_7.0.0_92\workspace\pecanpico6\src\sensors" -I"C:\Users\Sven\Documents\LPCXpresso_7.0.0_92\workspace\CMSIS_DSPLIB_CM0\inc" -include"C:\Users\Sven\Documents\LPCXpresso_7.0.0_92\workspace\pecanpico6\src\radio\modem.h" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


