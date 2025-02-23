//
// Created by qingzhixing on 24-11-13.
//

#ifndef YURIOS_TRAP_H
#define YURIOS_TRAP_H

#include "linkage.h"
#include "printk.h"
#include "lib.h"

// pt_regs结构定义 (必须与汇编代码严格对齐)
struct pt_regs
{
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
} __attribute__((packed));

#define INTERRUPT_ATTR __attribute__((interrupt, no_caller_saved_registers))

void debug_print_stack(unsigned long rsp);

void divide_error();

void debug();

void nmi();

void int3();

void overflow();

void bounds();

void undefined_opcode();

void dev_not_available();

void double_fault();

void coprocessor_segment_overrun();

void invalid_TSS();

void segment_not_present();

void stack_segment_fault();

void general_protection();

void page_fault();

void x87_FPU_error();

void alignment_check();

void machine_check();

void SIMD_exception();

void virtualization_exception();

void sys_vector_init();

#endif // YURIOS_TRAP_H
