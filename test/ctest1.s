LOCATE #0x0
mov r0, #0x0
mov r1, #0x0
mov fp, #0x0
mov sp, #0x4000
_start:
call main
jmp _exit
main:
push fp
mov fp, sp
sub sp, sp, #12
mov r0, #0x1008
push r0
mov r0, #0x20
pop r1
str r0, [r1]
mov r0, #0x0
add r0, fp, #-4
push r0
mov r0, #0x64
pop r1
str r0, [r1]
mov r0, #0x0
add r0, fp, #-8
push r0
mov r0, #0xc8
pop r1
str r0, [r1]
mov r0, #0x0
add r0, fp, #-12
push r0
mov r0, #0x0
add r0, fp, #-4
ldr r0, [r0]
push r0
mov r0, #0x0
add r0, fp, #-8
ldr r0, [r0]
pop r1
add r0, r0, r1
pop r1
str r0, [r1]
mov r0, #0x0
add r0, fp, #-12
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
DW #0x00000000
