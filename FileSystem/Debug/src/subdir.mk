################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/archivo.c \
../src/consola.c \
../src/database.c \
../src/directorio.c \
../src/filesystem.c \
../src/main.c \
../src/nodo.c \
../src/paquete.c \
../src/server.c \
../src/socket.c 

OBJS += \
./src/archivo.o \
./src/consola.o \
./src/database.o \
./src/directorio.o \
./src/filesystem.o \
./src/main.o \
./src/nodo.o \
./src/paquete.o \
./src/server.o \
./src/socket.o 

C_DEPS += \
./src/archivo.d \
./src/consola.d \
./src/database.d \
./src/directorio.d \
./src/filesystem.d \
./src/main.d \
./src/nodo.d \
./src/paquete.d \
./src/server.d \
./src/socket.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_GNU_SOURCE -I"/home/utnso/workspace/tp-2015-1c-milanesa/MilanesaLibrary" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


