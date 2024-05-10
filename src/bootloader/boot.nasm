org 0x7c00

BaseOfStack equ 0x7c00

; == FAT12 File System Info Structure
FAT_12_Structure:
    jmp short Label_Start
    nop								;跳转指令 3B

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

	BPB_SecPerTrk	dw	18			;每磁道扇区数 2B

	BPB_NumHeads	dw	2			;磁头数 2B

	BPB_HiddSec	dd	0				;隐藏扇区数 4B

	BPB_TotSec32	dd	0			;如果BPB_TotSec16值为0，
		;则由这个值记录扇区数 4B

	BS_DrvNum	db	0				;int 13 需要的驱动器号 1B

	BS_Reserved1	db	0			;未使用 1B

	BS_BootSig	db	0x29			;扩展引导标记(29h) 1B

	BS_VolID	dd	0				;卷序列号 4B

	BS_VolLab	db	'boot loader'	;卷标 11B
		;Windows或Linux系统中显示的磁盘名

	BS_FileSysType	db	'FAT12   '	;文件系统类型 8B
		;只是一个字符串而已，操作系统并不使用该字段来鉴别FAT类文件系统的类型

Label_Start:
    ; == Initialize Registers
    mov ax,cx
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov sp,BaseOfStack

    ; == Clear Screen
    mov ax,0600h
    mov bx,0700h
    mov cx,0
    mov dx,184fh    ; Right-Bottom(79,24)Start:(0,0)
                    ; DH : Column (0x18 = 79)
                    ; DL : Row  (0x4f = 4f)
    int 10h

    ; == Set Focus(Cursor)
    mov ax,0200h
    mov bx,0000h
    mov dx,0000h
    int 10h

    ; == Display Sign
    mov ax,1301h
    mov bx,000fh
    mov dx,0000h
    mov cx,14
    push ax
    mov ax,ds
    mov es,ax
    pop ax
    mov bp,StartBootMessage  ; es:bp -> source string
	int 10h

StartBootMessage:
	db "YuriOS Booting"

; == Fill Zero
times 510 - ($-$$) db 0
db 0x55,0xaa 