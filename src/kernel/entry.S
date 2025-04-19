#include "linkage.h"

/*异常帧结构
	高地址
	+-----------------+
	| SS (OLDSS)      | 0xb8
	+-----------------+
	| RSP (OLDRSP)    | 0xb0
	+-----------------+
	| RFLAGS          | 0xa8
	+-----------------+
	| CS              | 0xa0
	+-----------------+
	| RIP             | 0x98
	+-----------------+
	| ERRCODE         | 0x90
	+-----------------+
	| FUNC            | 0x88
	+-----------------+
	| RAX             | 0x80
	+-----------------+
	| ES              | 0x78
	+-----------------+
	| DS              | 0x70
	+-----------------+
	| RBP             | 0x68
	+-----------------+
	| RDI             | 0x60
	+-----------------+
	| RSI             | 0x58
	+-----------------+
	| RDX             | 0x50
	+-----------------+
	| RCX             | 0x48
	+-----------------+
	| RBX             | 0x40
	+-----------------+
	| R8              | 0x38
	+-----------------+
	| R9              | 0x30
	+-----------------+
	| R10             | 0x28
	+-----------------+
	| R11             | 0x20
	+-----------------+
	| R12             | 0x18
	+-----------------+
	| R13             | 0x10
	+-----------------+
	| R14             | 0x08
	+-----------------+
	| R15             | 0x00
	低地址
*/
/* 寄存器偏移定义 (必须与C结构体一致) */
.set R15,    0x00
.set R14,    0x08
.set R13,    0x10
.set R12,    0x18
.set R11,    0x20
.set R10,    0x28
.set R9,     0x30
.set R8,     0x38
.set RBX,    0x40
.set RCX,    0x48
.set RDX,    0x50
.set RSI,    0x58
.set RDI,    0x60
.set RBP,    0x68
.set DS,     0x70
.set ES,     0x78
.set RAX,    0x80
.set FUNC,   0x88
.set ERRCODE,0x90
.set RIP,    0x98
.set CS,     0xa0
.set RFLAGS, 0xa8
.set OLDRSP, 0xb0
.set OLDSS,  0xb8

// BUILD_TRAP [异常名称], [处理函数], [是否有错误码]

#define BUILD_TRAP(name, func, has_error)\
ENTRY(name)\
    .if has_error == 0;\
        pushq   $0;\
    .endif;\
    	pushq   %rax;\
    	leaq    func(%rip), %rax;\
    	xchgq   %rax, (%rsp);\
    jmp     error_code

#define PATCHABLE(name)\
ENTRY(name)\
	nop;\
	nop;\
	nop;\
	nop

.macro SAVE_ALL

    /* 保存临时寄存器和段寄存器 */
    pushq   %rax            // 临时保存RAX
    movq    %es, %rax
    pushq   %rax           // 保存ES
    movq    %ds, %rax
    pushq   %rax           // 保存DS

    /* 清零RAX用于后续操作 */
    xorq    %rax, %rax

    /* 保存通用寄存器 (顺序必须与结构体一致) */
    pushq   %rbp
    pushq   %rdi
    pushq   %rsi
    pushq   %rdx
    pushq   %rcx
    pushq   %rbx
    pushq   %r8
    pushq   %r9
    pushq   %r10
    pushq   %r11
    pushq   %r12
    pushq   %r13
    pushq   %r14
    pushq   %r15
.endm

.macro RESTORE_ALL
    /* 恢复通用寄存器 (逆序) */
    popq    %r15
    popq    %r14
    popq    %r13
    popq    %r12
    popq    %r11
    popq    %r10
    popq    %r9
    popq    %r8
    popq    %rbx
    popq    %rcx
    popq    %rdx
    popq    %rsi
    popq    %rdi
    popq    %rbp

    /* 恢复段寄存器 */
    popq    %rax           // 弹出DS值
    movw    %ax, %ds
    popq    %rax           // 弹出ES值
    movw    %ax, %es
    popq    %rax           // 恢复原始RAX值

    /* 跳过处理函数和错误码 */
    addq    $0x10, %rsp    // 跳过func和errcode
	iretq                   // 结束异常处理
.endm

#define GET_CURRENT(reg)	\
	movq	$-32768,reg;	\
	andq	%rsp,	reg

ret_from_exception:
/*GET_CURRENT(%ebx)	need rewrite*/
ENTRY(ret_from_intr)
RESTORE_ALL	/*need rewrite*/

PATCHABLE(stack_fault)

error_code:
	SAVE_ALL

	cld

	movq	ERRCODE(%rsp),	%rsi 	// RSI:第二个参数 - 保存错误码
	movq	FUNC(%rsp),	%rdx		// 保存中断处理函数地址

	movq	$0x10,	%rdi			// Code Segment
	movq	%rdi,	%ds				// Code Segment
	movq	%rdi,	%es				// Code Segment

	/* void handle_function (unsigned long rsp, unsigned long error_code)
	* 	传参顺序: doc/kernel/C_Asm/32-bit与64-bit下寄存器传参与定义.png
	*/
	movq	%rsp,	%rdi			// RDI:第一个参数 - 栈顶指针

	callq 	*%rdx					// 运行中断处理函数
	jmp	ret_from_exception			// 恢复运行现场并返回

/* 统一异常处理入口 */
BUILD_TRAP( divide_error,          do_divide_error,           0)
BUILD_TRAP( debug,                 do_debug,                  0)
BUILD_TRAP( int3, 				   do_int3, 				  0)
BUILD_TRAP( overflow,              do_overflow,               0)
BUILD_TRAP( bounds,                do_bounds,                 0)
BUILD_TRAP( undefined_opcode,      do_undefined_opcode,       0)
BUILD_TRAP( dev_not_available,     do_dev_not_available,      0)
BUILD_TRAP( double_fault,          do_double_fault,           1)
BUILD_TRAP( coprocessor_segment_overrun, do_coprocessor_segment_overrun, 0)
BUILD_TRAP( invalid_TSS,           do_invalid_TSS,            1)
BUILD_TRAP( segment_not_present,   do_segment_not_present,    1)
BUILD_TRAP( stack_segment_fault,   do_stack_segment_fault,    1)
BUILD_TRAP( general_protection,    do_general_protection,     1)
BUILD_TRAP( page_fault,            do_page_fault,             1)
BUILD_TRAP( x87_FPU_error,         do_x87_FPU_error,          0)
BUILD_TRAP( alignment_check,       do_alignment_check,        1)
BUILD_TRAP( machine_check,         do_machine_check,          0)
BUILD_TRAP( SIMD_exception,        do_SIMD_exception,         0)
BUILD_TRAP( virtualization_exception, do_virtualization_exception, 0)

ENTRY(nmi)
    /* 步骤1：保存完整上下文 */
    SAVE_ALL

    /* 步骤2：设置内核段寄存器 */
    movq    $0x10, %rax
    movw    %ax, %ds
    movw    %ax, %es

    /* 步骤3：安全检查 */
    stac                    /* SMAP保护 */
    testq   $0x0F, %rsp     /* 16字节对齐 */
    jnz     stack_fault

    /* 步骤4：准备参数 */
    movq    %rsp, %rdi      /* pt_regs指针 */
    xorq    %rsi, %rsi      /* 错误码 */

    /* 步骤5：调用处理程序 */
    call    do_nmi

    /* 步骤6：恢复环境 */
    clac
    RESTORE_ALL
    iretq                   /* 必须使用iretq */
