LOCATE #0x0
mov r0, #0x4000
_start:
    call main
    jmp _exit

main:
    push fp
    mov fp, sp
    sub sp, sp, #0x3
    ldr r0, [fp, #-1]
    ldr r0, [r0]
    push r0
    mov r0, #0x64
    pop  r1
    str r0, [r1]
    ldr r0, [fp, #-2]
    ldr r0, [r0]
    push r0
    mov r0, #0xc8
    pop  r1
    str r0, [r1]
    ldr r0, [fp, #-3]
    ldr r0, [r0]
    push r0
    ldr r0, [fp, #-1]
    ldr r0, [r0]
    push r0
    ldr r0, [fp, #-2]
    ldr r0, [r0]
    pop r1
    add r0, r0, r1
    pop  r1
    str r0, [r1]
    ldr r0, [fp, #-3]
    ldr r0, [r0]
    ldr fp, [fp]
    ret
    mov sp, fp
    pop fp
    ret

_exit:
    halt
