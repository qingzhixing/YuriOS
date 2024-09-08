//
// Created by qingzhixing on 24-9-8.
//

#include <stdarg.h>
#include <stdbool.h>
#include "printk.h"
#include "lib.h"
#include "linkage.h"

static inline int is_digit(char c) {
    return c >= '0' && c <= '9';
}

static inline int skip_atoi(const char **s) {
    int i = 0;
    while (is_digit(**s)) {
        i = i * 10 + *((*s)++) - '0';
    }
    return i;
}

#define do_div(n,base) ({ \
int __res; \
__asm__("divq %%rcx":"=a" (n),"=d" (__res):"0" (n),"1" (0),"c" (base)); \
__res; })

// Function to convert a number to a string in a specified base with formatting options
static char * number(char * str, long num, int base, int size, int precision, int type)
{
    char c,sign,tmp[50];
    const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i;

    if (type&SMALL) digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    if (type&LEFT) type &= ~ZEROPAD;
    if (base < 2 || base > 36)
        return 0;
    c = (type & ZEROPAD) ? '0' : ' ';
    sign = 0;

    if (type & SIGN && num < 0) {
        sign = '-';
        num = -num;
    } else if (type & PLUS) {
        sign = '+';
    } else if (type & SPACE) {
        sign = ' ';
    }

    if (sign) size--;

    if (type & SPECIAL) {
        if (base == 16) size -= 2;
        else if (base == 8) size--;
    }

    i = 0;
    if (num == 0) {
        tmp[i++] = '0';
    } else {
        while (num != 0) {
            tmp[i++] = digits[do_div(num, base)];
        }
    }

    if (i > precision) precision = i;
    size -= precision;

    if (!(type & (ZEROPAD + LEFT))) {
        while (size-- > 0) {
            *str++ = ' ';
        }
    }

    if (sign) {
        *str++ = sign;
        size--;
    }

    if (type & SPECIAL) {
        if (base == 8) {
            *str++ = '0';
            size--;
        } else if (base == 16) {
            *str++ = '0';
            *str++ = digits[33];
            size -= 2;
        }
    }

    if (!(type & LEFT)) {
        while (size-- > 0) {
            *str++ = c;
        }
    }

    if (precision < 0) {
        precision = 0; // 确保精度不会为负
    }

    while (i < precision) {
        *str++ = '0';
        precision--;
    }

    return str;
}


static inline void pad_left(char **str, int field_width) {
    while (--field_width > 0) {
        **str = ' ';
        (*str)++;
    }
}

static inline void pad_right(char **str, int field_width) {
    while (--field_width > 0) {
        **str = ' ';
        (*str)++;
    }
}

int vsprintf(char *buf, const char *fmt, va_list args) {
    char *str = buf;
    int flags, field_width, precision, qualifier;

    while (*fmt) {
        if (*fmt != '%') {
            *str++ = *fmt++;
            continue;
        }

        fmt++;
        flags = 0;
        while (true) {
            switch (*fmt) {
                case '-': flags |= LEFT; break;
                case '+': flags |= PLUS; break;
                case ' ': flags |= SPACE; break;
                case '#': flags |= SPECIAL; break;
                case '0': flags |= ZEROPAD; break;
                default: goto parse_width;
            }
            fmt++;
        }

        parse_width:
        field_width = -1;
        if (is_digit(*fmt)) {
            field_width = skip_atoi(&fmt);
        } else if (*fmt == '*') {
            fmt++;
            field_width = va_arg(args, int);
            if (field_width < 0) {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        precision = -1;
        if (*fmt == '.') {
            fmt++;
            if (is_digit(*fmt)) {
                precision = skip_atoi(&fmt);
            } else if (*fmt == '*') {
                fmt++;
                precision = va_arg(args, int);
            }
            if (precision < 0) {
                precision = 0;
            }
        }

        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z') {
            qualifier = *fmt;
            fmt++;
        }

        switch (*fmt) {
            case 'c':
                if (!(flags & LEFT)) {
                    pad_left(&str, field_width);
                }
                *str++ = (unsigned char)va_arg(args, int);
                if (flags & LEFT) {
                    pad_right(&str, field_width);
                }
                break;

            case 's': {
                char *s = va_arg(args, char *);
                if (!s) {
                    s = "";
                }
                int len = strlen(s);
                if (precision < 0) {
                    precision = len;
                } else if (len > precision) {
                    len = precision;
                }

                if (!(flags & LEFT)) {
                    pad_left(&str, field_width - len);
                }
                for (int i = 0; i < len; i++) {
                    *str++ = *s++;
                }
                if (flags & LEFT) {
                    pad_right(&str, field_width - len);
                }
                break;
            }

            case 'o':
                if (qualifier == 'l') {
                    str = number(str, va_arg(args, unsigned long), 8, field_width, precision, flags);
                } else {
                    str = number(str, va_arg(args, unsigned int), 8, field_width, precision, flags);
                }
                break;

            case 'p':
                if (field_width == -1) {
                    field_width = 2 * sizeof(void *);
                    flags |= ZEROPAD;
                }
                str = number(str, (unsigned long)va_arg(args, void *), 16, field_width, precision, flags);
                break;

            case 'x':
                flags |= SMALL;
            case 'X':
                if (qualifier == 'l') {
                    str = number(str, va_arg(args, unsigned long), 16, field_width, precision, flags);
                } else {
                    str = number(str, va_arg(args, unsigned int), 16, field_width, precision, flags);
                }
                break;

            case 'd':
            case 'i':
                flags |= SIGN;
            case 'u':
                if (qualifier == 'l') {
                    str = number(str, va_arg(args, unsigned long), 10, field_width, precision, flags);
                } else {
                    str = number(str, va_arg(args, unsigned int), 10, field_width, precision, flags);
                }
                break;

            case 'n':
                if (qualifier == 'l') {
                    long *ip = va_arg(args, long *);
                    *ip = (str - buf);
                } else {
                    int *ip = va_arg(args, int *);
                    *ip = (str - buf);
                }
                break;

            case '%':
                *str++ = '%';
                break;

            default:
                *str++ = '%';
                if (*fmt) {
                    *str++ = *fmt;
                } else {
                    fmt--;
                }
                break;
        }
        fmt++;
    }

    *str = '\0';
    return str - buf;
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
    int font_pixel_index;

    for (int row = 0; row < CHAR_SIZE_Y; row++) {
        pixel_addr = frame_buffer + XResolution*(y_pixel_offset + row) + x_pixel_offset;
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

// 返回格式化之后字符串的长度
int color_printk(unsigned int front_color, unsigned int back_color, const char *format, ...) {
    int formatted_len;
    int line;

    // format the message
    va_list args;
    va_start(args, format);
    formatted_len = vsprintf(printk_buffer, format, args);
    va_end(args);

    // print the message
    for (int str_index = 0; str_index < formatted_len || line; str_index++) {
        // add \n \b \t
        if (line > 0) {
            str_index--;
            goto Label_tab;
        }
        // \n
        if ((unsigned char) *(printk_buffer + str_index) == '\n') {
            cursor_global_state.x_offset++;
            cursor_global_state.y_offset = 0;
        }
            // \b
        else if ((unsigned char) *(printk_buffer + str_index) == '\b') {
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
        else if ((unsigned char) *(printk_buffer + str_index) == '\t') {
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
                    *(printk_buffer + str_index)
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
    return formatted_len;
}



