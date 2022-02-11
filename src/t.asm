section .text
    global asm_xor

asm_xor:
    push rbp
    mov rbp,rsp
    xor ecx,edx
    mov eax,ecx
    mov rsp,rbp
    pop rbp
    ret
