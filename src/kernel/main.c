//
// Created by qingzhixing on 24-6-30.
//

#include "lib.h"
#include "printk.h"
#include "trap.h"
#include "gate.h"
#include "memory.h"

#define GDT_INDEX_TSS 8

struct Global_Memory_descriptor memory_management_struct = {.e820={0},.e820_length=0};

void printk_color_test() {
	/*
		#define WHITE 0x00ffffff  // 白
		#define BLACK 0x00000000  // 黑
		#define RED 0x00ff0000    // 红
		#define ORANGE 0x00ff8000 // 橙
		#define YELLOW 0x00ffff00 // 黄
		#define GREEN 0x0000ff00  // 绿
		#define BLUE 0x000000ff   // 蓝
		#define INDIGO 0x0000ffff // 靛
		#define PURPLE 0x008000ff // 紫
	 * */
	color_printk(WHITE, WHITE, " ");
	color_printk(WHITE, BLACK, " ");
	color_printk(WHITE, RED, " ");
	color_printk(WHITE, ORANGE, " ");
	color_printk(WHITE, YELLOW, " ");
	color_printk(WHITE, GREEN, " ");
	color_printk(WHITE, BLUE, " ");
	color_printk(WHITE, INDIGO, " ");
	color_printk(WHITE, PURPLE, " ");
	color_printk(WHITE, BLACK, "#");
	color_printk(BLACK, WHITE, "#");
	color_printk(ORANGE, BLACK, "#");
	color_printk(YELLOW, BLACK, "#");
	color_printk(GREEN, BLACK, "#");
	color_printk(BLUE, BLACK, "#");
	color_printk(INDIGO, BLACK, "#");
	color_printk(PURPLE, BLACK, "#");
	color_printk(WHITE, BLACK, "\n");
}

void init() {

	// 显示模式: 1440x900x32
	// 模式号: 0x180
	// 显存地址: 0xffff800000a00000
	unsigned int *frame_buffer = (unsigned int *) (VGA_BASE_ADDR);
	for (int i = 0; i < VGA_SCREEN_WIDTH * 3; i++) {
		*(((char *) frame_buffer) + 0) = 0x00; // Blue
		*(((char *) frame_buffer) + 1) = 0x00; // Green
		*(((char *) frame_buffer) + 2) = 0xff; // Red
		*(((char *) frame_buffer) + 3) = 0x00; // 保留
		frame_buffer += 1;
	}

	init_printk();
	printk_color_test();
	init_memory();
}

void Start_Kernel(void) {
	init();

	color_printk(BLACK, WHITE, "%s", "Hello, YuriOS!\n");

	load_TR(GDT_INDEX_TSS); // 实际选择子=8*8=0x40

	set_tss64(
			0xffff800000007c00,
			0xffff800000007c00,
			0xffff800000007c00,
			0xffff800000007c00,
			0xffff800000007c00,
			0xffff800000007c00,
			0xffff800000007c00,
			0xffff800000007c00,
			0xffff800000007c00,
			0xffff800000007c00
	);

	// 验证代码（添加在set_tss64之后）
	color_printk(BLACK, WHITE, "TSS RSP0:%#llx\n", *(unsigned long *) (TSS64_Table + 1));
	color_printk(BLACK, WHITE, "TSS IST1:%#llx\n", *(unsigned long *) (TSS64_Table + 9));


	sys_vector_init();

//	int i;
//	i = 1 / 0; // #DE: division by zero
//	i = *(int *) 0xffff80000aa00000;

	while (1);
}
