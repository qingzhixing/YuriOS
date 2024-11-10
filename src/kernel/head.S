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

.section .text

.globl _start

_start:

	mov	$0x10,	%ax     // 0x10 is the data selector for 64-bit mode
	mov	%ax,	%ds
	mov	%ax,	%es
	mov	%ax,	%fs
	mov	%ax,	%ss
	mov	$0x7E00,	%esp

//=======	load GDTR

	// 使用 %rip 寻址
	lgdt	GDT_POINTER(%rip)       // load GDT

//=======	load	IDTR

	lidt	IDT_POINTER(%rip)

	mov	$0x10,	%ax
	mov	%ax,	%ds
	mov	%ax,	%es
	mov	%ax,	%fs
	mov	%ax,	%gs
	mov	%ax,	%ss

	movq	$0x7E00,	%rsp

//=======	load	cr3

	movq	$0x101000,	%rax
	movq	%rax,		%cr3
	movq	switch_seg(%rip),	%rax
	pushq	$0x08       // 0x08 is the code selector for 64-bit mode
	pushq	%rax
	lretq   // 长返回，切换到 64 位模式

//=======	64-bit mode code

switch_seg:
	.quad	entry64

entry64:
	movq	$0x10,	%rax
	movq	%rax,	%ds
	movq	%rax,	%es
	movq	%rax,	%gs
	movq	%rax,	%ss
	movq	$0xffff800000007E00,	%rsp		/* rsp address */

extern ignore_interrupt
setup_IDT:
	leaq ignore_interrupt(%rip), %rdx // leaq a(b,c,d) == mov (a+b+c*d)

	/*
	* IDT Structure Reference:
	*   "doc/kernel/IDT/IDT Gate Descriptor Structure - 64bits.png"
	* 将IDT数据写入rax:
	* bit 0 ~ bit 15 : dx           -> Offset_1
	* bit 16 ~ bit 31 : 0x00_08     -> Segment Selector(Code Segment)
	* bit 32 ~ bit 47 : 0x8e00      -> P, DPL, 0, Gate Type(Interrupt Gate), Reserved, IST - TSS段偏移
	* bit 48 ~ bit 63 : edx 高16bit  -> Offset_2
	* bit 64 ~ bit 95               -> Offset_3
	* bit 96 ~ bit 127              -> Reserved
	*
	*   Offset: 64-bit value -> Entry point of the Interrupt Service Routine (中断处理程序)
	*/
	movq $(0x08 << 16), %rax
	movw %dx, %ax
	movq $(0x8e00 << 32), %rcx
	addq %rcx, %rax

	// 放入rdx bit 16 ~ bit 31 到 offset_2
	movl %edx, %ecx
	shrl $16, %ecx
	shlq $48, %rcx
	addq %rcx, %rax

	shrq $32, %rdx

	leaq IDT_Table(%rip), %rdi
	mov $256, %rcx

rp_sidt:    // Repeat set IDT
	movq %rax, (%rdi)   // 将rax中存储的数据放入 IDT_Table(0)
	mov rdx, 8(%rdi)    // 填充 offset_3
	add $0x10 , %rdi
	dec %rcx
	jne rp_sidt

setup_TSS64:     // Task State Segment
	/*
	* TSS Descriptor Structure Reference:
	*   doc/kernel/Descriptor/System Segment Descriptor.png
	*/

	leaq TSS64_Table(%rip), %rdx    // Descriptor Base Address
	xorq %rax, %rax
	xorq %rcx, %rcx

	/*
	* Descriptor bit 40 ~ bit 47: 0x89
	* 40~43 -> Type: 0b1001 = 0x9 (80386-TSS, 32 bit)
	* 44~47 -> S(0 for System Segment),DPL(0b00),P(1)
	*/
	movq $0x89, %rax
	shlq $40, %rax

	// Base Address High(8 bit)
	movl %edx, %ecx
	shrl $24, %ecx
	shlq $56, %rcx
	addq %rcx, %rax
	xorq %rcx, %rcx

	// Base Address Low (24 bit)
	movl %edx, %ecx
	andl $0xffffff, %ecx    // 取后 24 bit
	shlq $16, %rcx
	addq %rcx, %rax

	// bit 0 ~ bit 15 -> Limit Low
	addq $103,  %rax
	/*
	* Q: 为何这里的 limit 为 103?
	* A(Fitten Code AI): 具体来说，103这个值可能表示TSS描述符所使用的内存空间的大小。
	*   根据不同操作系统和实现，TSS的大小可能会有所变化。对于大多数实现来说，
	*   TSS的最小大小为104字节（可以容纳一些基本的任务状态信息），但在某些系统或版本中可能会存在调整或优化。
	*/

	leaq GDT_Table(%rip), %rdi
	movq %rax, 64(%rdi)             // 将rax值存储到 GDT_Table 0x40偏移处(为TSS预留了位置)

	shrq $32, %rdx
	movq %rdx, 72(%rdi)

	// Load Task Register
	mov $0x40, %ax                  // 0x40 为 TSS Descriptor 选择子在GDT内的偏移
	ltr %ax

