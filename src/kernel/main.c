//
// Created by qingzhixing on 24-6-30.
//

#include "lib.h"
#include "printk.h"
#include "trap.h"
#include "gate.h"

#define GDT_INDEX_TSS 8

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

	load_TR(GDT_INDEX_TSS); // 实际选择子=8*8=0x40

	set_tss64(
		0xffff800000007e00, // RSP0（内核模式栈）
		0x0,				// RSP1
		0x0,				// RSP2
		0xffff80000000a000, // IST1（双重故障栈）
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0);

	// 验证代码（添加在set_tss64之后）
	printk("TSS RSP0:%#llx\n", *(unsigned long *)(TSS64_Table + 1));
	printk("TSS IST1:%#llx\n", *(unsigned long *)(TSS64_Table + 9));

	sys_vector_init();
	int i = 1 / 0; // #DE: division by zero

	while (1)
		;
}
