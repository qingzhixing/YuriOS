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

; Protect Mode Segment Descriptor: 8B
LABEL_GDT:		dd	0,0
LABEL_DESC_CODE32:	dd	0x0000FFFF,0x00CF9A00
LABEL_DESC_DATA32:	dd	0x0000FFFF,0x00CF9200

GdtLen	equ	$ - LABEL_GDT
GdtPtr	dw	GdtLen - 1  ; 2B
		dd	LABEL_GDT   ; 4B

SelectorCode32	equ	LABEL_DESC_CODE32 - LABEL_GDT
SelectorData32	equ	LABEL_DESC_DATA32 - LABEL_GDT

[SECTION gdt64]
; 不懂可以查阅资料 AMD开发手册-vol2 Chapter4-Segmented Virtual Memory

; 对于低32位(4B),全部为0:
;   Segmentation is disabled in 64-bit mode, and code segments span
;       all of virtual memory.
;   For the purpose of virtual-address calculations,
;       the base address is treated as if it has a value of zero
;   Segment-limit checking is not performed

; 对于高32位,仅有D, L, P, DPL, C 段有效
;   DPL:
;       Descriptor Privilege-Level (DPL) Field. Bits 14–13 of byte +4(高32位). The DPL field indicates the
;       descriptor-privilege level of the segment. DPL can be set to any value from 0 to 3, with 0 specifying the
;       most privilege and 3 the least privilege.
;   L - Long (L) Attribute Bit:
;       L=1 时表示 64位段描述符,L=0 则为32位兼容模式(Compatibility mode)
;   D:
;       If the processor is running in 64-bit mode (L=1), the only valid setting of the D bit is 0. This setting
;        produces a default operand size of 32 bits and a default address size of 64 bits. The combination L=1
;        and D=1 is reserved for future use
;   C: Data-Segment Descriptor—Long Mode
;       用于决定低特权级跳转到高特权级时CPL是否改变
;   P: 段存在标志(Segment present)
LABEL_GDT64:		dq	0x00000000_00000000
; Code Segment:
;   P = 1 存在
;   DPL = 0b00  最高特权级0
;   C = 0 从高特权级跳转到这个段时特权级仍然保持高特权级
;   L = 1 Long Mode 64位模式
;   D = 0 L=1 时 D 一定为 0
LABEL_DESC_CODE64:	dq	0x00209800_00000000
; Data Segment:
;   P = 1 存在
;   DPL = 0b00 最高特权级
;   C = 0, L = 1, D = 0
LABEL_DESC_DATA64:	dq	0x00009200_00000000

GdtLen64	equ	$ - LABEL_GDT64
GdtPtr64	dw	GdtLen64 - 1
            dd	LABEL_GDT64

SelectorCode64	equ	LABEL_DESC_CODE64 - LABEL_GDT64
SelectorData64	equ	LABEL_DESC_DATA64 - LABEL_GDT64

[SECTION .s16]
[BITS 16]

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
	mov	bp,	NoKernelMessage
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

; ===== 关闭软驱马达
KillMotor:

	push	dx
	mov	dx,	03F2h
	mov	al,	0
	out	dx,	al
	pop	dx

; ===== Get Memory Address size type
; 这里采用 int 15h 的 0xE820功能
; Ref: https://wiki.osdev.org/Detecting_Memory_(x86)#BIOS_Function:_INT_0x15,_EAX_=_0xE820
; print msg
	mov	ax,	1301h
    mov	bx,	000Fh
    mov	dx,	0400h		;row 4
    mov	cx,	24
    push	ax
    mov	ax,	ds
    mov	es,	ax
    pop	ax
    mov	bp,	StartGetMemStructMessage
    int	10h

	mov	ebx,	0
	mov	ax,	0x00
	mov	es,	ax
	mov	di,	MemoryStructBufferAddr

	Label_Get_Mem_Struct:

		mov	eax,	0x0E820
		mov	ecx,	20
		mov	edx,	0x534D4150
		int	15h
		jc	Label_Get_Mem_Fail
		add	di,	20

		cmp	ebx,	0
		jne	Label_Get_Mem_Struct
		jmp	Label_Get_Mem_OK

	Label_Get_Mem_Fail:

		mov	ax,	1301h
		mov	bx,	008Ch
		mov	dx,	0500h		;row 5
		mov	cx,	23
		push	ax
		mov	ax,	ds
		mov	es,	ax
		pop	ax
		mov	bp,	GetMemStructErrMessage
		int	10h
		jmp	$

	Label_Get_Mem_OK:

		mov	ax,	1301h
		mov	bx,	000Fh
		mov	dx,	0600h		;row 6
		mov	cx,	29
		push	ax
		mov	ax,	ds
		mov	es,	ax
		pop	ax
		mov	bp,	GetMemStructOKMessage
		int	10h

