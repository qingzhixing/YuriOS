org 0x10000

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

	jmp $

StartLoaderMessage:
	db "Start Loader"