org 0x7c00

BaseOfStack equ 0x7c00

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