;=======	get SVGA information
; Ref: https://wiki.osdev.org/VESA_Video_Modes
	mov	ax,	1301h
	mov	bx,	000Fh
	mov	dx,	0800h		;row 8
	mov	cx,	23
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	StartGetSVGAVBEInfoMessage
	int	10h

	;	in: es:di - 512-byte buffer
    ;	out: cf - set on error

    ;struct VbeInfoBlock {
    ;   char     VbeSignature[4];         // == "VESA"
    ;   uint16_t VbeVersion;              // == 0x0300 for VBE 3.0
    ;   uint16_t OemStringPtr[2];         // isa vbeFarPtr
    ;       指向oem字符串的指针，该指针是一个16位的selector:offset形式的指针，在实模式下可以直接使用。
    ;       示例: "VM ware, Inc. VBE support 3.0"
    ;   uint8_t  Capabilities[4];
    ;   uint16_t VideoModePtr[2];         // isa vbeFarPtr , 指向视频模式列表的指针
    ;   uint16_t TotalMemory;             // as # of 64KB blocks
    ;   uint8_t  Reserved[492];
    ;} __attribute__((packed));

	;====== INT 0x10, AX=0x4F00 ======
    ;    Get Controller Info. This is the one that returns the array of all supported video modes.
	mov	ax,	0x00
	mov	es,	ax
	mov	di,	0x8000      ; VbeInfoBlock保存在 0x8000
	mov	ax,	4F00h

	int	10h

	; All VESA functions return 0x4F in AL if they are supported
    ;  and use AH as a status flag, with 0x00 being success.
	cmp	ax,	004Fh

	jz	.KO

;=======	Fail

	mov	ax,	1301h
	mov	bx,	008Ch
	mov	dx,	0900h		;row 9
	mov	cx,	23
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	GetSVGAVBEInfoErrMessage
	int	10h

	jmp	$

	.KO:

		mov	ax,	1301h
		mov	bx,	000Fh
		mov	dx,	0A00h		;row 10
		mov	cx,	29
		push	ax
		mov	ax,	ds
		mov	es,	ax
		pop	ax
		mov	bp,	GetSVGAVBEInfoOKMessage
		int	10h

;=======	Get SVGA Mode Info

	; log msg
	mov	ax,	1301h
	mov	bx,	000Fh
	mov	dx,	0C00h		;row 12
	mov	cx,	24
	push	ax
	mov	ax,	ds
	mov	es,	ax
	pop	ax
	mov	bp,	StartGetSVGAModeInfoMessage
	int	10h

	mov	ax,	0x00
	mov	es,	ax
	mov	si,	0x800e ; VbeInfoBlock::VideoModePtr[0]-selector:offset中的offset

	mov	esi,	dword	[es:si]     ;	get first video mode number
	mov	edi,	0x8200


	Label_SVGA_Mode_Info_Get:

	; ===== INT 0x10, AX=0x4F01, CX=mode, ES:DI=256 byte buffer =====
    ;    Get Mode Info. Call this for each member of the mode array to find out the details of that mode.
    ;     The 256 byte buffer will be filled by the mode info block.

		mov	cx,	word	[es:esi]

;=======	display SVGA mode information

	push	ax

	mov	ax,	00h
	mov	al,	ch
	call	Label_DispAL

	mov	ax,	00h
	mov	al,	cl
	call	Label_DispAL

	pop	ax

;=======

	cmp	cx,	0FFFFh      ; vesa modes list empty
	jz	Label_SVGA_Mode_Info_Finish


	mov	ax,	4F01h
	int	10h

	cmp	ax,	004Fh

	jnz	Label_SVGA_Mode_Info_FAIL

	add	esi,	2
	add	edi,	0x100

	jmp	Label_SVGA_Mode_Info_Get

	Label_SVGA_Mode_Info_FAIL:

		mov	ax,	1301h
		mov	bx,	008Ch
		mov	dx,	0D00h		;row 13
		mov	cx,	24
		push	ax
		mov	ax,	ds
		mov	es,	ax
		pop	ax
		mov	bp,	GetSVGAModeInfoErrMessage
		int	10h

	Lable_SET_SVGA_Mode_VESA_VBE_FAIL:
		mov	ax,	1301h
        mov	bx,	008Ch
        mov	dx,	0D00h		;row 13
        mov	cx,	29
        push	ax
        mov	ax,	ds
		mov	es,	ax
        pop	ax
        mov	bp,	SetSVGAModeVESA_VBE_FAILMessage
        int	10h
        jmp $



	Label_SVGA_Mode_Info_Finish:

		mov	ax,	1301h
		mov	bx,	000Fh
		mov	dx,	0E00h		;row 14
		mov	cx,	30
		push	ax
		mov	ax,	ds
		mov	es,	ax
		pop	ax
		mov	bp,	GetSVGAModeInfoOKMessage
		int	10h

