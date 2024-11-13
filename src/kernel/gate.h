#pragma once
#ifndef YURIOS_GATE_H
#define YURIOS_GATE_H

struct desc_struct {
    // doc/kernel/Descriptor/System Segment Descriptor.png
    unsigned char x[8];     // 64 bit
};

struct gate_struct {
    // doc/kernel/IDT/IDT Gate Descriptor Structure - 64bits.png
    unsigned char x[16];    // 128 bit
};

extern struct desc_struct GDT_Table[];
extern struct gate_struct IDT_Table[];
extern unsigned int TSS64_Table[26];

/*
 * @param isr:一个表示中断栈表的索引（Interrupt Stack Table index）的参数
 *  在中断发生时，处理程序可以通过这个索引指定使用哪个栈来处理该中断。
 *  通常，在多任务或异常处理环境中，为了避免栈溢出或干扰，
 *  使用不同的栈来处理特定的中断是很常见的做法。
 * */
#define _set_gate(gate_selector_addr, attr, ist, code_addr)                                                 \
    do                                                                                                      \
    {                                                                                                       \
        unsigned long __d0, __d1;                                                                           \
        __asm__ __volatile__("movw	%%dx,	%%ax	\n\t"                                                         \
                             "andq	$0x7,	%%rcx	\n\t"                                                        \
                             "addq	%4,	%%rcx	\n\t"                                                          \
                             "shlq	$32,	%%rcx	\n\t"                                                         \
                             "addq	%%rcx,	%%rax	\n\t"                                                       \
                             "xorq	%%rcx,	%%rcx	\n\t"                                                       \
                             "movl	%%edx,	%%ecx	\n\t"                                                       \
                             "shrq	$16,	%%rcx	\n\t"                                                         \
                             "shlq	$48,	%%rcx	\n\t"                                                         \
                             "addq	%%rcx,	%%rax	\n\t"                                                       \
                             "movq	%%rax,	%0	\n\t"                                                          \
                             "shrq	$32,	%%rdx	\n\t"                                                         \
                             "movq	%%rdx,	%1	\n\t"                                                          \
                             : "=m"(*((unsigned long *)(gate_selector_addr))),                              \
                               "=m"(*(1 + (unsigned long *)(gate_selector_addr))), "=&a"(__d0), "=&d"(__d1) \
                             : "i"(attr << 8),                                                              \
                               "3"((unsigned long *)(code_addr)), "2"(0x8 << 16), "c"(ist)                  \
                             : "memory");                                                                   \
    } while (0)

inline void set_intr_gate(unsigned int n, unsigned char ist, void *addr) {
    _set_gate(IDT_Table + n, 0x8e, ist, addr);  // P, DPL=0, TYPE = E
}

inline void set_trap_gate(unsigned int n, unsigned char ist, void *addr) {
    _set_gate(IDT_Table + n, 0x8f, ist, addr);  // P, DPL=0, TYPE = F
}

inline void set_system_gate(unsigned int n, unsigned char ist, void *addr) {
    _set_gate(IDT_Table + n, 0xef, ist, addr);  // P, DPL=3, TYPE = F
}

#endif // !YURIOS_GATE_H
