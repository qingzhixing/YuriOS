//
// Created by qingzhixing on 25-4-17.
//

#ifndef KERNEL_INTERRUPT_H
#define KERNEL_INTERRUPT_H

#include "linkage.h"

void init_interrupt();

void do_IRQ(unsigned long regs, unsigned long nr);

#endif // KERNEL_INTERRUPT_H
