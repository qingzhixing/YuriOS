//
// Created by qingzhixing on 24-9-8.
//

#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include "printk.h"
#include "lib.h"

static inline int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

/*
    将当前指针指向的字符串转换为整数并同时移动指针,遇到非数字字符停止转换,返回整数值
*/
int skip_atoi(const char **s)
{
    int i = 0;
    while (is_digit(**s))
    {
        i = i * 10 + *((*s)++) - '0';
    }
    return i;
}

static inline int do_div(long *n, int base)
{
    int __res;
    __asm__("divq %%rcx"
            : "=a"(*n), "=d"(__res)
            : "0"(*n), "1"(0), "c"(base));
    return __res;
}

// Function to convert a number to a string in a specified base with formatting options
static char *number(char *str, long num, int base, int size, int precision, int type)
{
    char c, sign, tmp[64];
    const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i;

    if (type & SMALL)
        digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    if (type & LEFT)
        type &= ~ZEROPAD;
    if (base < 2 || base > 36)
        return 0;
    c = (type & ZEROPAD) ? '0' : ' ';
    sign = 0;

    if (type & SIGN && num < 0)
    {
        sign = '-';
        num = -num;
    }
    else if (type & PLUS)
    {
        sign = '+';
    }
    else if (type & SPACE)
    {
        sign = ' ';
    }

    if (sign)
        size--;

    if (type & SPECIAL)
    {
        if (base == 16)
            size -= 2;
        else if (base == 8)
            size--;
    }

    i = 0;
    if (num == 0)
    {
        tmp[i++] = '0';
    }
    else
    {
        while (num != 0)
        {
            tmp[i++] = digits[do_div(&num, base)];
        }
    }

    if (i > precision)
        precision = i;
    size -= precision;

    if (!(type & (ZEROPAD + LEFT)))
    {
        while (size-- > 0)
        {
            *str++ = ' ';
        }
    }

    if (sign)
    {
        *str++ = sign;
        size--;
    }

    if (type & SPECIAL)
    {
        if (base == 8)
        {
            *str++ = '0';
            size--;
        }
        else if (base == 16)
        {
            *str++ = '0';
            *str++ = digits[33];
            size -= 2;
        }
    }

    if (!(type & LEFT))
    {
        while (size-- > 0)
        {
            *str++ = c;
        }
    }

    if (precision < 0)
    {
        precision = 0; // 确保精度不会为负
    }

    while (i < precision)
    {
        *str++ = '0';
        precision--;
    }

    return str;
}

static inline void pad_left(char **str, int field_width)
{
    while (--field_width > 0)
    {
        **str = ' ';
        (*str)++;
    }
}

static inline void pad_right(char **str, int field_width)
{
    while (--field_width > 0)
    {
        **str = ' ';
        (*str)++;
    }
}

