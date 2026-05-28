include sources.mk

EXEC        = app.elf
OBJ_DIR     = Objects
LINKER_FILE = Linker/stm32f4.ld
CPU         = cortex-m4
ARCH        = armv7e-m
SPECS       = nosys.specs
FPU         = fpv4-sp-d16

ARCHFLAGS = -mcpu=$(CPU) -mthumb -march=$(ARCH) -mfloat-abi=hard -mfpu=$(FPU) --specs=$(SPECS)

OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

CC = arm-none-eabi-gcc

CFLAGS  = -g -O0 -std=c99 -Werror -Wall -DSTM32F411xE $(ARCHFLAGS)
LDFLAGS = -nostdlib -T $(LINKER_FILE)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -o $@

.PHONY: build
build : $(EXEC)

$(EXEC) : $(OBJS)
	$(CC) $(OBJS) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o $@

.PHONY: flash
flash :
	openocd -f board/st_nucleo_f4.cfg \
	  -c "init" \
	  -c "reset halt" \
	  -c "program $(EXEC) verify" \
	  -c "reset run" \
	  -c "shutdown"

.PHONY: clean
clean :
	rm -f $(OBJ_DIR)/*.o $(EXEC)