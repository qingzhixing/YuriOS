BUILD_PATH = ./build
SRC_PATH = ./src
BOCHS_PATH = ./build-tools/bochs
BOCHS_BIN = $(BOCHS_PATH)/bochs-2.6.11
BOCHS_GDB_BIN = $(BOCHS_PATH)/gdb-bochs-2.6.11

$(BUILD_PATH)/%.bin: $(SRC_PATH)/bootloader/%.nasm
	mkdir -p $(BUILD_PATH)
	nasm -I $(SRC_PATH)/bootloader -f bin -o $@ $<

$(BUILD_PATH)/kernel.bin:
	make -C $(SRC_PATH)/kernel all

$(BUILD_PATH)/boot.img: $(BUILD_PATH)/boot.bin\
$(BUILD_PATH)/loader.bin\
$(BUILD_PATH)/kernel.bin
	mkdir -p $(BUILD_PATH)
	yes | bximage -q -mode=create -fd=1.44M $@
	dd if=$(BUILD_PATH)/boot.bin of=$@ bs=512 count=1 conv=notrunc
	# mount loader
	mount $@ /media/ -t vfat -o loop
	cp $(BUILD_PATH)/loader.bin /media/
	cp $(BUILD_PATH)/kernel.bin /media/
	sync
	umount /media/

.PHONY:bochs
bochs: build
	$(BOCHS_BIN) -q -f $(BOCHS_PATH)/bochsrc.bxrc

.PHONY: bochs-gdb
bochs-gdb: build
	$(BOCHS_GDB_BIN) -q -f $(BOCHS_PATH)/bochsrc-gdb.bxrc

.PHONY: test
test: $(BUILD_PATH)/kernel.bin

.PHONY: build
build: clean\
$(BUILD_PATH)/boot.img
	chmod  --recursive 777 $(BUILD_PATH)/

.PHONY: clean
clean:
	rm -rf $(BUILD_PATH)