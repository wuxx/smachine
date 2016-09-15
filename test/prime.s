LOCATE #0x0
; check if a num is prime num

mov r1, #30 ;the examined num

mov r0, #0x1
mov sp, #0x1000

loop:
    add r0, r0, #1

    push r0
    push r1

    div r0, r1, r0 ;r0 = r1 / r0, r1 = r1 % r0

    or r1, r1, r1
    jmpz end0

    pop  r1
    pop  r0


    push r0

    add r0, r0, #0x1
    xor r0, r0, r1 

    jmpz end1
    pop r0

    jmp loop

end0:
    mov r0, #0
    halt

end1:
    mov r0, #1
    halt