; ===== set SVGA mode (VESA VBE)
; INT 0x10, AX=0x4F02, BX=mode, ES:DI=CRTCInfoBlock
	mov ax,0x4F02
	mov bx,0x4180       ; mode: 0x180 (1440 x 900)
						; or 0x143 (800 x 600)
	int 10h

	; All VESA functions return 0x4F in AL if they are supported
	;  and use AH as a status flag, with 0x00 being success.
	cmp ax,0x004f
	jnz Lable_SET_SVGA_Mode_VESA_VBE_FAIL

	; ===== init IDT & GDT goto protected mode

	; Protected Mode: https://wiki.osdev.org/Protected_Mode
	; Before switching to protected mode, you must:
    ;   Disable interrupts, including NMI (as suggested by Intel Developers Manual).
    ;   Enable the A20 Line.
    ;   Load the Global Descriptor Table with segment descriptors suitable for code, data, and stack.

	cli ; 关闭外部中断

	db 0x66     ; lgdt 是 32位指令
	lgdt [GdtPtr]
	db 0x66
	lidt [IDT_POINTER]

	; Whether the CPU is in Real Mode or in Protected Mode
	;   is defined by the lowest bit of the CR0 or MSW register.
	mov eax,cr0
	or eax,1    ; set PE (Protection Enable) bit in CR0 (Control Register 0)
	mov cr0,eax

	; 远跳转 进入保护模式
	jmp dword SelectorCode32:GO_TO_TMP_Protect

[SECTION .s32]
[BITS 32]

GO_TO_TMP_Protect:
	; ===== go to tmp long mode
	mov ax,0x10 ; TODO: Why 0x10?
	mov dx,ax
	mov es,ax
	mov fs,ax
	mov ss,ax
	mov esp, 0x7E00 ; 栈顶设置为0x7e00 到 0x7c00 可用空间为 0x100

	call check_support_long_mode
	test eax,eax    ; and eax,eax,但不改变EAX的值,只改变FLAG寄存器
					; 等价于判断eax是否为0

	jz long_mode_no_support
	jmp $; TODO:止步于此

[SECTION .s32lib]
[BITS 32]
	; ===== test support long mode or not

	; Ref: intel manual( volume-3a-system-programming )
	; pdf Chapter: 4-6 Vol. 3A
	; LM: IA-32e mode support.()
    ;   If CPUID.80000001H: EDX.LM [bit 29] = 1, IA32_EFER.LME may be set to 1, enabling IA-32e mode (with either
    ;   4-level paging or 5-level paging). (Processors that do not support CPUID function 80000001H do not allow
    ;   IA32_EFER.LME to be set to 1.)

    ; === CPUID Command
    ; CPUID Command Reference:
    ;   https://blog.csdn.net/lee_ham/article/details/103222475
    ;   https://www.felixcloutier.com/x86/cpuid
    ; Returns processor identification and feature information to the EAX, EBX, ECX, and EDX registers,
    ;  as determined by input entered in EAX (in some cases, ECX as well).
check_support_long_mode:
	mov eax,0x80000000      ; EAX Maximum Input Value for Extended Function CPUID Information. EBX Reserved. ECX Reserved. EDX Reserved.

	cpuid   ; 识别cpuid, 读出cpu支持的功能(如是否支持MMX,有无支持4MB页)

	; 判断是否支持0x8000_0001
	cmp eax, 0x80000001
    setnb al        ; setnb: Set byte if not below (CF=0).

	jb check_support_long_mode_done   ; Bigger than

	mov eax,0x80000001  ; EAX Extended Processor Signature and Feature Bits. EBX Reserved.
						; EDX Bits 29: Intel® 64 Architecture available if 1.
	cpuid
	bt edx, 29          ; bt(Bit Test): 用寄存器的第 k 位设置 CF
	setc al         ; set CF to al
					; al: 是否支持IA32-e模式

check_support_long_mode_done:
	movzx eax, al   ; 无符号扩展，并传送。
	ret             ; eax作为 support_long_mode 方法的返回值

long_mode_no_support:
	jmp $

[SECTION .s16lib]
[BITS 16]

