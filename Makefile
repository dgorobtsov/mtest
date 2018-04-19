 
BINARY = mtest
LD_SCRIPT = $(BINARY).ld
MAPFILE = $(BINARY).map

BUILD_ASM = 1 	# specify if assembly versions of test functions should be built

CC = $(RISCV)/bin/riscv32-unknown-elf-gcc
AS = $(RISCV)/bin/riscv32-unknown-elf-as

SRCS := main 

ifneq ($(BUILD_ASM),)
	SRCS += tests
endif

DEPS := config.h 

OBJS := $(SRCS:%=%.o)

INC_DIRS =
INC_FLAGS = $(addprefix -I,$(INC_DIRS))

CFLAGS = -march=rv32im 
ASFLAGS = -march=rv32im 
LDFLAGS = -Wl,--print-map > $(MAPFILE) -T $(LD_SCRIPT)

ifneq ($(BUILD_ASM),)
	 CPPFLAGS += -D BUILD_ASM
endif

$(LD_SCRIPT): $(LD_SCRIPT).c $(DEPS)
	$(CC) -P -E $< -o $@

%.o : %.s
	$(AS) $(ASFLAGS) -c $< -o $@

%.o : %.c $(DEPS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

all : $(BINARY)

$(BINARY): $(LD_SCRIPT) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

.PHONY: all clean

clean:
	rm -f $(OBJS) $(BINARY) $(MAPFILE) $(LD_SCRIPT)

