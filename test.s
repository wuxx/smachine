LOCATE #0x0
; comment
mov r0, #0x1000 ; some comment

loop:
    jmp loop
mov r1, r0
mov r1, r0
ldr r0, [r1]
str r0, [r1]
mov r2, #0x2000
push r0
pop  r0
;call r0
;ret
add r0, r0, r1
div r0, r0, r1
sub r0, r0, r1
or  r0, r0, r1
xor r0, r0, r1
mov r0, #0x0
jmp r0
jmpn r0
jmpz r0
jmpo r0 
jmpnn r0
jmpnz r0
jmpno r0

