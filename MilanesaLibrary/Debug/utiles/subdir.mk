################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../utiles/auxiliares.c \
../utiles/files.c \
../utiles/messages.c \
../utiles/sockets.c 

OBJS += \
./utiles/auxiliares.o \
./utiles/files.o \
./utiles/messages.o \
./utiles/sockets.o 

C_DEPS += \
./utiles/auxiliares.d \
./utiles/files.d \
./utiles/messages.d \
./utiles/sockets.d 


# Each subdirectory must supply rules for building sources it contributes
utiles/%.o: ../utiles/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


