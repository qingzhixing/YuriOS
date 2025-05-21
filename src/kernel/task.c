//
// Created by qingzhixing on 25-5-20.
//

#include "task.h"

inline void __switch_to(struct task_struct* prev,struct task_struct* next){
	// init_tss[0]: cpu_0 的 tss
	init_tss[0].rsp0 = next->thread->rsp0;

	set_tss64(init_tss[0].rsp0, init_tss[0].rsp1, init_tss[0].rsp2, init_tss[0].ist1, init_tss[0].ist2, init_tss[0].ist3, init_tss[0].ist4, init_tss[0].ist5, init_tss[0].ist6, init_tss[0].ist7);

	// 保存当前进程FS和GS段寄存器
	__asm__ __volatile__("movq	%%fs,	%0 \n\t":"=a"(prev->thread->fs));
	__asm__ __volatile__("movq	%%gs,	%0 \n\t":"=a"(prev->thread->gs));

	// 将next进程保存的FS和GS还原
	__asm__ __volatile__("movq	%0,	%%fs \n\t"::"a"(next->thread->fs));
	__asm__ __volatile__("movq	%0,	%%gs \n\t"::"a"(next->thread->gs));

	color_printk(WHITE,BLACK,"prev->thread->rsp0:%#018lx\n",prev->thread->rsp0);
	color_printk(WHITE,BLACK,"next->thread->rsp0:%#018lx\n",next->thread->rsp0);
}

/*
 * 初始化第一个进程，再调用kernel_thread为系统创建出一个新进程，随后借助switch_to模块执行进程实现切换
 */
void task_init(){
	struct task_struct* init_task = NULL;

	// 初始化第一个进程的内存空间结构体
	init_mm.pgd = (pml4t_t*)Global_CR3;
}
