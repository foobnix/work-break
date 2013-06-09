################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/about.c \
../src/core.c \
../src/fullscreen.c \
../src/preferences.c \
../src/trayicon.c 

OBJS += \
./src/about.o \
./src/core.o \
./src/fullscreen.o \
./src/preferences.o \
./src/trayicon.o 

C_DEPS += \
./src/about.d \
./src/core.d \
./src/fullscreen.d \
./src/preferences.d \
./src/trayicon.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include/atk-1.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/gtk-2.0 -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/gio-unix-2.0/ -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng12 -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


