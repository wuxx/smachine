LOCATE #0x0
; caculate 1+2+3+...+100
mov r0, #0x1
mov r1, #0x0
mov sp, #0x1000

loop:
    add r1, r1, r0
    add r0, r0, #0x1
    push r0
    xor r0, r0, #0x64 ; 100
    jmpz end
    pop  r0
    jmp loop

end:
   pop r0  ; restore r0

halt:
   jmp halt
