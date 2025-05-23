//
// Created by qingzhixing on 25-4-19.
//

#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H

#include "cpu.h"
#include "lib.h"
#include "memory.h"
#include "ptrace.h"

#define KERNEL_CS (0x08)
#define KERNEL_DS (0x10)
#define USER_CS (0x28)
#define USER_DS (0x30)

#define CLONE_FS (1 << 0)
#define CLONE_FILES (1 << 1)
#define CLONE_SIGNAL (1 << 2)

// @note: stack size 32K
#define STACK_SIZE 32768

//// task_struct::state
// 正在运行
#define TASK_RUNNING (1 << 0)
// 可中断
#define TASK_INTERRUPTIBLE (1 << 1)
// 不可中断
#define TASK_UNINTERRUPTIBLE (1 << 2)
// 僵死
#define TASK_ZOMBIE (1 << 3)
// 停止
#define TASK_STOPPED (1 << 4)

// task_struct::flags
// 运行在内核层
#define PF_KTHREAD (1 << 0)

extern char _text;
extern char _etext;
extern char _data;
extern char _edata;
extern char _rodata;
extern char _erodata;
extern char _bss;
extern char _ebss;
extern char _end;

extern unsigned long _stack_start;

extern void ret_from_intr();

typedef unsigned long (*ThreadFunction)(unsigned long);

// @brief 描述进程页表结构和各程序段信息
typedef struct mm_struct {
	// 内存页表指针
	pml4t_t *pgd; // page table point

	// 代码段空间
	unsigned long start_code, end_code;

	// 数据段空间
	unsigned long start_data, end_data;

	// 只读数据段空间
	unsigned long start_rodata, end_rodata;

	// 动态内存分布区(堆空间)
	unsigned long start_brk, end_brk;

	// 用户层栈基地址
	unsigned long start_stack;
} mm_struct;

// @brief 进程调度的执行现场
typedef struct thread_struct {
	// 内核层栈基地址
	unsigned long rsp0; // in tss

	// 内核层代码指针
	unsigned long rip;
	unsigned long rsp;

	unsigned long fs;
	unsigned long gs;

	unsigned long cr2;

	// 产生异常的异常号
	unsigned long trap_nr;

	// 异常的错误码
	unsigned long error_code;
} thread_struct;

// @brief PCB结构体(Process Control Block 进程控制结构体)，记录进程的资源使用情况
typedef struct task_struct {
	struct List list; // 双向链表
	volatile long state; // 进程状态: 运行，停止，可中断等
	unsigned long flags; // 进程标志：进程，线程，内核线程等

	struct mm_struct *mm; // 内存空间分布结构体
	struct thread_struct *thread; // 进程切换保留的状态信息

	unsigned long addr_limit; // 进程地址空间范围
	/*0x0000,0000,0000,0000 - 0x0000,7fff,ffff,ffff user*/
	/*0xffff,8000,0000,0000 - 0xffff,ffff,ffff,ffff kernel*/

	long pid; // 进程ID号

	long counter; // 进程可用时间片

	long signal; // 进程持有的信号

	long priority; // 进程优先级
} task_struct;

/* @brief 进程的内核层栈空间结构，将task_struct与进程内核栈空间连续到了一起
 * @note 大小暂时设定为为32KB，需要对齐32KB
 * */
typedef union task_union {
	struct task_struct task;
	unsigned long stack[STACK_SIZE / sizeof(unsigned long)];
} __attribute__((aligned(8))) task_union; // 8 Bytes align

// @brief tss结构，每个CPU一个tss结构体
typedef struct tss_struct {
	unsigned int reserved0;
	unsigned long rsp0;
	unsigned long rsp1;
	unsigned long rsp2;
	unsigned long reserved1;
	unsigned long ist1;
	unsigned long ist2;
	unsigned long ist3;
	unsigned long ist4;
	unsigned long ist5;
	unsigned long ist6;
	unsigned long ist7;
	unsigned long reserved2;
	unsigned short reserved3;
	unsigned short iomapbaseaddr;
} __attribute__((packed)) tss_struct;

// init_task 的 mm_struct
struct mm_struct init_mm;
// init_task 的 thread_struct
struct thread_struct init_thread;

