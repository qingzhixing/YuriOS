BUILD_PATH = ./build
SRC_PATH = ./src

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
	bochs -q -f bochsrc.bxrc

.PHONY: test
test: $(BUILD_PATH)/kernel.bin

.PHONY: build
build: clean\
$(BUILD_PATH)/boot.img
	chmod -R ugo+rw $(BUILD_PATH)/*

.PHONY: clean
clean:
	rm -rf $(BUILD_PATH)