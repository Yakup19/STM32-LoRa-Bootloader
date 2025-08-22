################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32G0xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := NUC.G070_LORA_BOOTLOADER
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
NUC.G070_LORA_BOOTLOADER.elf \

MAP_FILES += \
NUC.G070_LORA_BOOTLOADER.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
NUC.G070_LORA_BOOTLOADER.list \

OBJCOPY_HEX += \
NUC.G070_LORA_BOOTLOADER.hex \


# All Target
all: main-build

# Main-build Target
main-build: NUC.G070_LORA_BOOTLOADER.elf secondary-outputs

# Tool invocations
NUC.G070_LORA_BOOTLOADER.elf NUC.G070_LORA_BOOTLOADER.map: $(OBJS) $(USER_OBJS) C:\Users\yakup\OneDrive\Belgeler\STM32CubeIDE\Workspace_User\NUC.G070_LORA_BOOTLOADER\STM32G070RBTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "NUC.G070_LORA_BOOTLOADER.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m0plus -T"C:\Users\yakup\OneDrive\Belgeler\STM32CubeIDE\Workspace_User\NUC.G070_LORA_BOOTLOADER\STM32G070RBTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="NUC.G070_LORA_BOOTLOADER.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

NUC.G070_LORA_BOOTLOADER.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "NUC.G070_LORA_BOOTLOADER.list"
	@echo 'Finished building: $@'
	@echo ' '

NUC.G070_LORA_BOOTLOADER.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "NUC.G070_LORA_BOOTLOADER.hex"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) NUC.G070_LORA_BOOTLOADER.elf NUC.G070_LORA_BOOTLOADER.hex NUC.G070_LORA_BOOTLOADER.list NUC.G070_LORA_BOOTLOADER.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
