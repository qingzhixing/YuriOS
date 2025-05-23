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

extern void ret_system_call(void);

void user_level_function()
{
	color_printk(RED,BLACK,"user_level_function task is running\n");
	while(1);
}


unsigned long do_execve(struct pt_regs * regs)
{
	regs->rdx = 0x800000;	//RIP
	regs->rcx = 0xa00000;	//RSP
	regs->rax = 1;
	regs->ds = 0;
	regs->es = 0;
	color_printk(RED,BLACK,"do_execve task is running\n");

	memcpy(user_level_function,(void *)0x800000,1024);

	return 0;
}

unsigned long init(unsigned long arg)
{
	struct pt_regs *regs;
	
	color_printk(RED,BLACK,"init task is running,arg:%#018lx\n",arg);

	current->thread->rip = (unsigned long)ret_system_call;
	current->thread->rsp = (unsigned long)current + STACK_SIZE - sizeof(struct pt_regs);
	regs = (struct pt_regs *)current->thread->rsp;

	__asm__	__volatile__	(	"movq	%1,	%%rsp	\n\t"
						 "pushq	%2		\n\t"
						 "jmp	do_execve	\n\t"
						 ::"D"(regs),"m"(current->thread->rsp),"m"(current->thread->rip):"memory");

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


/*
 * @brief: 创建进程控制结构体并完成进程运行前的初始化工作
 * @param regs 进程执行前的寄存器环境
 * @param clone_flags 进程创建标志位
 * @param stack_start 进程栈的起始地址
 * */
unsigned long do_fork(struct pt_regs *regs, unsigned long clone_flags, unsigned long stack_start,
					  unsigned long stack_size) {
	task_struct *tsk = NULL;
	thread_struct *thd = NULL;
	Page *p = NULL;

	// 分配一个物理页，用于保存新进程的task_struct
	color_printk(WHITE, BLACK, "alloc_pages,bitmap:%#018lx\n", *memory_management_struct.bits_map);
	p = alloc_pages(ZONE_NORMAL, 1, PG_PTable_Maped | PG_Active | PG_Kernel);
	color_printk(WHITE, BLACK, "alloc_pages,bitmap:%#018lx\n", *memory_management_struct.bits_map);

	// 为新的task_struct分配物理页，并将其映射到虚拟地址空间中
	tsk = (task_struct *) Phy_To_Virt(p->PHY_address);
	color_printk(WHITE, BLACK, "struct task_struct address:%#018lx\n", (unsigned long) tsk);

	memset(tsk, 0, sizeof(*tsk));
	*tsk = *current; // TODO: why?

	// 将新进程的task_struct添加到进程就绪队列中
	list_init(&tsk->list);
	list_add_to_before(&init_task_union.task.list, &tsk->list);
	tsk->pid++;
	tsk->state = TASK_UNINTERRUPTIBLE;

	// 分配thread_struct结构空间
	thd = (thread_struct *) (tsk + 1); // 放在task_struct后面
	tsk->thread = thd;

	// 伪造进程执行现场，复制到目标进程的内核层栈顶处
	memcpy(regs, (void *) ((unsigned long) tsk + STACK_SIZE - sizeof(pt_regs)), sizeof(pt_regs));

	thd->rsp0 = (unsigned long) tsk + STACK_SIZE;
	thd->rip = regs->rip;
	thd->rsp = (unsigned long) tsk + STACK_SIZE - sizeof(pt_regs);

	// 运行在用户层
	if (!(tsk->flags & PF_KTHREAD)) {
		thd->rip = regs->rip = (unsigned long) ret_system_call;
	}

	tsk->state = TASK_RUNNING;

	return 0;
}

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
	struct task_struct *p = NULL;

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

	init_mm.start_stack = _stack_start; // 与init_thread.rsp0相同

	wrmsr(0x174,KERNEL_CS);
	
	// init_thread , init_tss
	set_tss64(init_thread.rsp0, init_tss[0].rsp1, init_tss[0].rsp2, init_tss[0].ist1, init_tss[0].ist2,
			  init_tss[0].ist3, init_tss[0].ist4, init_tss[0].ist5, init_tss[0].ist6, init_tss[0].ist7);

	init_tss[0].rsp0 = init_thread.rsp0;

	list_init(&init_task_union.task.list);

	// 创建init进程
	kernel_thread(init, 10, CLONE_FS | CLONE_FILES | CLONE_SIGNAL);
	init_task_union.task.state = TASK_RUNNING;
	p = container_of(list_next(&current->list), task_struct, list);
	switch_to(current, p);
}
