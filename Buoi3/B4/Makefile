
TARGET = main

CC      = arm-none-eabi-gcc
AS      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

STFLASH = st-flash

GDB     = gdb-multiarch

SRC_DIRS = User/src \
		  Startup \
		  Driver/GPIO/src \
		  Driver/RCC/src \
		  Driver/EXTI/src \
		  Driver/UART/src \
		  Driver/SYSTICK/src\
		  Driver/AFIO/src\
		  Driver/DMA/src\
		  Driver/ADC/src\
		  Driver/NVIC/src\
		  Driver/TIM/src
		  

SRC_FILES +=$(foreach SRC_DIRS,$(SRC_DIRS),$(wildcard $(SRC_DIRS)/*))

INC_DIRS = User/inc \
			  Startup \
			  Driver/GPIO/inc \
			  Driver/RCC/inc \
			  Driver/EXTI/inc \
			  Driver/UART/inc \
			  Driver/TYPE \
			  Driver/AFIO/inc\
			  Driver/SYSTICK/inc\
			  Driver/DMA/inc\
			  Driver/ADC/inc\
		  	  Driver/NVIC/inc\
		 	  Driver/TIM/inc

INC_FILES +=$(foreach INC_DIRS,$(INC_DIRS),$(wildcard $(INC_DIRS)/*))
INC_DIR_OPT = $(foreach INC_DIRS,$(INC_DIRS),-I$(INC_DIRS))

PATH_OUTPUT = Output

LD_File = Linker/linker.ld

MCU = cortex-m3

CHIP=cortex-m3

CCFLAGS= -c -mcpu=$(CHIP) -mthumb -std=gnu11 -O0 $(INC_DIR_OPT)

vpath %.c $(SRC_DIRS)

OBJ_FILE := $(notdir $(SRC_FILES))
OBJ_FILE := $(subst .c,.o,$(OBJ_FILE))
PATH_OBJ := $(foreach OBJ_FILE,$(OBJ_FILE),$(PATH_OUTPUT)/$(OBJ_FILE))

LDFLAGS= -nostdlib -T $(LD_File) -Wl,-Map=$(PATH_OUTPUT)/$(TARGET).map

build: $(PATH_OUTPUT)/$(TARGET).bin

$(PATH_OUTPUT)/%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@
$(PATH_OUTPUT)/$(TARGET).elf: $(PATH_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^
$(PATH_OUTPUT)/$(TARGET).bin:$(PATH_OUTPUT)/$(TARGET).elf
	$(OBJCOPY) -O binary "$^" "$@"

flash: Output/$(TARGET).bin
	$(STFLASH) write Output/$(TARGET).bin 0x08000000

erase:
	$(STFLASH) erase

clean:
	rm -f Output/*
print-%:
	@echo $($(subst print-,,$@))