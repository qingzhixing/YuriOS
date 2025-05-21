//
// Created by qingzhixing on 24-11-13.
//

#ifndef YURIOS_TRAP_H
#define YURIOS_TRAP_H

#include "lib.h"
#include "linkage.h"
#include "printk.h"
#include "ptrace.h"

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
