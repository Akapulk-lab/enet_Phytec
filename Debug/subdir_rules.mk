################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
enet_custom_board_config.o: C:/ti/ind_comms_sdk_am64x_09_00_00_03/examples/networking/enet_layer2_icssg/icssg_layer2_switch/am64x-evm/r5fss0-0_freertos/enet_custom_board_config.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/Ctiti-cgt-armllvm_2.3.3.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -mlittle-endian -mthumb -I"C:/ti/Ctiti-cgt-armllvm_2.3.3.LTS/include/c" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/source" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/kernel/freertos/FreeRTOS-Kernel/include" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CR5F" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/kernel/freertos/config/am64x/r5f" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/utils" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/utils/include" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/utils/V3" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/core" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/core/include" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/core/include/phy" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/core/include/core" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/soc/k3/am64x_am243x" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/hw_include" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/hw_include/mdio/V4" -DSOC_AM64X -DENET_ENABLE_PER_ICSSG=1 -D_DEBUG_=1 -g -Wall -Wno-gnu-variable-sized-type-not-at-end -Wno-unused-function -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)" -I"D:/WorkSpace/enet_layer2_icssg_am64x-evm_r5fss0-0_freertos_ti-arm-clang/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

enet_layer2_icssg.o: C:/ti/ind_comms_sdk_am64x_09_00_00_03/examples/networking/enet_layer2_icssg/enet_layer2_icssg.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/Ctiti-cgt-armllvm_2.3.3.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -mlittle-endian -mthumb -I"C:/ti/Ctiti-cgt-armllvm_2.3.3.LTS/include/c" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/source" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/kernel/freertos/FreeRTOS-Kernel/include" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CR5F" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/kernel/freertos/config/am64x/r5f" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/utils" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/utils/include" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/utils/V3" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/core" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/core/include" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/core/include/phy" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/core/include/core" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/soc/k3/am64x_am243x" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/hw_include" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/hw_include/mdio/V4" -DSOC_AM64X -DENET_ENABLE_PER_ICSSG=1 -D_DEBUG_=1 -g -Wall -Wno-gnu-variable-sized-type-not-at-end -Wno-unused-function -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)" -I"D:/WorkSpace/enet_layer2_icssg_am64x-evm_r5fss0-0_freertos_ti-arm-clang/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-1957227631: C:/ti/ind_comms_sdk_am64x_09_00_00_03/examples/networking/enet_layer2_icssg/icssg_layer2_switch/am64x-evm/r5fss0-0_freertos/example.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/ccs1260/ccs/utils/sysconfig_1.19.0/sysconfig_cli.bat" --script "C:/ti/ind_comms_sdk_am64x_09_00_00_03/examples/networking/enet_layer2_icssg/icssg_layer2_switch/am64x-evm/r5fss0-0_freertos/example.syscfg" -o "syscfg" -s "C:/ti/ind_comms_sdk_am64x_09_00_00_03/.metadata/product.json" --context "r5fss0-0" --part Default --package ALV --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/ti_dpl_config.c: build-1957227631 C:/ti/ind_comms_sdk_am64x_09_00_00_03/examples/networking/enet_layer2_icssg/icssg_layer2_switch/am64x-evm/r5fss0-0_freertos/example.syscfg
syscfg/ti_dpl_config.h: build-1957227631
syscfg/ti_drivers_config.c: build-1957227631
syscfg/ti_drivers_config.h: build-1957227631
syscfg/ti_drivers_open_close.c: build-1957227631
syscfg/ti_drivers_open_close.h: build-1957227631
syscfg/ti_pinmux_config.c: build-1957227631
syscfg/ti_power_clock_config.c: build-1957227631
syscfg/ti_board_config.c: build-1957227631
syscfg/ti_board_config.h: build-1957227631
syscfg/ti_board_open_close.c: build-1957227631
syscfg/ti_board_open_close.h: build-1957227631
syscfg/ti_enet_config.c: build-1957227631
syscfg/ti_enet_config.h: build-1957227631
syscfg/ti_enet_open_close.c: build-1957227631
syscfg/ti_enet_open_close.h: build-1957227631
syscfg/ti_enet_soc.c: build-1957227631
syscfg/ti_enet_lwipif.c: build-1957227631
syscfg/ti_enet_lwipif.h: build-1957227631
syscfg/ti_pru_io_config.inc: build-1957227631
syscfg: build-1957227631

syscfg/%.o: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/Ctiti-cgt-armllvm_2.3.3.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -mlittle-endian -mthumb -I"C:/ti/Ctiti-cgt-armllvm_2.3.3.LTS/include/c" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/source" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/kernel/freertos/FreeRTOS-Kernel/include" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CR5F" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/kernel/freertos/config/am64x/r5f" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/utils" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/utils/include" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/utils/V3" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/core" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/core/include" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/core/include/phy" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/core/include/core" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/soc/k3/am64x_am243x" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/hw_include" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/hw_include/mdio/V4" -DSOC_AM64X -DENET_ENABLE_PER_ICSSG=1 -D_DEBUG_=1 -g -Wall -Wno-gnu-variable-sized-type-not-at-end -Wno-unused-function -MMD -MP -MF"syscfg/$(basename $(<F)).d_raw" -MT"$(@)" -I"D:/WorkSpace/enet_layer2_icssg_am64x-evm_r5fss0-0_freertos_ti-arm-clang/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-1943723473:
	@$(MAKE) --no-print-directory -Onone -f subdir_rules.mk build-1943723473-inproc

build-1943723473-inproc: C:/ti/ind_comms_sdk_am64x_09_00_00_03/examples/networking/enet_layer2_icssg/icssg_layer2_switch/am64x-evm/r5fss0-0_freertos/example.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/ccs1260/ccs/utils/sysconfig_1.19.0/sysconfig_cli.bat" --script "C:/ti/ind_comms_sdk_am64x_09_00_00_03/examples/networking/enet_layer2_icssg/icssg_layer2_switch/am64x-evm/r5fss0-0_freertos/example.syscfg" -o "syscfg" -s "C:/ti/ind_comms_sdk_am64x_09_00_00_03/.metadata/product.json" --context "r5fss0-0" --part Default --package ALV --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/ti_dpl_config.c: build-1943723473 C:/ti/ind_comms_sdk_am64x_09_00_00_03/examples/networking/enet_layer2_icssg/icssg_layer2_switch/am64x-evm/r5fss0-0_freertos/example.syscfg
syscfg/ti_dpl_config.h: build-1943723473
syscfg/ti_drivers_config.c: build-1943723473
syscfg/ti_drivers_config.h: build-1943723473
syscfg/ti_drivers_open_close.c: build-1943723473
syscfg/ti_drivers_open_close.h: build-1943723473
syscfg/ti_pinmux_config.c: build-1943723473
syscfg/ti_power_clock_config.c: build-1943723473
syscfg/ti_board_config.c: build-1943723473
syscfg/ti_board_config.h: build-1943723473
syscfg/ti_board_open_close.c: build-1943723473
syscfg/ti_board_open_close.h: build-1943723473
syscfg/ti_enet_config.c: build-1943723473
syscfg/ti_enet_config.h: build-1943723473
syscfg/ti_enet_open_close.c: build-1943723473
syscfg/ti_enet_open_close.h: build-1943723473
syscfg/ti_enet_soc.c: build-1943723473
syscfg/ti_enet_lwipif.c: build-1943723473
syscfg/ti_enet_lwipif.h: build-1943723473
syscfg/ti_pru_io_config.inc: build-1943723473
syscfg: build-1943723473

main.o: C:/ti/ind_comms_sdk_am64x_09_00_00_03/examples/networking/enet_layer2_icssg/icssg_layer2_switch/am64x-evm/r5fss0-0_freertos/main.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/Ctiti-cgt-armllvm_2.3.3.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-r5 -mfloat-abi=hard -mfpu=vfpv3-d16 -mlittle-endian -mthumb -I"C:/ti/Ctiti-cgt-armllvm_2.3.3.LTS/include/c" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/source" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/kernel/freertos/FreeRTOS-Kernel/include" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CR5F" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/kernel/freertos/config/am64x/r5f" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/utils" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/utils/include" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/utils/V3" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/core" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/core/include" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/core/include/phy" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/core/include/core" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/soc/k3/am64x_am243x" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/hw_include" -I"C:/ti/ind_comms_sdk_am64x_09_00_00_03/mcu_plus_sdk/source/networking/enet/hw_include/mdio/V4" -DSOC_AM64X -DENET_ENABLE_PER_ICSSG=1 -D_DEBUG_=1 -g -Wall -Wno-gnu-variable-sized-type-not-at-end -Wno-unused-function -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)" -I"D:/WorkSpace/enet_layer2_icssg_am64x-evm_r5fss0-0_freertos_ti-arm-clang/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


