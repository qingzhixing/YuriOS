BUILD_PATH = ./../../build
$(BUILD_PATH)/main.o : main.c
	mkdir -p $(BUILD_PATH)
	gcc -mcmodel=large -fno-builtin -m64 -c main.c -o $@

$(BUILD_PATH)/head.o: head.S
	mkdir -p $(BUILD_PATH)
	gcc -E head.S > $(BUILD_PATH)/head.s
	as --64 -o $@ $(BUILD_PATH)/head.s

$(BUILD_PATH)/system: $(BUILD_PATH)/head.o $(BUILD_PATH)/main.o
	mkdir -p $(BUILD_PATH)
	ld -b elf64-x86-64 -o $@ $^ -T Kernel.lds

all: $(BUILD_PATH)/system
	mkdir -p $(BUILD_PATH)
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary $(BUILD_PATH)/system $(BUILD_PATH)/kernel.bin
	chmod -R ugo+rw $(BUILD_PATH)/*

.PHONY: test
test: $(BUILD_PATH)/head.o

