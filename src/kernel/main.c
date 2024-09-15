//
// Created by qingzhixing on 24-6-30.
//

#include "lib.h"
#include "printk.h"

void init()
{
    init_printk();
}

void Start_Kernel(void)
{
    init();

    // 显示模式: 1440x900x32
    // 模式号: 0x180
    // 显存地址: 0xffff800000a00000
    unsigned int *frame_buffer = (unsigned int *)(VGA_BASE_ADDR);
    for (int i = 0; i < VGA_SCREEN_WIDTH * 3; i++)
    {
        *(((char *)frame_buffer) + 0) = 0x00; // Blue
        *(((char *)frame_buffer) + 1) = 0x00; // Green
        *(((char *)frame_buffer) + 2) = 0xff; // Red
        *(((char *)frame_buffer) + 3) = 0x00; // 保留
        frame_buffer += 1;
    }

    color_printk(BLACK, WHITE, "%s", "Hello, YuriOS!\n");
    while (1)
        ;
}