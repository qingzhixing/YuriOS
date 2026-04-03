BUILD_PATH = ./build
SRC_PATH = ./src
GDB_SCRIPT = ./build-tools/qemu/client.gdb

$(BUILD_PATH)/%.bin: $(SRC_PATH)/bootloader/%.nasm
	mkdir -p $(BUILD_PATH)
	nasm -I $(SRC_PATH)/bootloader -f bin -o $@ $<

$(BUILD_PATH)/kernel.bin:
	make -C $(SRC_PATH)/kernel all

$(BUILD_PATH)/boot.img: $(BUILD_PATH)/boot.bin \
						$(BUILD_PATH)/loader.bin \
						$(BUILD_PATH)/kernel.bin
	mkdir -p $(BUILD_PATH)
	qemu-img create -f raw $@ 1474560
	dd if=$(BUILD_PATH)/boot.bin of=$@ bs=512 count=1 conv=notrunc

	sudo mount $@ /media/ -t vfat -o loop
	sudo cp $(BUILD_PATH)/loader.bin /media/
	sudo cp $(BUILD_PATH)/kernel.bin /media/
	sync
	sudo umount /media/

.PHONY: qemu
qemu: build
	qemu-system-x86_64 -fda $(BUILD_PATH)/boot.img -m 2048 \
		-d guest_errors,unimp,cpu_reset -D /dev/stderr

.PHONY: qemu-gdb-server
qemu-gdb-server: build
	qemu-system-x86_64 -fda $(BUILD_PATH)/boot.img -m 2048 \
		-d guest_errors,unimp,cpu_reset -D /dev/stderr \
		-s -S

.PHONY: qemu-gdb-client
qemu-gdb-client:
	gdb -tui -x $(GDB_SCRIPT)

.PHONY: test
test: $(BUILD_PATH)/kernel.bin

.PHONY: build
build: clean\
$(BUILD_PATH)/boot.img
	chmod  --recursive 777 $(BUILD_PATH)/

.PHONY: clean
clean:
	rm -rf $(BUILD_PATH)
