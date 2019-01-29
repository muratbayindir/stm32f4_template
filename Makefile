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
C_SOURCES = $(wildcard Drivers/CMSIS/Device/ST/STM32F4xx/Source/Templates/*.c) \
			$(wildcard Drivers/STM32F4xx_HAL_Driver/Src/*.c) \
			$(wildcard Drivers/BSP/Components/ili9341/*.c) \
			$(wildcard Drivers/BSP/Components/l3gd20/*.c) \
			$(wildcard Drivers/BSP/Components/stmpe811/*.c) \
			$(wildcard Drivers/BSP/STM32F429I-Discovery/*.c) \
			$(wildcard Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/*.c) \
			$(wildcard Middlewares/Third_Party/FreeRTOS/Source/*.c) \
			Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c \
			Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_2.c \
			$(wildcard Utilities/CPU/*.c) \
			$(wildcard Middlewares/ST/STemWin/Config/*.c) \
			Middlewares/ST/STemWin/OS/GUI_X.c \
			$(wildcard Middlewares/ST/STM32_USB_Host_Library/Core/Src/*.c) \
			$(wildcard Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Src/*.c) \
			$(wildcard Middlewares/Third_Party/FatFs/src/*.c) \
			$(wildcard Src/*.c)

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
-IInc \
-IDrivers/STM32F4xx_HAL_Driver/Inc \
-IDrivers/STM32F4xx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32F4xx/Include \
-IDrivers/BSP/Components/Common \
-IDrivers/BSP/STM32F429I-Discovery \
-IDrivers/CMSIS/Include \
-IMiddlewares/Third_Party/FreeRTOS/Source/include \
-IMiddlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS \
-IMiddlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
-IMiddlewares/ST/STemWin/inc \
-IMiddlewares/ST/STemWin/Config \
-IMiddlewares/ST/STM32_USB_Host_Library/Core/Inc \
-IMiddlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc \
-IMiddlewares/Third_Party/FatFs/src \
-IUtilities/CPU


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
Middlewares/ST/STemWin/Lib/STemWin540_CM4_GCC.a
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


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
# program
#######################################
program:
	stm32_programmer --connect port=SWD --halt --write '$(BUILD_DIR)/$(TARGET).hex' --verify --start 0x8000000

# *** EOF ***