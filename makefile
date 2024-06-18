BUILD_PATH = ./build
SRC_PATH = ./src

$(BUILD_PATH)/%.bin: $(SRC_PATH)/bootloader/%.nasm
	mkdir -p $(BUILD_PATH)
	nasm -f bin -o $@ $<

$(BUILD_PATH)/boot.img: $(BUILD_PATH)/boot.bin\
$(BUILD_PATH)/loader.bin
	mkdir -p $(BUILD_PATH)
	yes | bximage -q -mode=create -fd=1.44M $@
	dd if=$(BUILD_PATH)/boot.bin of=$@ bs=512 count=1 conv=notrunc
	# mount loader
	mount $@ /media/ -t vfat -o loop
	cp $(BUILD_PATH)/loader.bin /media/
	sync
	umount /media/

.PHONY:bochs
bochs: build
	bochs -q -f bochsrc.bxrc

.PHONY: test
test: $(BUILD_PATH)/boot.img

.PHONY: build
build: clean\
$(BUILD_PATH)/boot.img
	chmod -R ugo+rw ./build/*

.PHONY: clean
clean:
	rm -rf $(BUILD_PATH)