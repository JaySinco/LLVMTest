mov ax, 0xb800
mov ds, ax
mov byte [0x00], 'a'
mov byte [0x02], 's'
mov byte [0x03], 0b00101000
mov byte [0x04], 'm'
jmp $
times 483 db 0
db 0x55,0xaa
