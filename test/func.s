LOCATE #0x0
; factorial function

mov r0, #5
mov r1, #0x0
mov fp, #0x0
mov sp, #0x1000

push r0
call fact_func
jmp end

fact_func:
    push fp
    mov fp, sp   
    ldr r0, [fp, #8] ; arg1

    mov r1, #1  ; put sum in r1

loop:
    mul r1, r0, r1
    sub r0, r0, #1
    jmpnz loop

    mov r0, r1

    mov sp, fp
    pop fp
    ret

end:
    halt
