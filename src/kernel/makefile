BUILD_PATH = ./../../build
GCC_FLAGS =  -g -fno-builtin -static -mcmodel=large -m64 -fno-stack-protector
LINK_FLAGS = -b elf64-x86-64 -z muldefs
OBJCOPY_FLAGS = -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary

# 添加所有必要的目标文件
OBJS = $(BUILD_PATH)/head.o \
		$(BUILD_PATH)/entry.o\
		$(BUILD_PATH)/printk.o \
		$(BUILD_PATH)/trap.o \
		$(BUILD_PATH)/main.o

$(BUILD_PATH)/%.o : %.c
	mkdir -p $(BUILD_PATH)
	gcc $(GCC_FLAGS) -c $< -o $@

$(BUILD_PATH)/head.o: head.S
	mkdir -p $(BUILD_PATH)
	gcc -E head.S > $(BUILD_PATH)/head.s
	as --64 -o $@ $(BUILD_PATH)/head.s

$(BUILD_PATH)/entry.o: entry.S
	mkdir -p $(BUILD_PATH)
	gcc -E entry.S > $(BUILD_PATH)/entry.s
	as --64 -o $@ $(BUILD_PATH)/entry.s

$(BUILD_PATH)/system: $(OBJS)
	mkdir -p $(BUILD_PATH)
	ld $(LINK_FLAGS) -o $@ $^ -T Kernel.lds
	objcopy --only-keep-debug $(BUILD_PATH)/system $(BUILD_PATH)/system.debug
	objcopy --strip-debug $(BUILD_PATH)/system


.PHONY: chmod
chmod:
	chmod -R  777 $(BUILD_PATH)/*

all: chmod $(BUILD_PATH)/system
	mkdir -p $(BUILD_PATH)
	objcopy $(OBJCOPY_FLAGS) $(BUILD_PATH)/system $(BUILD_PATH)/kernel.bin

.PHONY: test
test: $(BUILD_PATH)/system
