################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=
LINKER_SCRIPT_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS :=  \
../Device_Startup/ \
../FatFS


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../coolant_control.c \
../Device_Startup/startup_sam3xa.c \
../Device_Startup/system_sam3xa.c \
../eeprom.c \
../efc.c \
../FatFS/diskio.c \
../FatFS/fattime_rtc.c \
../FatFS/ff.c \
../FatFS/LinkedList.c \
../flash_efc.c \
../flash_storage.c \
../gcode.c \
../limits.c \
../main.c \
../motion_control.c \
../nuts_bolts.c \
../planner.c \
../print.c \
../probe.c \
../protocol.c \
../ra8875.c \
../report.c \
../serial.c \
../settings.c \
../spindle_control.c \
../stepper.c \
../syscall.c \
../system.c \
../utils.c


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
coolant_control.o \
Device_Startup/startup_sam3xa.o \
Device_Startup/system_sam3xa.o \
eeprom.o \
efc.o \
FatFS/diskio.o \
FatFS/fattime_rtc.o \
FatFS/ff.o \
FatFS/LinkedList.o \
flash_efc.o \
flash_storage.o \
gcode.o \
limits.o \
main.o \
motion_control.o \
nuts_bolts.o \
planner.o \
print.o \
probe.o \
protocol.o \
ra8875.o \
report.o \
serial.o \
settings.o \
spindle_control.o \
stepper.o \
syscall.o \
system.o \
utils.o

OBJS_AS_ARGS +=  \
coolant_control.o \
Device_Startup/startup_sam3xa.o \
Device_Startup/system_sam3xa.o \
eeprom.o \
efc.o \
FatFS/diskio.o \
FatFS/fattime_rtc.o \
FatFS/ff.o \
FatFS/LinkedList.o \
flash_efc.o \
flash_storage.o \
gcode.o \
limits.o \
main.o \
motion_control.o \
nuts_bolts.o \
planner.o \
print.o \
probe.o \
protocol.o \
ra8875.o \
report.o \
serial.o \
settings.o \
spindle_control.o \
stepper.o \
syscall.o \
system.o \
utils.o

C_DEPS +=  \
coolant_control.d \
Device_Startup/startup_sam3xa.d \
Device_Startup/system_sam3xa.d \
eeprom.d \
efc.d \
FatFS/diskio.d \
FatFS/fattime_rtc.d \
FatFS/ff.d \
FatFS/LinkedList.d \
flash_efc.d \
flash_storage.d \
gcode.d \
limits.d \
main.d \
motion_control.d \
nuts_bolts.d \
planner.d \
print.d \
probe.d \
protocol.d \
ra8875.d \
report.d \
serial.d \
settings.d \
spindle_control.d \
stepper.d \
syscall.d \
system.d \
utils.d

C_DEPS_AS_ARGS +=  \
coolant_control.d \
Device_Startup/startup_sam3xa.d \
Device_Startup/system_sam3xa.d \
eeprom.d \
efc.d \
FatFS/diskio.d \
FatFS/fattime_rtc.d \
FatFS/ff.d \
FatFS/LinkedList.d \
flash_efc.d \
flash_storage.d \
gcode.d \
limits.d \
main.d \
motion_control.d \
nuts_bolts.d \
planner.d \
print.d \
probe.d \
protocol.d \
ra8875.d \
report.d \
serial.d \
settings.d \
spindle_control.d \
stepper.d \
syscall.d \
system.d \
utils.d

OUTPUT_FILE_PATH +=grbl-v9.elf

OUTPUT_FILE_PATH_AS_ARGS +=grbl-v9.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

LINKER_SCRIPT_DEP+=  \
../Device_Startup/sam3x8e_flash.ld \
../Device_Startup/sam3x8e_sram.ld \
../Device_Startup/sam3xa_flash.ld \
../Device_Startup/sam3xa_sram.ld


# AVR32/GNU C Compiler



























































./%.o: .././%.c
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : 4.9.3
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-gcc.exe$(QUOTE)  -x c -mthumb -D__SAM3X8E__ -DDEBUG  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\SAM3X_DFP\1.0.33\include" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\cmsis\4.2.0\CMSIS\Include"  -O1 -ffunction-sections -mlong-calls -g3 -Wall -mcpu=cortex-m3 -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

Device_Startup/%.o: ../Device_Startup/%.c
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : 4.9.3
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-gcc.exe$(QUOTE)  -x c -mthumb -D__SAM3X8E__ -DDEBUG  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\SAM3X_DFP\1.0.33\include" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\cmsis\4.2.0\CMSIS\Include"  -O1 -ffunction-sections -mlong-calls -g3 -Wall -mcpu=cortex-m3 -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

FatFS/%.o: ../FatFS/%.c
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : 4.9.3
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-gcc.exe$(QUOTE)  -x c -mthumb -D__SAM3X8E__ -DDEBUG  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\SAM3X_DFP\1.0.33\include" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\cmsis\4.2.0\CMSIS\Include"  -O1 -ffunction-sections -mlong-calls -g3 -Wall -mcpu=cortex-m3 -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP) $(LIB_DEP) $(LINKER_SCRIPT_DEP)
	@echo Building target: $@
	@echo Invoking: ARM/GNU Linker : 4.9.3
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -mthumb -Wl,-Map="grbl-v9.map" -Wl,--start-group -lm  -Wl,--end-group -L"..\\Device_Startup"  -Wl,--gc-sections -mcpu=cortex-m3 -Tsam3x8e_flash.ld  
	@echo Finished building target: $@
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objcopy.exe" -O binary "grbl-v9.elf" "grbl-v9.bin"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature  "grbl-v9.elf" "grbl-v9.hex"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objcopy.exe" -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O binary "grbl-v9.elf" "grbl-v9.eep" || exit 0
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objdump.exe" -h -S "grbl-v9.elf" > "grbl-v9.lss"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature  "grbl-v9.elf" "grbl-v9.srec"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-size.exe" "grbl-v9.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "grbl-v9.elf" "grbl-v9.a" "grbl-v9.hex" "grbl-v9.bin" "grbl-v9.lss" "grbl-v9.eep" "grbl-v9.map" "grbl-v9.srec"
	