# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ------------------------------------------------

######################################
# target
######################################
TARGET = stm32f4_template

######################################
# building variables
######################################
# debug build?
DEBUG = 0
# optimization
OPT = -O3

# firmware library path
PERIFLIB_PATH = 

# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES = ../Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c \
			$(wildcard ../Drivers/STM32F4xx_HAL_Driver/Src/*.c) \
			$(wildcard ../Drivers/BSP/Components/ili9341/*.c) \
			$(wildcard ../Drivers/BSP/Components/l3gd20/*.c) \
			$(wildcard ../Drivers/BSP/Components/stmpe811/*.c) \
			$(wildcard ../Drivers/BSP/STM32F429I-Discovery/*.c) \
			$(wildcard ../Middlewares/FreeRTOS/Source/CMSIS_RTOS/*.c) \
			$(wildcard ../Middlewares/FreeRTOS/Source/*.c) \
			../Middlewares/FreeRTOS/Source/portable/Common/mpu_wrappers.c \
			../Middlewares/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c \
			../Middlewares/FreeRTOS/Source/portable/MemMang/heap_2.c \
			$(wildcard ../Utilities/CPU/*.c) \
			$(wildcard ../Middlewares/STemWin/Config/*.c) \
			../Middlewares/STemWin/OS/GUI_X_OS.c \
			$(wildcard ../Middlewares/STM32_USB_Host_Library/Core/Src/*.c) \
			$(wildcard ../Middlewares/STM32_USB_Host_Library/Class/MSC/Src/*.c) \
			$(wildcard ../Middlewares/FatFs/source/*.c) \
			$(wildcard ../Middlewares/FatFs/source/option/syscall.c) \
			$(wildcard ../Application/*.c)

# ASM sources
ASM_SOURCES =  \
startup_stm32f429xx.s


######################################
# firmware library
######################################
PERIFLIB_SOURCES = 


#######################################
# binaries
#######################################
# gcc-arm installation folder
# BINPATH = /opt/gcc-arm-none-eabi-7-2018-q2-update/bin/
PREFIX = arm-none-eabi-
ifdef BINPATH
CC = $(BINPATH)/$(PREFIX)gcc
AS = $(BINPATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(BINPATH)/$(PREFIX)objcopy
AR = $(BINPATH)/$(PREFIX)ar
SZ = $(BINPATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
AR = $(PREFIX)ar
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32F429xx


# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-I../Application \
-I../Drivers/STM32F4xx_HAL_Driver/Inc \
-I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy \
-I../Drivers/CMSIS/Device/ST/STM32F4xx/Include \
-I../Drivers/BSP/Components/Common \
-I../Drivers/BSP/STM32F429I-Discovery \
-I../Drivers/CMSIS/Include \
-I../Middlewares/FreeRTOS/Source/include \
-I../Middlewares/FreeRTOS/Source/CMSIS_RTOS \
-I../Middlewares/FreeRTOS/Source/portable/GCC/ARM_CM4F \
-I../Middlewares/STemWin/inc \
-I../Middlewares/STemWin/Config \
-I../Middlewares/STM32_USB_Host_Library/Core/Inc \
-I../Middlewares/STM32_USB_Host_Library/Class/MSC/Inc \
-I../Middlewares/FatFs/source \
-I../Utilities/CPU


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32F429ZITx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys \
../Middlewares/STemWin/Lib/STemWin_CM4_OS_wc32.a
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	-rm -fR .dep $(BUILD_DIR)
  
#######################################
# dependencies
#######################################
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

#######################################
# flash
#######################################
flash:
	STM32_Programmer_CLI --connect port=SWD --halt --write '$(BUILD_DIR)/$(TARGET).hex' --verify --start 0x8000000

# *** EOF ***