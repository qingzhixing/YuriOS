org 0x7c00

BaseOfStack equ 0x7c00

; Loader 0x1000:0x00 -> 0x10000
BaseOfLoader equ 0x1000
OffsetOfLoader equ 0x00

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
; TODO: 具体如何使用SectorBalance?
SectorBalance equ 17

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

; == search loader.bin
	mov word [SectorNo],	SectorNumOfRootDirStart

; == 在根目录表中查询
Label_Search_In_Root_Dir_Begin:
	cmp word [RootDirSizeForLoop], 0
	jz Label_No_LoaderBin
	dec word [RootDirSizeForLoop]

	; == call read func
	mov ax,0000h
	mov es,ax
	mov bx,8000h
	mov ax,[SectorNo]
	mov cl,1
	call Func_ReadOneSector

	; == Judge loader.bin
	mov si,LoaderFileName
	mov di,8000h		; di存放从硬盘读取的扇区读入内存后的偏移地址(基地址:0x0000)

	cld	; 忽略内存增长方向,全部由低到高

	; 每个扇区可容纳的目录项个数（512 / 32 = 16 = 0x10）
	; 在该扇区搜寻的次数: 0x10 次
	mov dx,10h

; Input: dx
Label_Search_For_LoaderBin:
	cmp dx,0
	jz Label_Goto_Next_Sector_In_Root_Dir
	dec dx
	; LoaderFileName - 11B
	mov cx,11

Label_Cmp_FileName:
	cmp cx,0
	jz Label_FileName_Found
	dec cx

	; 块装入指令，把SI指向的存储单元读入累加器,LODSB读入AL,SI + 1
	lodsb
	cmp al,byte [es:di]
	jz Label_Go_On
	jmp Label_Different

Label_Go_On:
	; 移动模板串指针指向下一个字符
	inc di
	jmp Label_Cmp_FileName

Label_Different:
	; 0xFFE0 = 0b_1111_1111_1110_0000
	; 这里作用是去掉di的后5位,数学上可以认为将di对齐到比他小的最大的32倍数地址上
	; 即 向0取整去掉 模32 的余数
	; 例如 di = 0x8011 -> 0x8000
	; di = 0x8023 -> 0x8020
	; 作用: 回到当前目录项的第一个bit
	and di,0xffe0
	add di,20h		; di 增加 32 bit = 0x20 bit, 即移动到下一个目录项
	mov si,LoaderFileName
	jmp Label_Search_For_LoaderBin

Label_Goto_Next_Sector_In_Root_Dir:
	add word [SectorNo], 1
	jmp Label_Search_In_Root_Dir_Begin

Label_No_LoaderBin:
	mov ax,1301h
	mov bx,008ch
	mov dx,0100h
	mov cx,21
	push ax
	mov ax,ds
	mov es,ax
	pop ax
	mov bp,NoLoaderMessage
	int 10h
	jmp $

Label_FileName_Found:
	mov ax,RootDirSectors
	and di,0xffe0	; 回到当前目录项的第一个bit
	add di,0x1a
	mov cx,word [es:di]	; 获取loader的起始簇号
	push cx
	add cx,ax
	add cx, SectorBalance   ; 啥?没看懂

	; 为读入Loader作准备
	mov ax,BaseOfLoader
	mov es,ax
	mov bx,OffsetOfLoader
	mov ax,cx

; 读入Loader一个簇 = (BPB_SecPerClus = 1) 个扇区
; 寄存器输入:
;   ax => 待读入的簇号
;   ex:bx => 读入Loader对应扇区在内存中放置的位置
Label_Go_On_Loading_File:
	push ax
	push bx

    ; 在屏幕上显示一个'.',增强交互效果
    ; INT 0x10,AH=0x0e 在屏幕上显示一个字符
    ; AL = 待显示字符
    ; BL = 前景色
	mov ah,0x0e
	mov al,'.'
	mov bl,0x0f
	int 0x10

	; 读入一个Loader扇区
	pop bx
	pop ax
	mov cl,1
	call Func_ReadOneSector

    ; Label_Go_On_Reading_File 中,ax作为读取簇的索引
	pop ax              ; 第一次进入时: 将前面push的cx - loader的起始簇号恢复到ax中
						; 非第一次进入: 获取当前簇号(由上一次循环保存)
	; AH=FAT表项号（输入参数/输出参数）
	call Func_GetFATEntry
	cmp ax,0x0fff   ; 当前簇是最后一个簇
	jz Label_File_Loaded
	push ax             ; 备份我们查询到的下一个簇编号号

	mov dx,RootDirSectors
	add ax,dx
	add ax,SectorBalance

	add bx, [BPB_BytesPerSec]   ; bx右移一个扇区大小,es:bp指向内存中Loader读取到的地址
	jmp Label_Go_On_Loading_File

Label_File_Loaded:
	jmp $
	; TODO:止步于此

; == Functions

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

	; To Test
	sub esp,2
	mov byte [bp - 1],cl

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
		mov al,byte [bp - 1]
		int 13
		; wait for complete(CF flag == 0)
		jc Label_Go_On_Reading

	add esp,2	; recycle memory
	pop bp
	ret


; == get FAT Entry
; 根据当前FAT表项索引出下一个FAT表项
; AH=FAT表项号（输入参数/输出参数）
; TODO:止步于此
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

; == temp var
RootDirSizeForLoop:    	dw    RootDirSectors 		; 用于循环查询根目录扇区
SectorNo:              	dw    0 					; 当前查找到第 SectorNo 号根目录扇区
Odd:                   	db    0

; == display msgs
StartBootMessage:		db 		"YuriOS Booting" 
NoLoaderMessage:     	db    	"ERROR:No LOADER Found" 
LoaderFileName:      	db    	"LOADER  BIN",0

; == Fill Zero
times 510 - ($-$$) db 0
db 0x55,0xaa 