BUILD_PATH = ./../../build
GCC_FLAGS = -static -mcmodel=large -fno-builtin -m64 -fno-stack-protector
LINK_FLAGS = -b elf64-x86-64 -z muldefs
OBJCOPY_FLAGS = -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary

$(BUILD_PATH)/%.o : %.c
	mkdir -p $(BUILD_PATH)
	gcc $(GCC_FLAGS) -c $< -o $@

$(BUILD_PATH)/head.o: head.S
	mkdir -p $(BUILD_PATH)
	gcc -E head.S > $(BUILD_PATH)/head.s
	as --64 -o $@ $(BUILD_PATH)/head.s

$(BUILD_PATH)/system: $(BUILD_PATH)/head.o\
 $(BUILD_PATH)/printk.o\
 $(BUILD_PATH)/main.o
	mkdir -p $(BUILD_PATH)
	ld $(LINK_FLAGS) -o $@ $^ -T Kernel.lds

.PHONY: chmod
chmod:
	chmod -R  777 $(BUILD_PATH)/*

all: chmod $(BUILD_PATH)/system
	mkdir -p $(BUILD_PATH)
	objcopy $(OBJCOPY_FLAGS) $(BUILD_PATH)/system $(BUILD_PATH)/kernel.bin

.PHONY: test
test: $(BUILD_PATH)/head.o

.phony: clean
clean:
	rm -rf *.o *.s~ *.s *.S~ *.c~ *.h~ system  Makefile~ Kernel.lds~ kernel.bin

