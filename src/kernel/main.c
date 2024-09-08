//
// Created by qingzhixing on 24-6-30.
//

#include "lib.h"
#include "printk.h"

void Start_Kernel(void) {
    // 显示模式: 1440x900x32
    // 模式号: 0x180
    // 显存地址: 0xffff800000a00000
    unsigned int *fram_buffer = (unsigned int *) (VGA_BASE);
    for (int i = 0; i < VGA_WIDTH * 20; i++) {
        *(((char *) fram_buffer) + 0) = 0x00;   // Blue
        *(((char *) fram_buffer) + 1) = 0x00;   // Green
        *(((char *) fram_buffer) + 2) = 0xff;   // Red
        *(((char *) fram_buffer) + 3) = 0x00;   // 保留
        fram_buffer += 1;
    }
    for (int i = 0; i < VGA_WIDTH * 20; i++) {
        *(((char *) fram_buffer) + 0) = 0x00;   // Blue
        *(((char *) fram_buffer) + 1) = 0xff;   // Green
        *(((char *) fram_buffer) + 2) = 0x00;   // Red
        *(((char *) fram_buffer) + 3) = 0x00;   // 保留
        fram_buffer += 1;
    }

    for (int i = 0; i < VGA_WIDTH * 20; i++) {
        *(((char *) fram_buffer) + 0) = 0xff;   // Blue
        *(((char *) fram_buffer) + 1) = 0x00;   // Green
        *(((char *) fram_buffer) + 2) = 0x00;   // Red
        *(((char *) fram_buffer) + 3) = 0x00;   // 保留
        fram_buffer += 1;
    }

    for (int i = 0; i < VGA_WIDTH * 20; i++) {
        *(((char *) fram_buffer) + 0) = 0xff;   // Blue
        *(((char *) fram_buffer) + 1) = 0xff;   // Green
        *(((char *) fram_buffer) + 2) = 0x00;   // Red
        *(((char *) fram_buffer) + 3) = 0x00;   // 保留
        fram_buffer += 1;
    }

    for (int i = 0; i < VGA_WIDTH * 20; i++) {
        *(((char *) fram_buffer) + 0) = 0xff;   // Blue
        *(((char *) fram_buffer) + 1) = 0xff;   // Green
        *(((char *) fram_buffer) + 2) = 0xff;   // Red
        *(((char *) fram_buffer) + 3) = 0x00;   // 保留
        fram_buffer += 1;
    }

    vga_global_state.x_resolution = VGA_WIDTH;
    vga_global_state.y_resolution = VGA_HEIGHT;

    cursor_global_state.x_offset = 0;
    cursor_global_state.y_offset = 0;

    vga_global_state.x_charSize = CHAR_SIZE_X;
    vga_global_state.y_charSize = CHAR_SIZE_Y;

    vga_global_state.FrameBuffer_addr = (unsigned int *) (VGA_BASE);
    vga_global_state.FrameBufferSize = VGA_WIDTH * VGA_HEIGHT * 4;

    // TODO:通过编译但是无法正常显示
    color_printk(YELLOW, BLACK, "1");
//    color_printk(GREEN, BLUE, "This is a test message!\n");

    while (1);
}