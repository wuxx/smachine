LOCATE #0x0
mov r0, #0x0
mov r1, #0x0
mov fp, #0x0
mov sp, #0x4000
_start:
call main
jmp _exit
_sum:
push fp
mov fp, sp
sub sp, sp, #8
mov r0, #0x0
add r0, fp, #-4
push r0
mov r0, #0x0
pop r1
str r0, [r1]
mov r0, #0x0
add r0, fp, #-8
push r0
mov r0, #0x0
pop r1
str r0, [r1]
L0:
mov r0, #0x0
add r0, fp, #-4
ldr r0, [r0]
push r0
mov r0, #0x0
add r0, fp, #8
ldr r0, [r0]
pop r1
sub r0, r1, r0
jmpn L2
jmpz L2
mov r0, #0
jmp L3
L2:
mov r0, #1
L3:
add r0, r0, #0x0
jmpz L1
mov r0, #0x0
add r0, fp, #-8
push r0
mov r0, #0x0
add r0, fp, #-8
ldr r0, [r0]
push r0
mov r0, #0x0
add r0, fp, #-4
ldr r0, [r0]
pop r1
add r0, r0, r1
pop r1
str r0, [r1]
mov r0, #0x0
add r0, fp, #-4
push r0
mov r0, #0x0
add r0, fp, #-4
ldr r0, [r0]
push r0
mov r0, #0x1
pop r1
add r0, r0, r1
pop r1
str r0, [r1]
jmp L0
L1:
mov r0, #0x0
add r0, fp, #-8
ldr r0, [r0]
mov sp, fp
pop fp
ret
mov sp, fp
pop fp
ret
main:
push fp
mov fp, sp
sub sp, sp, #4
mov r0, #0x0
add r0, fp, #-4
push r0
mov r0, #0x64
push r0
call _sum
add sp, sp, #0x4
pop r1
str r0, [r1]
mov r0, #0x0
add r0, fp, #-4
ldr r0, [r0]
mov sp, fp
pop fp
ret
mov sp, fp
pop fp
ret
_exit:
halt

LOCATE #0x1008
DW #0x00000000
