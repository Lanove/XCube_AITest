################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/adc.c \
../Core/Src/crc.c \
../Core/Src/dma.c \
../Core/Src/gpio.c \
../Core/Src/main.c \
../Core/Src/rng.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/tim.c \
../Core/Src/usart.c 

CPP_SRCS += \
../Core/Src/main_cxx.cpp 

C_DEPS += \
./Core/Src/adc.d \
./Core/Src/crc.d \
./Core/Src/dma.d \
./Core/Src/gpio.d \
./Core/Src/main.d \
./Core/Src/rng.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/tim.d \
./Core/Src/usart.d 

OBJS += \
./Core/Src/adc.o \
./Core/Src/crc.o \
./Core/Src/dma.o \
./Core/Src/gpio.o \
./Core/Src/main.o \
./Core/Src/main_cxx.o \
./Core/Src/rng.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/tim.o \
./Core/Src/usart.o 

CPP_DEPS += \
./Core/Src/main_cxx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I"/home/kohigashi/CLionProjects/F407-VA_Measure/Core/Inc" -I/home/kohigashi/CLionProjects/F407-VA_Measure/Drivers/STM32F4xx_HAL_Driver/Inc -I/home/kohigashi/CLionProjects/F407-VA_Measure/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I/home/kohigashi/CLionProjects/F407-VA_Measure/Drivers/CMSIS/Device/ST/STM32F4xx/Include -I/home/kohigashi/CLionProjects/F407-VA_Measure/Drivers/CMSIS/Device/ST/STM32F4xx/Include -I/home/kohigashi/CLionProjects/F407-VA_Measure/Drivers/CMSIS/Include -I/home/kohigashi/CLionProjects/F407-VA_Measure/Middlewares/ST/AI/Inc -I/home/kohigashi/CLionProjects/F407-VA_Measure/X-CUBE-AI/App -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/%.o Core/Src/%.su: ../Core/Src/%.cpp Core/Src/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m4 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -DTFLM_RUNTIME -DCMSIS_NN -DTFLM_RUNTIME_USE_ALL_OPERATORS=0 -DTF_LITE_STATIC_MEMORY -DTF_LITE_DISABLE_X86_NEON -DTF_LITE_MCU_DEBUG_LOG -DARM_MATH -DARM_MATH_LOOPUNROLL -DARM_MATH_DSP -DARM_MATH_CM4 -D__FPU_PRESENT=1U -c -I"/home/kohigashi/CLionProjects/F407-VA_Measure/Core/Inc" -I/home/kohigashi/CLionProjects/F407-VA_Measure/Drivers/STM32F4xx_HAL_Driver/Inc -I/home/kohigashi/CLionProjects/F407-VA_Measure/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I/home/kohigashi/CLionProjects/F407-VA_Measure/Drivers/CMSIS/Device/ST/STM32F4xx/Include -I/home/kohigashi/CLionProjects/F407-VA_Measure/Drivers/CMSIS/Device/ST/STM32F4xx/Include -I/home/kohigashi/CLionProjects/F407-VA_Measure/Drivers/CMSIS/Include -I/home/kohigashi/CLionProjects/F407-VA_Measure/Middlewares/ST/AI/Inc -I/home/kohigashi/CLionProjects/F407-VA_Measure/X-CUBE-AI/App -Os -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"  -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/adc.d ./Core/Src/adc.o ./Core/Src/adc.su ./Core/Src/crc.d ./Core/Src/crc.o ./Core/Src/crc.su ./Core/Src/dma.d ./Core/Src/dma.o ./Core/Src/dma.su ./Core/Src/gpio.d ./Core/Src/gpio.o ./Core/Src/gpio.su ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/main_cxx.d ./Core/Src/main_cxx.o ./Core/Src/main_cxx.su ./Core/Src/rng.d ./Core/Src/rng.o ./Core/Src/rng.su ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/tim.d ./Core/Src/tim.o ./Core/Src/tim.su ./Core/Src/usart.d ./Core/Src/usart.o ./Core/Src/usart.su

.PHONY: clean-Core-2f-Src