/*
    格式化字符串并返回格式化后的字符串长度
*/
int vsprintf(char *buf, const char *format, va_list args)
{
    char *formatted_str = buf;
    int flags;
    int field_width;
    int precision; // 精度 'h', 'l', 'L', 'Z' for integer fields
    int qualifier;

    while (*format)
    {
        if (*format != '%')
        {
            *formatted_str++ = *format++;
            continue;
        }

        format++;
        flags = 0;
        while (true)
        {
            switch (*format)
            {
            case '-':
                flags |= LEFT;
                break;
            case '+':
                flags |= PLUS;
                break;
            case ' ':
                flags |= SPACE;
                break;
            case '#':
                flags |= SPECIAL;
                break;
            case '0':
                flags |= ZEROPAD;
                break;
            default:
                goto parse_width;
            }
            format++;
        }

    parse_width:
        field_width = -1;
        if (is_digit(*format))
        {
            field_width = skip_atoi(&format);
        }
        else if (*format == '*')
        {
            format++;
            field_width = va_arg(args, int);
            if (field_width < 0)
            {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        precision = -1;
        if (*format == '.')
        {
            format++;
            if (is_digit(*format))
            {
                precision = skip_atoi(&format);
            }
            else if (*format == '*')
            {
                format++;
                precision = va_arg(args, int);
            }
            if (precision < 0)
            {
                precision = 0;
            }
        }

        qualifier = -1;
        if (*format == 'h' || *format == 'l' || *format == 'L' || *format == 'Z')
        {
            qualifier = *format;
            format++;
        }

        switch (*format)
        {
        case 'c':
            if (!(flags & LEFT))
            {
                pad_left(&formatted_str, field_width);
            }
            *formatted_str++ = (unsigned char)va_arg(args, int);
            if (flags & LEFT)
            {
                pad_right(&formatted_str, field_width);
            }
            break;

        case 's':
        {
            char *s = va_arg(args, char *);
            if (!s)
            {
                s = "";
            }
            int len = strlen(s);
            if (precision < 0)
            {
                precision = len;
            }
            else if (len > precision)
            {
                len = precision;
            }

            if (!(flags & LEFT))
            {
                pad_left(&formatted_str, field_width - len);
            }
            for (int i = 0; i < len; i++)
            {
                *formatted_str++ = *s++;
            }
            if (flags & LEFT)
            {
                pad_right(&formatted_str, field_width - len);
            }
            break;
        }

        case 'o':
            if (qualifier == 'l')
            {
                formatted_str = number(formatted_str, va_arg(args, unsigned long), 8, field_width, precision, flags);
            }
            else
            {
                formatted_str = number(formatted_str, va_arg(args, unsigned int), 8, field_width, precision, flags);
            }
            break;

        case 'p':
            if (field_width == -1)
            {
                field_width = 2 * sizeof(void *);
                flags |= ZEROPAD;
            }
            formatted_str = number(formatted_str, (unsigned long)va_arg(args, void *), 16, field_width, precision, flags);
            break;

        case 'x':
            flags |= SMALL;
        case 'X':
            if (qualifier == 'l')
            {
                formatted_str = number(formatted_str, va_arg(args, unsigned long), 16, field_width, precision, flags);
            }
            else
            {
                formatted_str = number(formatted_str, va_arg(args, unsigned int), 16, field_width, precision, flags);
            }
            break;

        case 'd':
        case 'i':
            flags |= SIGN;
        case 'u':
            if (qualifier == 'l')
            {
                formatted_str = number(formatted_str, va_arg(args, unsigned long), 10, field_width, precision, flags);
            }
            else
            {
                formatted_str = number(formatted_str, va_arg(args, unsigned int), 10, field_width, precision, flags);
            }
            break;

        case 'n':
            if (qualifier == 'l')
            {
                long *ip = va_arg(args, long *);
                *ip = (formatted_str - buf);
            }
            else
            {
                int *ip = va_arg(args, int *);
                *ip = (formatted_str - buf);
            }
            break;

        case '%':
            *formatted_str++ = '%';
            break;

        default:
            *formatted_str++ = '%';
            if (*format)
            {
                *formatted_str++ = *format;
            }
            else
            {
                format--;
            }
            break;
        }
        format++;
    }

    *formatted_str = '\0';
    return formatted_str - buf;
}

void putchar(
    unsigned int *frame_buffer,
    int XResolution,
    int x_pixel_offset, int y_pixel_offset,
    unsigned int front_color,
    unsigned int back_color,
    unsigned char font_char)
{
    unsigned int *pixel_addr = NULL;
    unsigned char *font_row_ptr = font_ascii[font_char]; // 获取当前行的像素填充数据
    int font_pixel_index;

    // 枚举当前字符的每一行
    for (int row = 0; row < CHAR_SIZE_Y; row++)
    {
        // 计算当前行的像素地址
        pixel_addr = frame_buffer + XResolution * (y_pixel_offset + row) + x_pixel_offset;
        font_pixel_index = 0x100; // 左移8位，用于判断当前像素是否需要填充

        // 填充当前行的像素
        for (int col = 0; col < CHAR_SIZE_X; col++)
        {
            font_pixel_index >>= 1;
            if ((*font_row_ptr) & font_pixel_index)
            {
                (*pixel_addr) = front_color;
            }
            else
            {
                // 只在字符的背景部分填充背景色
                (*pixel_addr) = back_color;
            }
            pixel_addr++;
        }
        // 指向下一行的像素数据
        font_row_ptr++;
    }
}

void putchar_str(unsigned int *frame_buffer,
                 int XResolution,
                 unsigned int front_color,
                 unsigned int back_color,
                 const char *str)
{
    cursor_global_state.x_offset = 0;
    cursor_global_state.y_offset = 0;

    while (*str != '\0')
    {
        putchar(frame_buffer, XResolution, cursor_global_state.x_offset * CHAR_SIZE_X, cursor_global_state.y_offset * CHAR_SIZE_Y, front_color, back_color, *str++);
        cursor_global_state.x_offset++;
        if (cursor_global_state.x_offset * CHAR_SIZE_X >= XResolution)
        {
            cursor_global_state.x_offset = 0;
            cursor_global_state.y_offset++;
        }
    }
}

// 返回格式化之后字符串的长度
int color_printk(unsigned int front_color, unsigned int back_color, const char *format, ...)
{
    int formatted_len;
    int line;

    // format the message
    va_list args;
    va_start(args, format);
    formatted_len = vsprintf(printk_buffer, format, args);
    va_end(args);

    // print the message
    for (int str_index = 0; str_index < formatted_len || line; str_index++)
    {
        // add \n \b \t
        if (line > 0)
        {
            str_index--;
            goto Label_tab;
        }
        // \n
        if ((unsigned char)*(printk_buffer + str_index) == '\n')
        {
            cursor_global_state.y_offset++;
            cursor_global_state.x_offset = 0;
        }
        // \b
        else if ((unsigned char)*(printk_buffer + str_index) == '\b')
        {
            cursor_global_state.x_offset--;
            if (cursor_global_state.x_offset < 0)
            {
                // 回退到上一行最后一个字符的位置
                cursor_global_state.x_offset = (vga_global_state.x_resolution / vga_global_state.x_char_size - 1) * vga_global_state.x_char_size;
                cursor_global_state.y_offset--;

                if (cursor_global_state.y_offset < 0)
                {
                    // 回退到屏幕最下方
                    cursor_global_state.y_offset = (vga_global_state.y_resolution / vga_global_state.y_char_size - 1) * vga_global_state.y_char_size;
                }
            }
            putchar(
                vga_global_state.FrameBuffer_addr,
                vga_global_state.x_resolution,
                cursor_global_state.x_offset * vga_global_state.x_char_size,
                cursor_global_state.y_offset * vga_global_state.y_char_size,
                front_color, back_color,
                ' ');
        }
        // \t
        else if ((unsigned char)*(printk_buffer + str_index) == '\t')
        {
            line = ((cursor_global_state.x_offset + 8) & (8 - 1)) - cursor_global_state.x_offset;
        Label_tab:
            line--;
            putchar(
                vga_global_state.FrameBuffer_addr,
                vga_global_state.x_resolution,
                cursor_global_state.x_offset * vga_global_state.x_char_size,
                cursor_global_state.y_offset * vga_global_state.y_char_size,
                front_color, back_color,
                ' ');
            cursor_global_state.x_offset++;
        } // 普通字符
        else
        {
            putchar(
                vga_global_state.FrameBuffer_addr,
                vga_global_state.x_resolution,
                cursor_global_state.x_offset * vga_global_state.x_char_size,
                cursor_global_state.y_offset * vga_global_state.y_char_size,
                front_color, back_color,
                *(printk_buffer + str_index));
            cursor_global_state.x_offset++;
        }

        if (cursor_global_state.x_offset >= (vga_global_state.x_resolution / vga_global_state.x_char_size))
        {
            cursor_global_state.x_offset = 0;
            cursor_global_state.y_offset++;
        }
        if (cursor_global_state.y_offset >= (vga_global_state.y_resolution / vga_global_state.y_char_size))
        {
            cursor_global_state.y_offset = 0;
        }
    }
    return formatted_len;
}
