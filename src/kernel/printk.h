//
// Created by qingzhixing on 24-9-8.
//

#ifndef YURIOS_PRINTK_H
#define YURIOS_PRINTK_H

#include <stdarg.h>
#include "font.h"
#include "linkage.h"

#define PRINTK_BUFFER_SIZE 4096

#define VGA_SCREEN_WIDTH 1440
#define VGA_SCREEN_HEIGHT 900
#define VGA_X_RESOLUTION VGA_SCREEN_WIDTH
#define VGA_Y_RESOLUTION VGA_SCREEN_HEIGHT
#define VGA_COLOR_DEPTH 32
#define VGA_BASE_ADDR 0xffff800000a00000

#define CHAR_SIZE_X 8
#define CHAR_SIZE_Y 16

#define ZEROPAD 0x01 /* pad with zero */
#define SIGN 0x02    /* unsigned/signed long */
#define PLUS 0x04    /* show plus */
#define SPACE 0x08   /* space if plus */
#define LEFT 0x10    /* left justified */
#define SPECIAL 0x20 /* 0x */
#define SMALL 0x40   /* use 'abcdef' instead of 'ABCDEF' */

#define is_digit(c) ((c) >= '0' && (c) <= '9')
#define do_div(n, base) ({ \
int __res; \
__asm__("divq %%rcx":"=a" (n),"=d" (__res):"0" (n),"1" (0),"c" (base)); \
__res; })

#define WHITE 0x00ffffff  // 白
#define BLACK 0x00000000  // 黑
#define RED 0x00ff0000    // 红
#define ORANGE 0x00ff8000 // 橙
#define YELLOW 0x00ffff00 // 黄
#define GREEN 0x0000ff00  // 绿
#define BLUE 0x000000ff   // 蓝
#define INDIGO 0x0000ffff // 靛
#define PURPLE 0x008000ff // 紫

extern unsigned char font_ascii[256][16];

char printk_buffer[PRINTK_BUFFER_SIZE] = {0};

struct Cursor_State
{
    int x_offset;
    int y_offset;
} cursor_global_state;

struct VGA_State
{
    int x_resolution; // x方向分辨率
    int y_resolution; // y方向分辨率

    int x_char_size; // x方向字符宽度
    int y_char_size; // y方向字符高度

    unsigned int *FrameBuffer_addr;
    unsigned long FrameBufferSize;
} vga_global_state;

int skip_atoi(const char **s);

static char *number(char *str, long num, int base, int size, int precision, int type);

int color_printk(unsigned int front_color, unsigned int back_color, const char *format, ...);

int vsprintf(char *buffer, const char *format, va_list args);

int sprintf(char *buffer, const char *format, ...);

void putchar(
    unsigned int *frame_buffer,
    int XResolution,
    int x_pixel_offset, int y_pixel_offset,
    unsigned int front_color,
    unsigned int back_color,
    unsigned char font_char);

void putchar_str(unsigned int *frame_buffer,
                 int XResolution,
                 unsigned int front_color,
                 unsigned int back_color,
                 const char *str);

#endif // YURIOS_PRINTK_H
