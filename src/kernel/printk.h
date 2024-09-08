//
// Created by qingzhixing on 24-9-8.
//

#ifndef YURIOS_PRINTK_H
#define YURIOS_PRINTK_H

#include <stdarg.h>
#include "font.h"
#include "linkage.h"

struct Cursor_State {
    int x_offset;
    int y_offset;
} cursor_global_state;

struct VGA_State {
    int x_resolution;        // x方向分辨率
    int y_resolution;        // y方向分辨率

    int x_charSize;          // x方向字符宽度
    int y_charSize;          // y方向字符高度

    unsigned int *FrameBuffer_addr;
    unsigned long FrameBufferSize;
} vga_global_state;


int color_printk(unsigned int front_color, unsigned int back_color, const char *format, ...);

int vsprintf(char *buffer, const char *format, va_list args);

void putchar(
        unsigned int *frame_buffer,
        int XResolution,
        int x_pixel_offset, int y_pixel_offset,
        unsigned int front_color,
        unsigned int back_color,
        unsigned char font_char
);

#define PRINTK_BUFFER_SIZE 1024

#define VGA_WIDTH 1440
#define VGA_HEIGHT 900
#define VGA_DEPTH 32
#define VGA_BASE 0xffff800000a00000

#define CHAR_SIZE_X 8
#define CHAR_SIZE_Y 16
#endif //YURIOS_PRINTK_H
