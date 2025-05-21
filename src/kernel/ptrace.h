/***************************************************
 *		版权声明
 *
 *	本操作系统名为：MINE
 *	该操作系统未经授权不得以盈利或非盈利为目的进行开发，
 *	只允许个人学习以及公开交流使用
 *
 *	代码最终所有权及解释权归田宇所有；
 *
 *	本模块作者：	田宇
 *	EMail:		345538255@qq.com
 *
 *
 ***************************************************/

#ifndef __PTRACE_H__

#define __PTRACE_H__

// @brief 现场数据结构体 (必须与汇编代码严格对齐)
typedef struct pt_regs {
	/* 软件保存部分 (通过SAVE_ALL) */
	unsigned long r15;
	unsigned long r14;
	unsigned long r13;
	unsigned long r12;
	unsigned long r11;
	unsigned long r10;
	unsigned long r9;
	unsigned long r8;
	unsigned long rbx;
	unsigned long rcx;
	unsigned long rdx;
	unsigned long rsi;
	unsigned long rdi;
	unsigned long rbp;

	/* 段寄存器和临时保存 */
	unsigned long ds;
	unsigned long es;
	unsigned long rax;

	/* 处理函数和错误码 */
	void (*func)(void); // 处理函数指针
	unsigned long errcode;

	/* 硬件自动保存部分 */
	unsigned long rip;
	unsigned long cs;
	unsigned long rflags;
	unsigned long rsp;
	unsigned long ss;
} __attribute__((packed)) pt_regs;

#endif
