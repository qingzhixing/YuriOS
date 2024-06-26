org 0x10000
	jmp Label_Start

; == include
; 根目录占用的扇区数
;(BPB_RootEntCnt * 32 + BPB_BytesPerSec – 1) / BPB_BytesPerSec
; = (224×32 + 512 – 1) / 512 = 14
; 这里 "+ BPB_BytesPerSec – 1" 是向上取整的意思
RootDirSectors equ 14

; 根目录的起始扇区号
; 保留扇区数 + FAT表扇区数 * FAT表份数 = 1 + 9 * 2 = 19
SectorNumOfRootDirStart equ 19

; FAT1 表的起始扇区号
SectorNumOfFAT1Start equ 1

; 用于平衡文件
SectorBalance equ 17

; == FAT12 File System Info Structure
FAT_12_Structure:
    BS_OEMName	db	'MINEboot'		;生产厂商名 8B
    BPB_BytesPerSec	dw	512			;每扇区字节数 2B
    BPB_SecPerClus	db	1			;每簇扇区数 1B
    BPB_RsvdSecCnt	dw	1			;保留扇区数 2B
        ;FAT 表从软盘的第二个扇区开始。
    BPB_NumFATs	db	2				;FAT表份数 1B
		;FAT 表2是FAT表1的数据备份表
	BPB_RootEntCnt	dw	224			;根目录可容纳的目录项数 2B
	BPB_TotSec16	dw	2880		;总扇区数 2B
		;包括保留扇区（内含引导扇区）、FAT 表、根目录区以及数据区占用的全部扇区数，
		;如果此域值为0，那么BPB_TotSec32字段必须是非0值。
	BPB_Media	db	0xf0			;介质描述符 1B
		;对于可移动的存储介质，常用值为0xF0
	BPB_FATSz16	dw	9				;每FAT扇区数 2B
		;FAT 表1和FAT 表2拥有相同的容量，它们的容量均由此值记录
	BPB_SecPerTrk	dw	18			;每磁道扇区数 2Bs
;%include "fat12-header.nasm"
BaseOfKernelFile	equ	0x00
OffsetOfKernelFile	equ	0x100000

BaseTmpOfKernelAddr	equ	0x00
OffsetTmpOfKernelFile	equ	0x7E00

MemoryStructBufferAddr	equ	0x7E00

[SECTION gdt]

LABEL_GDT:		dd	0,0
LABEL_DESC_CODE32:	dd	0x0000FFFF,0x00CF9A00
LABEL_DESC_DATA32:	dd	0x0000FFFF,0x00CF9200

GdtLen	equ	$ - LABEL_GDT
GdtPtr	dw	GdtLen - 1
		dd	LABEL_GDT

SelectorCode32	equ	LABEL_DESC_CODE32 - LABEL_GDT
SelectorData32	equ	LABEL_DESC_DATA32 - LABEL_GDT

[SECTION gdt64]

LABEL_GDT64:		dq	0x0000000000000000
LABEL_DESC_CODE64:	dq	0x0020980000000000
LABEL_DESC_DATA64:	dq	0x0000920000000000

GdtLen64	equ	$ - LABEL_GDT64
GdtPtr64	dw	GdtLen64 - 1
            dd	LABEL_GDT64

SelectorCode64	equ	LABEL_DESC_CODE64 - LABEL_GDT64
SelectorData64	equ	LABEL_DESC_DATA64 - LABEL_GDT64

Label_Start:
	; 初始化寄存器
	mov ax,cs
	mov ds,ax
	mov es,ax

	; 初始化栈寄存器
	mov ax,0x00
	mov ss,ax
	mov sp,0x7c00 ; 0x00500 到 0x07C00都是可用区域

	; display loader msg

	mov ax,0x1301
	mov bx,0x000f
	mov dx,0x0200       ; row 2
	mov cx, 12
	push ax
	mov ax,ds
	mov es,ax
	pop ax
	mov bp, StartLoaderMessage
	int 10h

; ===== Open address A20
	push ax
	in al,0x92   ; 使用0x92 I/O端口 -> A20快速门
	or al, 0b_0000_0010
	out 0x92,al
	pop ax

	cli          ; 关闭外部中断

	db 0x66      ; 32位指令前缀
	lgdt [GdtPtr]

	; Enable Protect Mode
	mov eax,cr0
	or eax,0b_0000_0001
	mov cr0,eax

	mov ax, SelectorData32
	mov fs, ax              ; 使 fs 具有超过1MB的寻址能力

	; Disable Protect Mode
	mov eax,cr0
	and al,0b_1111_1110
	mov cr0,eax

	sti                     ; 打开外部中断
	jmp $

; ==== messages
StartLoaderMessage:
	db "Start Loader"