; == read one sector from floppy
; input:
; AX=待读取的磁盘起始扇区号(LBA)
; CL=读入的扇区数量；
; ES:BX=>目标缓冲区起始地址。
Func_ReadOneSector:
	; backup bp
	push bp
	; use [bp + x] to find data
	mov bp,sp

	sub esp,2
	mov byte [bp - 2],cl

	push bx

	; LBA to CHS
	mov bl,[BPB_SecPerTrk]
	div bl		; LBA_id / BPB_SecPerTrk
	; ah - 余数 R
	; al - 商 Q

	inc ah
	mov cl,ah	;  cl = R + 1(LBA起始扇区0,CHS为1)

	mov ch,al
	shr ch,1	; ch = Q >> 1 柱面号

	mov dh,al
	and dh,1	; dh = Q & 1 磁头号

	mov dl,[BS_DrvNum]	; 驱动器号

	pop bx		; S:BX=>数据缓冲区

	Label_Go_On_Reading:
		mov ah,02h
		mov al,byte [bp - 2]
		int 13h
		; wait for complete(CF flag == 0)
		jc Label_Go_On_Reading

	add esp,2	; recycle memory
	pop bp
	ret


; == get FAT Entry
; 根据当前FAT表项索引出下一个FAT表项
; AH=FAT表项号（输入参数/输出参数）
Func_GetFATEntry:
	push es
	push bx
	push ax

	mov ax,0000h
	mov es,ax
	pop ax
	mov byte [Odd],0
	; TODO:这里对bx操作干什么?bx存储的不是Loader在内存中的段偏移地址吗?
	mov bx,3
	mul bx
	mov bx,2
	div bx
	cmp dx,0
	jz Label_Even
	mov byte [Odd],1

	Label_Even:
		xor dx,dx
		mov bx,[BPB_BytesPerSec]
		div bx
		push dx
		mov bx,8000h
		add ax,SectorNumOfFAT1Start
		mov cl,2
		call Func_ReadOneSector

		pop dx
		add bx,dx
		mov ax,[es:bx]
		cmp byte [Odd],1
		jnz Label_Even_2
		shr ax,4

	Label_Even_2:
		and ax,0fffh

		pop bx
		pop es
		ret

;=======	display num in al

Label_DispAL:

	push	ecx
	push	edx
	push	edi

	mov	edi,	[DisplayPosition]
	mov	ah,	0Fh
	mov	dl,	al
	shr	al,	4
	mov	ecx,	2
	.begin:

		and	al,	0Fh
		cmp	al,	9
		ja	.1
		add	al,	'0'
		jmp	.2
	.1:

		sub	al,	0Ah
		add	al,	'A'
	.2:

		mov	[gs:edi],	ax
		add	edi,	2

		mov	al,	dl
		loop	.begin

		mov	[DisplayPosition],	edi

		pop	edi
		pop	edx
		pop	ecx

		ret


; == temp var
RootDirSizeForLoop:
	dw    RootDirSectors 		; 用于循环查询根目录扇区
SectorNo:
	dw    0 					; 当前查找到第 SectorNo 号根目录扇区
Odd:
	db    0
OffsetOfKernelFileCount:
	dd	OffsetOfKernelFile
MemStructNumber:
	dd	0
SVGAModeCounter:
	dd	0
DisplayPosition:
	dd	0

;=======	display messages

StartLoaderMessage:
	db	"Start Loader"

NoKernelMessage:
	db	"ERROR:No KERNEL Found"
KernelFileName:
	db	"KERNEL  BIN",0     ; 文件名 8 Byte,扩展名 3 Byte, 文件名不足 8 Byte 补足空格

StartGetMemStructMessage:
	db	"Start Get Memory Struct (address,size,type)."
GetMemStructErrMessage:
	db	"Get Memory Struct ERROR"
GetMemStructOKMessage:
	db	"Get Memory Struct SUCCESSFUL!"

StartGetSVGAVBEInfoMessage:
	db	"Start Get SVGA VBE Info"
GetSVGAVBEInfoErrMessage:
	db	"Get SVGA VBE Info ERROR"
GetSVGAVBEInfoOKMessage:
	db	"Get SVGA VBE Info SUCCESSFUL!"

StartGetSVGAModeInfoMessage:
	db	"Start Get SVGA Mode Info"
GetSVGAModeInfoErrMessage:
	db	"Get SVGA Mode Info ERROR"
GetSVGAModeInfoOKMessage:
	db	"Get SVGA Mode Info SUCCESSFUL!"
SetSVGAModeVESA_VBE_FAILMessage:
	db "Set SVGA Mode VESA VBE FAILED"

;CPUIDNotSupport:
;	db "CPUID Command is NOT support on this machine."

;IA32ENotSupport:
;	db "IA32-e Mode is NOT support on this machine."

; ====== temp IDT
IDT:
	times 0x50 dq 0 ; 0x50 * 8B
IDT_END:

IDT_POINTER:
	dw IDT_END - IDT - 1
	dd IDT