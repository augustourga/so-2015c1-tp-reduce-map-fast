################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/config.c \
../src/job.c \
../src/marta.c \
../src/server.c \
../src/tarea.c 

OBJS += \
./src/config.o \
./src/job.o \
./src/marta.o \
./src/server.o \
./src/tarea.o 

C_DEPS += \
./src/config.d \
./src/job.d \
./src/marta.d \
./src/server.d \
./src/tarea.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/tp-2015-1c-milanesa/MilanesaLibrary" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


