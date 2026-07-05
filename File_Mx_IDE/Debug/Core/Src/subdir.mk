################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/app_entry.c \
../Core/Src/hx711.c \
../Core/Src/main.c \
../Core/Src/servo_tg9e.c \
../Core/Src/stm32wb0x_hal_msp.c \
../Core/Src/stm32wb0x_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32wb0x.c 

OBJS += \
./Core/Src/app_entry.o \
./Core/Src/hx711.o \
./Core/Src/main.o \
./Core/Src/servo_tg9e.o \
./Core/Src/stm32wb0x_hal_msp.o \
./Core/Src/stm32wb0x_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32wb0x.o 

C_DEPS += \
./Core/Src/app_entry.d \
./Core/Src/hx711.d \
./Core/Src/main.d \
./Core/Src/servo_tg9e.d \
./Core/Src/stm32wb0x_hal_msp.d \
./Core/Src/stm32wb0x_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32wb0x.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0plus -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32WB09 -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32WB0x_HAL_Driver/Inc -I../Drivers/STM32WB0x_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32WB0X/Include -I../Drivers/CMSIS/Include -I../STM32_BLE/App -I../STM32_BLE/Target -I../System/Config/Debug_GPIO -I../System/Interfaces -I../Utilities/trace/adv_trace -I../Projects/Common/BLE/Interfaces -I../Projects/Common/BLE/Modules -I../Projects/Common/BLE/Modules/RTDebug -I../Projects/Common/BLE/Modules/RADIO_utils/Inc -I../Projects/Common/BLE/Modules/Profiles/Inc -I../Projects/Common/BLE/Modules/PKAMGR/Inc -I../Projects/Common/BLE/Modules/NVMDB/Inc -I../Projects/Common/BLE/Modules/Flash -I../Projects/Common/BLE/Startup -I../Utilities/misc -I../Utilities/sequencer -I../Utilities/lpm/tiny_lpm -I../Middlewares/ST/STM32_BLE -I../Middlewares/ST/STM32_BLE/cryptolib/Inc -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/Common -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/AES -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/AES/CBC -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/AES/CMAC -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/AES/Common -I../Middlewares/ST/STM32_BLE/cryptolib/Inc/AES/ECB -I../Middlewares/ST/STM32_BLE/evt_handler/inc -I../Middlewares/ST/STM32_BLE/queued_writes/inc -I../Middlewares/ST/STM32_BLE/stack/include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/app_entry.cyclo ./Core/Src/app_entry.d ./Core/Src/app_entry.o ./Core/Src/app_entry.su ./Core/Src/hx711.cyclo ./Core/Src/hx711.d ./Core/Src/hx711.o ./Core/Src/hx711.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/servo_tg9e.cyclo ./Core/Src/servo_tg9e.d ./Core/Src/servo_tg9e.o ./Core/Src/servo_tg9e.su ./Core/Src/stm32wb0x_hal_msp.cyclo ./Core/Src/stm32wb0x_hal_msp.d ./Core/Src/stm32wb0x_hal_msp.o ./Core/Src/stm32wb0x_hal_msp.su ./Core/Src/stm32wb0x_it.cyclo ./Core/Src/stm32wb0x_it.d ./Core/Src/stm32wb0x_it.o ./Core/Src/stm32wb0x_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32wb0x.cyclo ./Core/Src/system_stm32wb0x.d ./Core/Src/system_stm32wb0x.o ./Core/Src/system_stm32wb0x.su

.PHONY: clean-Core-2f-Src