jump_to_kernel:
	movq	go_to_kernel(%rip),	%rax		/* movq address */
	pushq	$0x08           // code selector for 64-bit mode
	pushq	%rax
	lretq   // 长返回，切换到内核 (从栈中读取 段选择子 与 偏移)

go_to_kernel:
	.quad	Start_Kernel

//=======	init page
.align 8

.org	0x1000      // 位于内核执行头程序段的起始位置0x1000处
/*
	页初始化: ref: [doc/kernel/paging/64-bit_page_tables1.png]
*/

/*
    Ref: https://wiki.osdev.org/Paging#64-Bit_Paging
	the level-4 page map table (PML4).
	The PML4 is a 64-bit page table that maps the first 512 GiB of physical memory.

	This allows a processor to map 48-bit virtual addresses to 52-bit physical addresses.
*/
__PML4E:                // Page Map Level 4 Entry
	// quad - 8 Bytes
	/*
		0x102007 = 0b 0000 0001 0000 0010 0000 0000 0111
		0x102007 表示一个存在且可写的页表项，指向物理地址 0x102000，并且该页可被用户模式访问。
	*/
	.quad	0x102007
	.fill	255,8,0
	.quad	0x102007
	.fill	255,8,0

.org	0x2000          // 位于内核代码段的起始位置0x2000处

__PDPTE:                // Page Directory Pointer Table Entry

	/*
		0x103003 是一个页目录指针表项（PDPTE），表示一个存在且可写的页表项，
			指向物理地址 0x103000，并且该页可被用户模式访问。
	*/
	.quad	0x103003
	.fill	511,8,0

.org	0x3000          // 位于内核数据段的起始位置0x3000处

__PDE:                  // Page Directory Entry

						// 前10MB映射到 0x0 与 0xffff800000000000处
	.quad	0x000083
	.quad	0x200083
	.quad	0x400083
	.quad	0x600083
	.quad	0x800083

						// 把从0xe0000000 开始的16MB 映射到0xa00000 与 0xffff800000a00000处
	.quad	0xe0000083		/*0x a00000*/
	.quad	0xe0200083
	.quad	0xe0400083
	.quad	0xe0600083
	.quad	0xe0800083		/*0x1000000*/
	.quad	0xe0a00083
	.quad	0xe0c00083
	.quad	0xe0e00083
	.fill	499,8,0

//=======	GDT_Table

.section .data

.globl GDT_Table

// Description Table

GDT_Table:  // 详见第6章 全局描述符表
	.quad	0x0000000000000000			/*0	NULL descriptor		       	00*/
	.quad	0x0020980000000000			/*1	KERNEL	Code	64-bit	Segment	-> 08*/
	.quad	0x0000920000000000			/*2	KERNEL	Data	64-bit	Segment	-> 10*/
	.quad	0x0020f80000000000			/*3	USER	Code	64-bit	Segment	-> 18*/
	.quad	0x0000f20000000000			/*4	USER	Data	64-bit	Segment	-> 20*/
	.quad	0x00cf9a000000ffff			/*5	KERNEL	Code	32-bit	Segment	-> 28*/
	.quad	0x00cf92000000ffff			/*6	KERNEL	Data	32-bit	Segment	-> 30*/
	.fill	10,8,0					/*8 ~ 9	TSS (jmp one segment <7>) in long-mode 128-bit -> 40*/
	// fill (repeat, size, value)
GDT_END:

GDT_POINTER:
GDT_LIMIT:	.word	GDT_END - GDT_Table - 1
GDT_BASE:	.quad	GDT_Table

//=======	IDT_Table

// 中断描述符表
.globl IDT_Table

IDT_Table:
	.fill  512,8,0
IDT_END:

IDT_POINTER:
IDT_LIMIT:	.word	IDT_END - IDT_Table - 1
IDT_BASE:	.quad	IDT_Table

//=======	TSS64_Table

// 任务状态段
.globl	TSS64_Table

TSS64_Table:
	.fill  13,8,0
TSS64_END:

TSS64_POINTER:
TSS64_LIMIT:	.word	TSS64_END - TSS64_Table - 1
TSS64_BASE:	.quad	TSS64_Table