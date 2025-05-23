//
// Created by qingzhixing on 25-4-17.
//

#include "interrupt.h"
#include "gate.h"
#include "lib.h"
#include "linkage.h"
#include "memory.h"
#include "printk.h"
#include "ptrace.h"

// 中断保存现场模块
#define SAVE_ALL                                                                                                       \
	"cld;			\n\t"                                                                                                      \
	"pushq	%rax;		\n\t"                                                                                                \
	"pushq	%rax;		\n\t"                                                                                                \
	"movq	%es,	%rax;	\n\t"                                                                                             \
	"pushq	%rax;		\n\t"                                                                                                \
	"movq	%ds,	%rax;	\n\t"                                                                                             \
	"pushq	%rax;		\n\t"                                                                                                \
	"xorq	%rax,	%rax;	\n\t"                                                                                            \
	"pushq	%rbp;		\n\t"                                                                                                \
	"pushq	%rdi;		\n\t"                                                                                                \
	"pushq	%rsi;		\n\t"                                                                                                \
	"pushq	%rdx;		\n\t"                                                                                                \
	"pushq	%rcx;		\n\t"                                                                                                \
	"pushq	%rbx;		\n\t"                                                                                                \
	"pushq	%r8;		\n\t"                                                                                                 \
	"pushq	%r9;		\n\t"                                                                                                 \
	"pushq	%r10;		\n\t"                                                                                                \
	"pushq	%r11;		\n\t"                                                                                                \
	"pushq	%r12;		\n\t"                                                                                                \
	"pushq	%r13;		\n\t"                                                                                                \
	"pushq	%r14;		\n\t"                                                                                                \
	"pushq	%r15;		\n\t"                                                                                                \
	"movq	$0x10,	%rdx;	\n\t"                                                                                           \
	"movq	%rdx,	%ds;	\n\t"                                                                                             \
	"movq	%rdx,	%es;	\n\t"


#define IRQ_NAME2(nr) nr##_interrupt(void)
#define IRQ_NAME(nr) IRQ_NAME2(IRQ##nr)

#define Build_IRQ(nr)                            \
void IRQ_NAME(nr);                        \
__asm__ (    SYMBOL_NAME_STR(IRQ)#nr"_interrupt:		\n\t"    \
            "pushq	$0x00				\n\t"    \
            SAVE_ALL                    \
            "movq	%rsp,	%rdi			\n\t"    \
            "leaq	ret_from_intr(%rip),	%rax	\n\t"    \
            "pushq	%rax				\n\t"    \
            "movq	$"#nr",	%rsi			\n\t"    \
            "jmp	do_IRQ	\n\t");


Build_IRQ(0x20);

Build_IRQ(0x21);

Build_IRQ(0x22);

Build_IRQ(0x23);

Build_IRQ(0x24);

Build_IRQ(0x25);

Build_IRQ(0x26);

Build_IRQ(0x27);

Build_IRQ(0x28);

Build_IRQ(0x29);

Build_IRQ(0x2a);

Build_IRQ(0x2b);

Build_IRQ(0x2c);

Build_IRQ(0x2d);

Build_IRQ(0x2e);

Build_IRQ(0x2f);

Build_IRQ(0x30);

Build_IRQ(0x31);

Build_IRQ(0x32);

Build_IRQ(0x33);

Build_IRQ(0x34);

Build_IRQ(0x35);

Build_IRQ(0x36);

Build_IRQ(0x37);

void (*interrupt[24])(void) = {
		IRQ0x20_interrupt, IRQ0x21_interrupt, IRQ0x22_interrupt, IRQ0x23_interrupt, IRQ0x24_interrupt,
		IRQ0x25_interrupt, IRQ0x26_interrupt, IRQ0x27_interrupt, IRQ0x28_interrupt, IRQ0x29_interrupt,
		IRQ0x2a_interrupt, IRQ0x2b_interrupt, IRQ0x2c_interrupt, IRQ0x2d_interrupt, IRQ0x2e_interrupt,
		IRQ0x2f_interrupt, IRQ0x30_interrupt, IRQ0x31_interrupt, IRQ0x32_interrupt, IRQ0x33_interrupt,
		IRQ0x34_interrupt, IRQ0x35_interrupt, IRQ0x36_interrupt, IRQ0x37_interrupt,
};

void init_interrupt() {
	for (int i = 32; i < 56; i++) {
		set_intr_gate(i, 2, interrupt[i - 32]);
	}

	color_printk(RED, BLACK, "8259A init \n");

	/*
	 * 8259A initialization: https://wiki.osdev.org/8259_PIC
	 * 8259A bits defines: https://helppc.netcore2k.net/hardware/8259
	 * */
	// 主 PIC 初始化
	io_out8(0x20, 0x11); // ICW1: 边沿触发, 级联, 需要 ICW4
	io_out8(0x21, 0x20); // ICW2: 主 PIC 中断向量基址（如 0x20）
	io_out8(0x21, 0x04); // ICW3: 主 PIC 的 IRQ2 连接从 PIC
	io_out8(0x21, 0x01); // ICW4: 8086 模式

	// 从 PIC 初始化
	io_out8(0xA0, 0x11); // ICW1
	io_out8(0xA1, 0x28); // ICW2: 从 PIC 中断向量基址（如 0x28）
	io_out8(0xA1, 0x02); // ICW3: 从 PIC 连接到主 PIC 的 IRQ2
	io_out8(0xA1, 0x01); // ICW4

	// 8259A-M/S	OCW1
	io_out8(0x21, 0xfd);
	io_out8(0xa1, 0xff);

	sti();
}

void do_IRQ(struct pt_regs * regs,unsigned long nr)	//regs,nr
{
	unsigned char x;
	color_printk(RED,BLACK,"do_IRQ:%#018lx\t",nr);
	x = io_in8(0x60);
	color_printk(RED,BLACK,"key code:%#018lx\t",x);
	io_out8(0x20,0x20);
	color_printk(RED,BLACK,"<RIP:%#018lx\tRSP:%#018lx>\n",regs->rip,regs->rsp);
}
