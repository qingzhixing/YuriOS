//
// Created by qingzhixing on 25-4-17.
//

#ifndef KERNEL_INTERRUPT_H
#define KERNEL_INTERRUPT_H

#include "linkage.h"
#include "ptrace.h"

void init_interrupt();

void do_IRQ(struct pt_regs * regs,unsigned long nr);

#endif // KERNEL_INTERRUPT_H