#define INIT_TASK(tsk)                                                                                                 \
	{.state = TASK_UNINTERRUPTIBLE,                                                                                    \
	 .flags = PF_KTHREAD,                                                                                              \
	 .mm = &init_mm,                                                                                                   \
	 .thread = &init_thread,                                                                                           \
	 .addr_limit = 0xffff800000000000,                                                                                 \
	 .pid = 0,                                                                                                         \
	 .counter = 1,                                                                                                     \
	 .signal = 0,                                                                                                      \
	 .priority = 0}

union task_union init_task_union __attribute__((__section__(".data.init_task"))) = {INIT_TASK(init_task_union.task)};

struct task_struct *init_task[NR_CPUS] = {&init_task_union.task, 0};

struct mm_struct init_mm = {0};

struct thread_struct init_thread = {
		.rsp0 = (unsigned long) (init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),
		.rsp = (unsigned long) (init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),
		.fs = KERNEL_DS,
		.gs = KERNEL_DS,
		.cr2 = 0,
		.trap_nr = 0,
		.error_code = 0};


#define INIT_TSS                                                                                                       \
	{.reserved0 = 0,                                                                                                   \
	 .rsp0 = (unsigned long) (init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),                             \
	 .rsp1 = (unsigned long) (init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),                             \
	 .rsp2 = (unsigned long) (init_task_union.stack + STACK_SIZE / sizeof(unsigned long)),                             \
	 .reserved1 = 0,                                                                                                   \
	 .ist1 = 0xffff800000007c00,                                                                                       \
	 .ist2 = 0xffff800000007c00,                                                                                       \
	 .ist3 = 0xffff800000007c00,                                                                                       \
	 .ist4 = 0xffff800000007c00,                                                                                       \
	 .ist5 = 0xffff800000007c00,                                                                                       \
	 .ist6 = 0xffff800000007c00,                                                                                       \
	 .ist7 = 0xffff800000007c00,                                                                                       \
	 .reserved2 = 0,                                                                                                   \
	 .reserved3 = 0,                                                                                                   \
	 .iomapbaseaddr = 0}

// 每一个CPU都有一个tss结构体
struct tss_struct init_tss[NR_CPUS] = {[0 ... NR_CPUS - 1] = INIT_TSS};

task_struct *get_current();

inline task_struct *get_current() {
	task_struct *_current = NULL;
	__asm__ __volatile__("andq %%rsp,%0	\n\t" : "=r"(_current) : "0"(~32767UL));
	return _current;
}

#define current get_current()

#define GET_CURRENT                                                                                                    \
	"movq	%rsp,	%rbx	\n\t"                                                                                             \
	"andq	$-32768,%rbx	\n\t"

/* @brief 继续完成进程切换的后续工作,由switch_to调用
 * @param prev 上一个进程的task_struct
 * @param next 下一个进程的task_struct
 */
void __switch_to(struct task_struct *prev, struct task_struct *next);

/* @brief: 调用并将RDI和RSI寄存器作为参数传递到__switch_to函数
 * */
#define switch_to(prev, next)                                                                                          \
	do {                                                                                                               \
		__asm__ __volatile__("pushq	%%rbp	\n\t"                                                                        \
							 "pushq	%%rax	\n\t"                                                                        \
							 "movq	%%rsp,	%0	\n\t" /*保存当RSP寄存器的值到prev->thread->rsp中*/                     \
							 "movq	%2,	%%rsp	\n\t" /*恢复next进程的RSP寄存器的值到RSP寄存器中*/                     \
							 "leaq	1f(%%rip),	%%rax	\n\t" /*指定切换回prev进程时RIP寄存器的值（默认在Label 1:处)*/ \
							 "movq	%%rax,	%1	\n\t" /*将next进程执行现场的RIP寄存器压入next进程的内核层栈空间*/      \
							 "pushq	%3		\n\t"                                                                          \
							 "jmp	__switch_to	\n\t" /*执行__switch_to函数,在__switch_to函数中的RET会跳转至next进程*/ \
							 "1:	\n\t"                                                                                 \
							 "popq	%%rax	\n\t"                                                                         \
							 "popq	%%rbp	\n\t"                                                                         \
							 : "=m"(prev->thread->rsp), "=m"(prev->thread->rip)                                        \
							 : "m"(next->thread->rsp), "m"(next->thread->rip), "D"(prev), "S"(next)                    \
							 : "memory");                                                                              \
	} while (0)


unsigned long do_fork(struct pt_regs *regs, unsigned long clone_flags, unsigned long stack_start,
					  unsigned long stack_size);

void task_init();

#endif // KERNEL_TASK_H
