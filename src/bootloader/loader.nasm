org 0x10000
	jmp Label_Start

%include "fat12-header.nasm"
BaseOfKernelFile	equ	0x00
OffsetOfKernelFile	equ	0x100000

; Kernel临时缓存地址
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

;=======	reset floppy

	xor	ah,	ah
	xor	dl,	dl
	int	13h

; ===== Search kernel.bin
	mov	word	[SectorNo],	SectorNumOfRootDirStart

Lable_Search_In_Root_Dir_Begin:

	cmp	word	[RootDirSizeForLoop],	0
	jz	Label_No_LoaderBin
	dec	word	[RootDirSizeForLoop]
	mov	ax,	00h
	mov	es,	ax
	mov	bx,	8000h
	mov	ax,	[SectorNo]
	mov	cl,	1
	call	Func_ReadOneSector
	mov	si,	KernelFileName
	mov	di,	8000h
	cld
	mov	dx,	10h

Label_Search_For_LoaderBin:

	cmp	dx,	0
	jz	Label_Goto_Next_Sector_In_Root_Dir
	dec	dx
	mov	cx,	11

Label_Cmp_FileName:

	cmp	cx,	0
	jz	Label_FileName_Found
	dec	cx
	lodsb
	cmp	al,	byte	[es:di]
	jz	Label_Go_On
	jmp	Label_Different

Label_Go_On:

	inc	di
	jmp	Label_Cmp_FileName

Label_Different:

	and	di,	0FFE0h
	add	di,	20h
	mov	si,	KernelFileName
	jmp	Label_Search_For_LoaderBin

Label_Goto_Next_Sector_In_Root_Dir:

	add	word	[SectorNo],	1
	jmp	Lable_Search_In_Root_Dir_Begin

;=======	display on screen : ERROR:No KERNEL Found

Label_No_LoaderBin:

	mov	ax,	1301h
	mov	bx,	008Ch
	mov	dx,	0300h		;row 3
	mov	cx,	21
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	NoLoaderMessage
	int	10h
	jmp	$

;=======	found loader.bin name in root director struct

Label_FileName_Found:
	mov	ax,	RootDirSectors
	and	di,	0FFE0h                      ; 回到目录项第0位
	add	di,	01Ah
	mov	cx,	word	[es:di]             ; cx: DIR_FstClus -> Kernel.bin起始簇号
	push	cx
	add	cx,	ax
	add	cx,	SectorBalance
	mov	eax,	BaseTmpOfKernelAddr	    ; es = BaseOfKernelFile
	mov	es,	eax
	mov	bx,	OffsetTmpOfKernelFile	    ; bs = OffsetOfKernelFile
	mov	ax,	cx                          ; ax = 经过处理后的 FstClus 即物理软盘中真正的Kernel起始簇号

Label_Go_On_Loading_File:
	push	ax
	push	bx
	mov	ah,	0Eh
	mov	al,	'.'                         ; 增强交互
	mov	bl,	0Fh
	int	10h
	pop	bx
	pop	ax

	mov	cl,	1
	call	Func_ReadOneSector
	pop	ax

;;;;;;;;;;;;;;;;;;;;;;;
	push	cx
	push	eax
	push	fs
	push	edi
	push	ds
	push	esi

	mov	cx,	200h                        ; 0x200 = 512
	mov	ax,	BaseOfKernelFile
	mov	fs,	ax                          ; FIXME!:这条指令在物理机器上不可行,待修正
	mov	edi,	dword	[OffsetOfKernelFileCount]       ; [OffsetOfKernelFileCount] 累加器,保存已读入字节数,
															; 用于索引写入到内存的位置

	mov	ax,	BaseTmpOfKernelAddr
	mov	ds,	ax
	mov	esi,	OffsetTmpOfKernelFile

Label_Mov_Kernel:	;------------------

	mov	al,	byte	[ds:esi]        ; Copy 1B
	mov	byte	[fs:edi],	al      ; 到 0x1000000(1MB)处

	inc	esi
	inc	edi

	loop	Label_Mov_Kernel

	mov	eax,	0x1000
	mov	ds,	eax

	mov	dword	[OffsetOfKernelFileCount],	edi     ; 更新已写入字节数

	pop	esi
	pop	ds
	pop	edi
	pop	fs
	pop	eax
	pop	cx
;;;;;;;;;;;;;;;;;;;;;;;

	call	Func_GetFATEntry
	cmp	ax,	0FFFh
	jz	Label_File_Loaded
	push	ax
	mov	dx,	RootDirSectors
	add	ax,	dx
	add	ax,	SectorBalance

	jmp	Label_Go_On_Loading_File

Label_File_Loaded:

	mov	ax, 0B800h
	mov	gs, ax
	mov	ah, 0Fh				; 0000: 黑底    1111: 白字
	mov	al, 'G'
	mov	[gs:((80 * 0 + 39) * 2)], ax	; 屏幕第 0 行, 第 39 列。

KillMotor:

	push	dx
	mov	dx,	03F2h
	mov	al,	0
	out	dx,	al
	pop	dx

; ==== messages
StartLoaderMessage:
	db "Start Loader"
NoKernelMessage:     	db    	"ERROR:No LOADER Found"
KernelFileName:      	db    	"KERNEL  BIN",0     ; 文件名 8 Byte,扩展名 3 Byte, 文件名不足 8 Byte 补足空格