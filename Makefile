CUR_DIR = $(PWD)

# KERNEL_PATH := $(CUR_DIR)/../cvi_mmf_sdk/linux_5.10/
HOST_TOOL_PATH := $(CUR_DIR)/../cvi_mmf_sdk/host-tools/gcc/riscv64-linux-musl-x86_64/bin

CC = $(HOST_TOOL_PATH)/riscv64-unknown-linux-musl-gcc
CXX = $(HOST_TOOL_PATH)/riscv64-unknown-linux-musl-g++

# INCS = $(KERNEL_PATH)/include
# INCS +=
SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)


TARGET = sample_spi_oled

EXTRA_CFLAGS = $(INCS) $(DEFS)
EXTRA_LDFLAGS = $(LIBS) -mcpu=c906fdv -march=rv64imafdcv0p7xthead -mcmodel=medany -mabi=lp64d

all: $(TARGET) install

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET): $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(EXTRA_LDFLAGS)

install:
	@cp $(TARGET) /mnt/localwork/

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET)