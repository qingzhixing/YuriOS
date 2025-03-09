#include "gate.h"
#include "trap.h"

void debug_print_stack(unsigned long rsp) {
	color_printk(RED, WHITE, "Stack Dump @ %#lx:\n", rsp);
	for (int i = 0; i < 8; i++) {
		color_printk(BLUE, BLACK, "[%+03d] %#018lx\n",
					 i * 8, *(unsigned long *) (rsp + i * 8));
	}
}

void do_divide_error(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_divide_error(0),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp,
				 rip);
	while (1);
}

void do_debug(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_debug(1),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, rip);
	while (1);
}

void do_nmi(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_nmi(2),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, rip);
	while (1);
}

void do_int3(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_int3(3),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, rip);
	while (1);
}

void do_overflow(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_overflow(4),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, rip);
	while (1);
}

void do_bounds(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_bounds(5),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp, rip);
	while (1);
}

void do_undefined_opcode(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_undefined_opcode(6),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp,
				 rip);
	while (1);
}

void do_dev_not_available(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_dev_not_available(7),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp,
				 rip);
	while (1);
}

void do_double_fault(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_double_fault(8),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp,
				 rip);
	while (1);
}

void do_coprocessor_segment_overrun(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_coprocessor_segment_overrun(9),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n",
				 error_code, rsp, rip);
	while (1);
}

void do_invalid_TSS(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_invalid_TSS(10),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp,
				 rip);

	if (error_code & 0x01)        // bit EXT
		color_printk(RED, BLACK,
					 "The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");

	if (error_code & 0x02)        // bit IDT
		color_printk(RED, BLACK, "Refers to a gate descriptor in the IDT;\n");
	else
		color_printk(RED, BLACK, "Refers to a descriptor in the GDT or the current LDT;\n");

	if ((error_code & 0x02) == 0)
		if (error_code & 0x04)    // bit TI
			color_printk(RED, BLACK, "Refers to a segment or gate descriptor in the LDT;\n");
		else
			color_printk(RED, BLACK, "Refers to a descriptor in the current GDT;\n");

	color_printk(RED, BLACK, "Segment Selector Index:%#010x\n", error_code & 0xfff8);

	while (1);
}

void do_segment_not_present(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_segment_not_present(11),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp,
				 rip);

	if (error_code & 0x01)
		color_printk(RED, BLACK,
					 "The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");

	if (error_code & 0x02)
		color_printk(RED, BLACK, "Refers to a gate descriptor in the IDT;\n");
	else
		color_printk(RED, BLACK, "Refers to a descriptor in the GDT or the current LDT;\n");

	if ((error_code & 0x02) == 0)
		if (error_code & 0x04)
			color_printk(RED, BLACK, "Refers to a segment or gate descriptor in the LDT;\n");
		else
			color_printk(RED, BLACK, "Refers to a descriptor in the current GDT;\n");

	color_printk(RED, BLACK, "Segment Selector Index:%#010x\n", error_code & 0xfff8);

	while (1);
}

void do_stack_segment_fault(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_stack_segment_fault(12),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp,
				 rip);

	if (error_code & 0x01)
		color_printk(RED, BLACK,
					 "The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");

	if (error_code & 0x02)
		color_printk(RED, BLACK, "Refers to a gate descriptor in the IDT;\n");
	else
		color_printk(RED, BLACK, "Refers to a descriptor in the GDT or the current LDT;\n");

	if ((error_code & 0x02) == 0)
		if (error_code & 0x04)
			color_printk(RED, BLACK, "Refers to a segment or gate descriptor in the LDT;\n");
		else
			color_printk(RED, BLACK, "Refers to a descriptor in the current GDT;\n");

	color_printk(RED, BLACK, "Segment Selector Index:%#010x\n", error_code & 0xfff8);

	while (1);
}

void do_general_protection(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_general_protection(13),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp,
				 rip);

	if (error_code & 0x01)
		color_printk(RED, BLACK,
					 "The exception occurred during delivery of an event external to the program,such as an interrupt or an earlier exception.\n");

	if (error_code & 0x02)
		color_printk(RED, BLACK, "Refers to a gate descriptor in the IDT;\n");
	else
		color_printk(RED, BLACK, "Refers to a descriptor in the GDT or the current LDT;\n");

	if ((error_code & 0x02) == 0)
		if (error_code & 0x04)
			color_printk(RED, BLACK, "Refers to a segment or gate descriptor in the LDT;\n");
		else
			color_printk(RED, BLACK, "Refers to a descriptor in the current GDT;\n");

	color_printk(RED, BLACK, "Segment Selector Index:%#010x\n", error_code & 0xfff8);

	while (1);
}

void do_page_fault(unsigned long rsp, unsigned long error_code) {
	unsigned long cr2 = 0;
	__asm__ __volatile__("movq	%%cr2,	%0" : "=r"(cr2)::"memory");            // 在第二个页错误发生前保存cr2寄存器的值

//	unsigned long *rip_ptr = NULL;
//	rip_ptr = (unsigned long *) (rsp + 0x98);
//
//	color_printk(RED, BLACK, "do_page_fault(14),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp,
//				 *rip_ptr);

	unsigned long rip = 0;
	rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_page_fault(14),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp,
				 rip);

	if (!(error_code & 0x01))        // P
		color_printk(RED, BLACK, "Page Not-Present,\t");

	if (error_code & 0x02)            // W/R
		color_printk(RED, BLACK, "Write Cause Fault,\t");
	else
		color_printk(RED, BLACK, "Read Cause Fault,\t");

	if (error_code & 0x04)            // U/S
		color_printk(RED, BLACK, "Fault in user(3)\t");
	else
		color_printk(RED, BLACK, "Fault in supervisor(0,1,2)\t");

	if (error_code & 0x08)            // RSVD
		color_printk(RED, BLACK, ",Reserved Bit Cause Fault\t");

	if (error_code & 0x10)            // I/D
		color_printk(RED, BLACK, ",Instruction fetch Cause Fault");

	color_printk(RED, BLACK, "\n");

	color_printk(RED, BLACK, "CR2:%#018lx\n", cr2);

	while (1);
}

void do_x87_FPU_error(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_x87_FPU_error(16),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp,
				 rip);
	while (1);
}

void do_alignment_check(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_alignment_check(17),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp,
				 rip);
	while (1);
}

void do_machine_check(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_machine_check(18),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp,
				 rip);
	while (1);
}

void do_SIMD_exception(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_SIMD_exception(19),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code, rsp,
				 rip);
	while (1);
}

void do_virtualization_exception(unsigned long rsp, unsigned long error_code) {
	unsigned long rip = ((struct pt_regs *) rsp)->rip;
	color_printk(RED, BLACK, "do_virtualization_exception(20),ERROR_CODE:%#018lx,RSP:%#018lx,RIP:%#018lx\n", error_code,
				 rsp, rip);
	while (1);
}

void sys_vector_init() {
	set_trap_gate(0, 1, divide_error);
	set_trap_gate(1, 1, debug);
	set_intr_gate(2, 1, nmi);
	set_system_gate(3, 3, int3); // 允许用户态触发断点
	set_system_gate(4, 1, overflow);
	set_system_gate(5, 1, bounds);
	set_trap_gate(6, 1, undefined_opcode);
	set_trap_gate(7, 1, dev_not_available);
	set_intr_gate(8, 1, double_fault); // 双重错误使用IST=1
	set_trap_gate(9, 1, coprocessor_segment_overrun);
	set_trap_gate(10, 1, invalid_TSS);
	set_trap_gate(11, 1, segment_not_present);
	set_trap_gate(12, 1, stack_segment_fault);
	set_trap_gate(13, 1, general_protection);
	set_intr_gate(14, 1, page_fault);
	// 15 Intel reserved. Do not use.
	set_trap_gate(16, 1, x87_FPU_error);
	set_trap_gate(17, 1, alignment_check);
	set_trap_gate(18, 1, machine_check);
	set_trap_gate(19, 1, SIMD_exception);
	set_trap_gate(20, 1, virtualization_exception);

	// set_system_gate(SYSTEM_CALL_VECTOR,7,system_call);
}
