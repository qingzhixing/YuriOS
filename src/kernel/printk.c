//
// Created by qingzhixing on 24-9-8.
//

#include <stdarg.h>
#include "printk.h"
#include "lib.h"
#include "linkage.h"

int vsprintf(char *buffer, const char *format, va_list args) {
    // TODO: implement vsprintf
    return 0;
}

void putchar(
        unsigned int *frame_buffer,
        int XResolution,
        int x_pixel_offset, int y_pixel_offset,
        unsigned int front_color,
        unsigned int back_color,
        unsigned char font_char
) {
    unsigned int *pixel_addr = NULL;
    unsigned char *font_row_ptr = font_ascii[font_char];    // 获取当前行的像素填充数据
    int font_pixel_index = 0;

    for (int row = 0; row < CHAR_SIZE_Y; row++) {
        pixel_addr = frame_buffer + (y_pixel_offset + row) + x_pixel_offset;
        font_pixel_index = 0x100;
        for (int col = 0; col < CHAR_SIZE_X; col++) {
            font_pixel_index >>= 1;
            if (*font_row_ptr & font_pixel_index) {
                *pixel_addr = front_color;
            } else {
                *pixel_addr = back_color;
            }
        }
        font_row_ptr++;
    }
}

int color_printk(unsigned int front_color, unsigned int back_color, const char *format, ...) {
    int formatted_len = 0;
    int line;

    // format the message

    char buffer[PRINTK_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    formatted_len = vsprintf(buffer, format, args);
    va_end(args);

    // print the message
    for (int str_index = 0; str_index < formatted_len || line; str_index++) {
        // add \n \b \t
        if (line > 0) {
            str_index--;
            goto Label_tab;
        }
        // \n
        if ((unsigned char) *(buffer + str_index) == '\n') {
            cursor_global_state.x_offset++;
            cursor_global_state.y_offset = 0;
        }
            // \b
        else if ((unsigned char) *(buffer + str_index) == '\b') {
            cursor_global_state.x_offset--;
            if (cursor_global_state.x_offset < 0) {
                // 回退到上一行最后一个字符的位置
                cursor_global_state.x_offset = vga_global_state.x_resolution / vga_global_state.x_charSize - 1;
                cursor_global_state.y_offset--;

                if (cursor_global_state.y_offset < 0) {
                    // 回退到屏幕最下方
                    cursor_global_state.y_offset = vga_global_state.y_resolution / vga_global_state.y_charSize - 1;
                }
            }
            putchar(
                    vga_global_state.FrameBuffer_addr,
                    vga_global_state.x_resolution,
                    cursor_global_state.x_offset * vga_global_state.x_charSize,
                    cursor_global_state.y_offset * vga_global_state.y_charSize,
                    front_color, back_color,
                    ' '
            );
        }
            // \t
        else if ((unsigned char) *(buffer + str_index) == '\t') {
            line = ((cursor_global_state.x_offset + 8) & (8 - 1)) - cursor_global_state.x_offset;
            Label_tab:
            line--;
            putchar(
                    vga_global_state.FrameBuffer_addr,
                    vga_global_state.x_resolution,
                    cursor_global_state.x_offset * vga_global_state.x_charSize,
                    cursor_global_state.y_offset * vga_global_state.y_charSize,
                    front_color, back_color,
                    ' '
            );
            cursor_global_state.x_offset++;
        }   // 普通字符
        else {
            putchar(
                    vga_global_state.FrameBuffer_addr,
                    vga_global_state.x_resolution,
                    cursor_global_state.x_offset * vga_global_state.x_charSize,
                    cursor_global_state.y_offset * vga_global_state.y_charSize,
                    front_color, back_color,
                    *(buffer + str_index)
            );
            cursor_global_state.x_offset++;
        }

        if (cursor_global_state.x_offset >= (vga_global_state.x_resolution / vga_global_state.x_charSize)) {
            cursor_global_state.x_offset = 0;
            cursor_global_state.y_offset++;
        }
        if (cursor_global_state.y_offset >= (vga_global_state.y_resolution / vga_global_state.y_charSize)) {
            cursor_global_state.y_offset = 0;
        }
    }
    return 0;
}



