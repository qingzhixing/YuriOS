BUILD_PATH = ./build
SRC_PATH = ./src

$(BUILD_PATH)/%.bin: $(SRC_PATH)/bootloader/%.nasm
	mkdir -p $(BUILD_PATH)
	nasm -f bin -o $@ $<

$(BUILD_PATH)/boot.img: $(BUILD_PATH)/boot.bin
	mkdir -p $(BUILD_PATH)
	yes | bximage -q -mode=create -fd=1.44M $@
	dd if=$(BUILD_PATH)/boot.bin of=$@ bs=512 count=1 conv=notrunc

.PHONY: bochs
bochs: $(BUILD_PATH)/boot.img
	bochs -q -f bochsrc.bxrc

.PHONY: test
test: $(BUILD_PATH)/boot.img

.PHONY: clean
clean:
	rm -rf $(BUILD_PATH)