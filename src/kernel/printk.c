//
// Created by qingzhixing on 24-9-8.
//

#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include "printk.h"
#include "lib.h"
#include "linkage.h"

/*
    将当前指针指向的字符串转换为整数并同时移动指针,遇到非数字字符停止转换,返回整数值
*/
int skip_atoi(const char **s)
{
    int i = 0;

    while (is_digit(**s))
        i = i * 10 + *((*s)++) - '0';
    return i;
}

// Function to convert a number to a string in a specified base with formatting options
// TODO:无法正常转换
static char *number(char *str, long num, int base, int size, int precision, int type)
{
    char c, sign, tmp[50];
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
    else
        sign = (type & PLUS) ? '+' : ((type & SPACE) ? ' ' : 0);
    if (sign)
        size--;
    if (type & SPECIAL)
        if (base == 16)
            size -= 2;
        else if (base == 8)
            size--;
    i = 0;
    if (num == 0)
        tmp[i++] = '0';
    else
        while (num != 0)
            tmp[i++] = digits[do_div(num, base)];
    if (i > precision)
        precision = i;
    size -= precision;
    if (!(type & (ZEROPAD + LEFT)))
        while (size-- > 0)
            *str++ = ' ';
    if (sign)
        *str++ = sign;
    if (type & SPECIAL)
        if (base == 8)
            *str++ = '0';
        else if (base == 16)
        {
            *str++ = '0';
            *str++ = digits[33];
        }
    if (!(type & LEFT))
        while (size-- > 0)
            *str++ = c;

    while (i < precision--)
        *str++ = '0';
    while (i-- > 0)
        *str++ = tmp[i];
    while (size-- > 0)
        *str++ = ' ';
    return str;
}

/*
    格式化字符串并返回格式化后的字符串长度
*/
int vsprintf(char *buf, const char *fmt, va_list args)
{
    char *str, *s;
    int flags;
    int field_width;
    int precision;
    int len, i;

    int qualifier; /* 'h', 'l', 'L' or 'Z' for integer fields */

    for (str = buf; *fmt; fmt++)
    {

        if (*fmt != '%')
        {
            *str++ = *fmt;
            continue;
        }
        flags = 0;
    repeat:
        fmt++;
        switch (*fmt)
        {
        case '-':
            flags |= LEFT;
            goto repeat;
        case '+':
            flags |= PLUS;
            goto repeat;
        case ' ':
            flags |= SPACE;
            goto repeat;
        case '#':
            flags |= SPECIAL;
            goto repeat;
        case '0':
            flags |= ZEROPAD;
            goto repeat;
        }

        /* get field width */

        field_width = -1;
        if (is_digit(*fmt))
            field_width = skip_atoi(&fmt);
        else if (*fmt == '*')
        {
            fmt++;
            field_width = va_arg(args, int);
            if (field_width < 0)
            {
                field_width = -field_width;
                flags |= LEFT;
            }
        }

        /* get the precision */

        precision = -1;
        if (*fmt == '.')
        {
            fmt++;
            if (is_digit(*fmt))
                precision = skip_atoi(&fmt);
            else if (*fmt == '*')
            {
                fmt++;
                precision = va_arg(args, int);
            }
            if (precision < 0)
                precision = 0;
        }

        qualifier = -1;
        if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z')
        {
            qualifier = *fmt;
            fmt++;
        }

        switch (*fmt)
        {
        case 'c':

            if (!(flags & LEFT))
                while (--field_width > 0)
                    *str++ = ' ';
            *str++ = (unsigned char)va_arg(args, int);
            while (--field_width > 0)
                *str++ = ' ';
            break;

        case 's':

            s = va_arg(args, char *);
            if (!s)
                s = '\0';
            len = strlen(s);
            if (precision < 0)
                precision = len;
            else if (len > precision)
                len = precision;

            if (!(flags & LEFT))
                while (len < field_width--)
                    *str++ = ' ';
            for (i = 0; i < len; i++)
                *str++ = *s++;
            while (len < field_width--)
                *str++ = ' ';
            break;

        case 'o':

            if (qualifier == 'l')
                str = number(str, va_arg(args, unsigned long), 8, field_width, precision, flags);
            else
                str = number(str, va_arg(args, unsigned int), 8, field_width, precision, flags);
            break;

        case 'p':

            if (field_width == -1)
            {
                field_width = 2 * sizeof(void *);
                flags |= ZEROPAD;
            }

            str = number(str, (unsigned long)va_arg(args, void *), 16, field_width, precision, flags);
            break;

        case 'x':

            flags |= SMALL;

        case 'X':

            if (qualifier == 'l')
                str = number(str, va_arg(args, unsigned long), 16, field_width, precision, flags);
            else
                str = number(str, va_arg(args, unsigned int), 16, field_width, precision, flags);
            break;

        case 'd':
        case 'i':

            flags |= SIGN;
        case 'u':

            if (qualifier == 'l')
                str = number(str, va_arg(args, unsigned long), 10, field_width, precision, flags);
            else
                str = number(str, va_arg(args, unsigned int), 10, field_width, precision, flags);
            break;

        case 'n':

            if (qualifier == 'l')
            {
                long *ip = va_arg(args, long *);
                *ip = (str - buf);
            }
            else
            {
                int *ip = va_arg(args, int *);
                *ip = (str - buf);
            }
            break;

        case '%':

            *str++ = '%';
            break;

        default:

            *str++ = '%';
            if (*fmt)
                *str++ = *fmt;
            else
                fmt--;
            break;
        }
    }
    *str = '\0';
    return str - buf;
}

int sprintf(char *buffer, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int len = vsprintf(buffer, format, args);
    va_end(args);
    return len;
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
    int line; // 用来控制制表符处理的逻辑，以实现正确的制表显示效果

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
