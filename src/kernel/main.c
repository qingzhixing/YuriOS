//
// Created by qingzhixing on 24-6-30.
//

#include "lib.h"
#include "printk.h"

void Start_Kernel(void)
{
    // 显示模式: 1440x900x32
    // 模式号: 0x180
    // 显存地址: 0xffff800000a00000
    unsigned int *fram_buffer = (unsigned int *)(VGA_BASE_ADDR);
    for (int i = 0; i < VGA_SCREEN_WIDTH * 20; i++)
    {
        *(((char *)fram_buffer) + 0) = 0x00; // Blue
        *(((char *)fram_buffer) + 1) = 0x00; // Green
        *(((char *)fram_buffer) + 2) = 0xff; // Red
        *(((char *)fram_buffer) + 3) = 0x00; // 保留
        fram_buffer += 1;
    }
    for (int i = 0; i < VGA_SCREEN_WIDTH * 20; i++)
    {
        *(((char *)fram_buffer) + 0) = 0x00; // Blue
        *(((char *)fram_buffer) + 1) = 0xff; // Green
        *(((char *)fram_buffer) + 2) = 0x00; // Red
        *(((char *)fram_buffer) + 3) = 0x00; // 保留
        fram_buffer += 1;
    }

    for (int i = 0; i < VGA_SCREEN_WIDTH * 20; i++)
    {
        *(((char *)fram_buffer) + 0) = 0xff; // Blue
        *(((char *)fram_buffer) + 1) = 0x00; // Green
        *(((char *)fram_buffer) + 2) = 0x00; // Red
        *(((char *)fram_buffer) + 3) = 0x00; // 保留
        fram_buffer += 1;
    }

    for (int i = 0; i < VGA_SCREEN_WIDTH * 20; i++)
    {
        *(((char *)fram_buffer) + 0) = 0xff; // Blue
        *(((char *)fram_buffer) + 1) = 0xff; // Green
        *(((char *)fram_buffer) + 2) = 0x00; // Red
        *(((char *)fram_buffer) + 3) = 0x00; // 保留
        fram_buffer += 1;
    }

    for (int i = 0; i < VGA_SCREEN_WIDTH * 20; i++)
    {
        *(((char *)fram_buffer) + 0) = 0xff; // Blue
        *(((char *)fram_buffer) + 1) = 0xff; // Green
        *(((char *)fram_buffer) + 2) = 0xff; // Red
        *(((char *)fram_buffer) + 3) = 0x00; // 保留
        fram_buffer += 1;
    }

    vga_global_state.x_resolution = VGA_X_RESOLUTION;
    vga_global_state.y_resolution = VGA_Y_RESOLUTION;

    cursor_global_state.x_offset = 0;
    cursor_global_state.y_offset = 0;

    vga_global_state.x_char_size = CHAR_SIZE_X;
    vga_global_state.y_char_size = CHAR_SIZE_Y;

    vga_global_state.FrameBuffer_addr = (unsigned int *)(VGA_BASE_ADDR);
    vga_global_state.FrameBufferSize = VGA_SCREEN_WIDTH * VGA_SCREEN_HEIGHT * 4;

    putchar(vga_global_state.FrameBuffer_addr, vga_global_state.x_resolution, 30, 30, WHITE, BLACK, 'Y');
    putchar_str(vga_global_state.FrameBuffer_addr, vga_global_state.x_resolution, WHITE, BLACK, "Welcome to YuriOS !"); // TODO:printk无法使用,会使屏幕全部变为背景色并陷入死循环
    putchar(vga_global_state.FrameBuffer_addr, vga_global_state.x_resolution, 50, 50, WHITE, BLACK, '0');

    while (1)
        ;
}