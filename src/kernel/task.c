//
// Created by qingzhixing on 25-5-20.
//

#include "task.h"
#include "gate.h"
#include "lib.h"
#include "linkage.h"
#include "memory.h"
#include "printk.h"
#include "ptrace.h"

static unsigned long init(unsigned long arg) {
	color_printk(RED, BLACK, "init task is running, arg:%#018lx\n", arg);
	return 1;
}

/* @brief 继续完成进程切换的后续工作,由switch_to调用
 * @param prev 上一个进程的task_struct
 * @param next 下一个进程的task_struct
 */
inline void __switch_to(struct task_struct *prev, struct task_struct *next) {
	// init_tss[0]: cpu_0 的 tss
	// 将内核层栈基地址设置到TSS结构体对应的成员变量中
	init_tss[0].rsp0 = next->thread->rsp0;

	set_tss64(init_tss[0].rsp0, init_tss[0].rsp1, init_tss[0].rsp2, init_tss[0].ist1, init_tss[0].ist2,
			  init_tss[0].ist3, init_tss[0].ist4, init_tss[0].ist5, init_tss[0].ist6, init_tss[0].ist7);

	// 保存当前进程FS和GS段寄存器
	__asm__ __volatile__("movq	%%fs,	%0 \n\t" : "=a"(prev->thread->fs));
	__asm__ __volatile__("movq	%%gs,	%0 \n\t" : "=a"(prev->thread->gs));

	// 将next进程保存的FS和GS还原
	__asm__ __volatile__("movq	%0,	%%fs \n\t" ::"a"(next->thread->fs));
	__asm__ __volatile__("movq	%0,	%%gs \n\t" ::"a"(next->thread->gs));

	color_printk(WHITE, BLACK, "prev->thread->rsp0:%#018lx\n", prev->thread->rsp0);
	color_printk(WHITE, BLACK, "next->thread->rsp0:%#018lx\n", next->thread->rsp0);

	// 执行汇编代码RET后会跳转至next进程
}

unsigned long do_exit(unsigned long code) {
	color_printk(RED, BLACK, "exit task is running, arg:%#018lx\n", code);
	while (1)
		;
}

// @brief 进程执行前的引导程序
extern void kernel_thread_func(void);
__asm__("kernel_thread_func:	\n\t"

		// 还原进程执行现场
		"	popq	%r15	\n\t"
		"	popq	%r14	\n\t"
		"	popq	%r13	\n\t"
		"	popq	%r12	\n\t"
		"	popq	%r11	\n\t"
		"	popq	%r10	\n\t"
		"	popq	%r9	\n\t"
		"	popq	%r8	\n\t"
		"	popq	%rbx	\n\t"
		"	popq	%rcx	\n\t"
		"	popq	%rdx	\n\t"
		"	popq	%rsi	\n\t"
		"	popq	%rdi	\n\t"
		"	popq	%rbp	\n\t"
		"	popq	%rax	\n\t"
		"	movq	%rax,	%ds	\n\t"
		"	popq	%rax		\n\t"
		"	movq	%rax,	%es	\n\t"
		"	popq	%rax		\n\t"
		"	addq	$0x38,	%rsp	\n\t"

		// 运行进程
		"	movq	%rdx,	%rdi	\n\t"
		"	callq	*%rbx		\n\t"

		// 退出进程
		"	movq	%rax,	%rdi	\n\t"
		"	callq	do_exit		\n\t");


unsigned long do_fork(struct pt_regs *regs, unsigned long clone_flags, unsigned long stack_start,
					  unsigned long stack_size) {}

/*
 * @brief: 为操作系统创建进程
 * @param fn 进程入口函数
 * @param arg 进程入口函数的参数
 * @param flags 进程创建标志位
 * */
static int kernel_thread(ThreadFunction fn, unsigned long arg, unsigned long flags) {
	struct pt_regs regs;

	memset(&regs, 0, sizeof(regs));

	//// 准备SYSEXIT汇编命令的参数
	regs.rbx = (unsigned long) fn; // 程序入口地址
	regs.rcx = (unsigned long) arg; // 程序入口参数

	regs.ds = KERNEL_DS;
	regs.es = KERNEL_DS;
	regs.cs = KERNEL_CS;
	regs.ss = KERNEL_DS;
	regs.rflags = (1 << 9);
	regs.rip = (unsigned long) kernel_thread_func; // 进程执行前的引导程序

	return do_fork(&regs, flags, 0, 0);
}

/*
 * @brief: 初始化第一个进程，再调用kernel_thread为系统创建出一个新进程，
 * 随后借助switch_to模块执行进程实现切换
 */
void task_init() {
	struct task_struct *init_task = NULL;

	//// 补完系统第一个进程控制结构体中未赋值的成员变量
	// 初始化第一个进程的内存空间结构体
	init_mm.pgd = (pml4t_t *) Global_CR3;
	init_mm.start_code = memory_management_struct.start_code;
	init_mm.end_code = memory_management_struct.end_code;
	init_mm.start_data = (unsigned long) &_data;
	init_mm.end_data = memory_management_struct.end_data;
	init_mm.start_rodata = (unsigned long) &_rodata;
	init_mm.end_rodata = (unsigned long) &_erodata;
	init_mm.start_brk = 0;
	init_mm.end_brk = memory_management_struct.end_brk;

	// init_thread , init_tss
	set_tss64(init_thread.rsp0, init_tss[0].rsp1, init_tss[0].rsp2, init_tss[0].ist1, init_tss[0].ist2,
			  init_tss[0].ist3, init_tss[0].ist4, init_tss[0].ist5, init_tss[0].ist6, init_tss[0].ist7);

	init_tss[0].rsp0 = init_thread.rsp0;

	kernel_thread(init, 10, CLONE_FS | CLONE_FILES | CLONE_SIGNAL);
	init_task_union.task.state = TASK_RUNNING;
	init_task = container_of(list_next(&(current->list)), task_struct, list);
	switch_to(current, init_task);
}
