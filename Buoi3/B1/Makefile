# Tên file thực thi đầu ra
TARGET = DS1307_RTC

# Thư mục chứa mã nguồn và output
SRC_DIR   = src
INC_DIR   = inc
LD_DIR    = ld
BUILD_DIR = build

# Trình biên dịch ARM GCC Cross-Compiler
PREFIX  = arm-none-eabi-
CC      = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE    = $(PREFIX)size

# Tự động lấy tất cả các file .c và .s trong thư mục src/
C_SRCS = $(wildcard $(SRC_DIR)/*.c)
S_SRCS = $(wildcard $(SRC_DIR)/*.s)

# Tạo danh sách các file .o tương ứng nằm trong thư mục build/
OBJS  = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SRCS))
OBJS += $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.o, $(S_SRCS))

# Linker Script
LDSCRIPT = $(LD_DIR)/stm32_f103.ld

# Cờ biên dịch cho Cortex-M3 (STM32F103)
MCU_FLAGS = -mcpu=cortex-m3 -mthumb

# Cờ cho Compiler (CFLAGS)
CFLAGS  = $(MCU_FLAGS)
CFLAGS += -I$(INC_DIR)
CFLAGS += -Wall -Wextra
CFLAGS += -g -O0
CFLAGS += -DSTM32F10X_MD

# Cờ cho Assembler (ASFLAGS)
ASFLAGS = $(MCU_FLAGS) -g

# Cờ cho Linker (LDFLAGS)
LDFLAGS  = $(MCU_FLAGS)
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -nostartfiles
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref
LDFLAGS += --specs=nano.specs -specs=nosys.specs

# Target mặc định khi gõ lệnh 'make'
all: $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex
	@echo "================================"
	@echo " BUILD SUCCESSFUL! "
	@echo "================================"
	$(SIZE) $(BUILD_DIR)/$(TARGET).elf

# 1. Biên dịch từ file .c thành file .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "[CC] $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@

# 1b. Biên dịch từ file .s (assembly) thành file .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s | $(BUILD_DIR)
	@echo "[AS] $< -> $@"
	$(CC) $(ASFLAGS) -c $< -o $@

# 2. Link các file .o thành file thực thi ELF
$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	@echo "[LD] Linking to $@"
	$(CC) $(OBJS) $(LDFLAGS) -o $@

# 3. Chuyển đổi từ file ELF sang file BIN
$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	@echo "[OBJCOPY] -> $@"
	$(OBJCOPY) -O binary $< $@

# 4. Chuyển đổi từ file ELF sang file HEX
$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	@echo "[OBJCOPY] -> $@"
	$(OBJCOPY) -O ihex $< $@

# Tạo thư mục build nếu chưa có
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Lệnh dọn dẹp các file đã build
clean:
	rm -rf $(BUILD_DIR)

# Lệnh nạp trực tiếp qua ST-Link (bằng st-flash)
flash: all
	st-flash write $(BUILD_DIR)/$(TARGET).bin 0x08000000

.PHONY: all clean flash