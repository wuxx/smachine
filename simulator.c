#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "cpu.h"

struct __cpu__ cpu = 
{
    .r    = {0, 0, 0, 0},
    .flag = 0,
};

u8 cpu_mem[MEM_SIZE] = {0};
int ifd = 0, cpu_cycles = 0;

/*
format: op dst, src1
   mov ri, rj
   mov ri, #imm
*/
void op_mov(struct __instruction__ *pinst)
{
    u32 imm;
    assert(pinst->src2 == 0);
    assert(pinst->am_src2 == 0);

    assert(pinst->am_dst == AM_REG_DIRECT);

    switch (pinst->am_src1) {
        case (AM_IMM):
            imm = cpu_read_mem(PC + 4);
            R(pinst->dst) = imm;
            PC = PC + 8;
            break;
        case (AM_REG_DIRECT):
            R(pinst->dst) = R(pinst->src1);
            PC = PC + 4;
            break;
        case (AM_REG_INDIRECT):
            inst_illegal(pinst);
            break;
        default:
            inst_illegal(pinst);
            break;
    }
}

/*
format: op dst, src1
    ldr ri, [rj]
*/
void op_ldr(struct __instruction__ *pinst)
{
    u32 data;
    assert(pinst->src2 == 0);
    assert(pinst->am_src2 == 0);

    assert(pinst->am_dst == AM_REG_DIRECT);

    switch (pinst->am_src1) {
        case (AM_REG_INDIRECT):
            data = cpu_read_mem(R(pinst->src1));
            R(pinst->dst) = data;
            PC = PC + 4;
            break;
        default:
            inst_illegal(pinst);
            break;
    }

    PC = PC + 4;
}

/*
format: op src, dst
    str ri, [rj] (rj is op_dst, ri is op_src1)
*/
void op_str(struct __instruction__ *pinst)
{
    u32 addr, data;
    assert(pinst->src2 == 0);
    assert(pinst->am_src2 == 0);

    assert(pinst->am_src1 == AM_REG_DIRECT);
    assert(pinst->am_dst  == AM_REG_INDIRECT);

    addr = R(pinst->dst);
    data = R(pinst->src1);

    cpu_write_mem(addr, data);

    PC = PC + 4;
}

/*
format: op src1
    push ri
equal:
    str ri, [sp]
    sp = sp - 4
*/
void op_push(struct __instruction__ *pinst)
{
    u32 addr, data;
    assert(pinst->am_src1 == AM_REG_DIRECT);

    assert(pinst->src2 == 0);
    assert(pinst->am_src2 == 0);
    assert(pinst->dst == RINDEX(SP));
    assert(pinst->am_dst == AM_REG_INDIRECT);

    addr = SP;
    data = R(pinst->src1);

    cpu_write_mem(addr, data);
    SP = SP - 4;
    PC = PC + 4;

}

/*
format: op, src1
    pop ri
equal:
    ldr ri, [sp]
    sp = sp + 4;
*/
void op_pop(struct __instruction__ *pinst)
{
    u32 addr, data;

    assert(pinst->am_dst == AM_REG_DIRECT);

    assert(pinst->src1 == RINDEX(SP));
    assert(pinst->am_src1 == AM_REG_INDIRECT);

    assert(pinst->src2    == 0);
    assert(pinst->am_src2 == 0);

    addr = SP;
    data = R(pinst->src1);

    cpu_write_mem(addr, data);
    SP = SP + 4;
    PC = PC + 4;

}

/*
format: op, src1
    call ri
equal:
    mov pc, ri

    call #imm   TODO
equal:
    mov pc #imm
*/
void op_call(struct __instruction__ *pinst)
{
    u32 addr, data;

    assert(pinst->dst == RINDEX(PC));
    assert(pinst->am_dst == AM_REG_DIRECT);

    assert(pinst->am_src1 == AM_REG_DIRECT);

    assert(pinst->src2    == 0);
    assert(pinst->am_src2 == 0);

    PC = R(pinst->src1);

    SP = SP + 4;
    PC = PC + 4;

}

/*
format: op
    ret
equal:
    ldr pc, [sp]
    sp = sp + 4;
*/
void op_ret(struct __instruction__ *pinst)
{
    u32 addr, data;

    assert(pinst->dst == RINDEX(PC));
    assert(pinst->am_dst == AM_REG_DIRECT);

    assert(pinst->src1 == RINDEX(SP));
    assert(pinst->am_src1 == AM_REG_INDIRECT);

    assert(pinst->src2    == 0);
    assert(pinst->am_src2 == 0);

    PC = cpu_read_mem(SP);
    SP = SP + 4;
}

/*  FIXME: update the flag
format:
    add ri, rj, rk

    add ri, rj, #imm TODO
*/
void op_add(struct __instruction__ *pinst)
{
    assert(pinst->am_dst  == AM_REG_DIRECT);

    assert(pinst->am_src1 == AM_REG_DIRECT);

    assert(pinst->am_src2 == AM_REG_DIRECT);

    R(pinst->dst) = R(pinst->src1) + R(pinst->src2);

    if (R(pinst->dst) == 0x0) {
        FLAG |=  (0x1 << FG_ZERO);
    } else {
        FLAG &= ~(0x1 << FG_ZERO);
    }

    if (R(pinst->dst) < R(pinst->src1) ||
        R(pinst->dst) < R(pinst->src1)) {
        FLAG |=  (0x1 << FG_OVFW);
    } else {
        FLAG &= ~(0x1 << FG_OVFW);
    }

    PC = PC + 4;
}

/*
format:
    sub ri, rj, rk
    sub ri, rj, #imm TODO
*/
void op_sub(struct __instruction__ *pinst)
{
    assert(pinst->am_dst  == AM_REG_DIRECT);

    assert(pinst->am_src1 == AM_REG_DIRECT);

    assert(pinst->am_src2 == AM_REG_DIRECT);

    R(pinst->dst) = R(pinst->src1) - R(pinst->src2);

    if (R(pinst->dst) == 0x0) {
        FLAG |=  (0x1 << FG_ZERO);
    } else {
        FLAG &= ~(0x1 << FG_ZERO);
    }

    if (R(pinst->src1) < R(pinst->src2)) {
        FLAG |=  (0x1 << FG_NEG);
    } else {
        FLAG &= ~(0x1 << FG_NEG);
    }

    PC = PC + 4;
}

/*
format:
    mul ri, rj, rk
    mul ri, rj, #imm TODO
*/
void op_mul(struct __instruction__ *pinst)
{
    assert(pinst->am_dst  == AM_REG_DIRECT);

    assert(pinst->am_src1 == AM_REG_DIRECT);

    assert(pinst->am_src2 == AM_REG_DIRECT);

    R(pinst->dst) = R(pinst->src1) * R(pinst->src2);

    if (R(pinst->dst) == 0x0) {
        FLAG |=  (0x1 << FG_ZERO);
    } else {
        FLAG &= ~(0x1 << FG_ZERO);
    }

    if (R(pinst->dst) < R(pinst->src1) ||
        R(pinst->dst) < R(pinst->src1)) {
        FLAG |=  (0x1 << FG_OVFW);
    } else {
        FLAG &= ~(0x1 << FG_OVFW);
    }

    PC = PC + 4;
}

/*
format:
    div ri, rj, rk
    div ri, rj, #imm TODO
*/
void op_div(struct __instruction__ *pinst)
{
    assert(pinst->am_dst  == AM_REG_DIRECT);

    assert(pinst->am_src1 == AM_REG_DIRECT);

    assert(pinst->am_src2 == AM_REG_DIRECT);
    assert(R(pinst->src2) != 0);

    R(pinst->dst)  = R(pinst->src1) / R(pinst->src2);
    R(pinst->src1) = R(pinst->src1) % R(pinst->src2);

    if (R(pinst->dst) == 0x0) {
        FLAG |=  (0x1 << FG_ZERO);
    } else {
        FLAG &= ~(0x1 << FG_ZERO);
    }

    PC = PC + 4;
}

/*
format:
    and ri, rj, rk
    and ri, rj, #imm TODO
*/
void op_and(struct __instruction__ *pinst)
{
    assert(pinst->am_dst  == AM_REG_DIRECT);

    assert(pinst->am_src1 == AM_REG_DIRECT);

    assert(pinst->am_src2 == AM_REG_DIRECT);

    R(pinst->dst)  = R(pinst->src1) & R(pinst->src2);

    if (R(pinst->dst) == 0x0) {
        FLAG |=  (0x1 << FG_ZERO);
    } else {
        FLAG &= ~(0x1 << FG_ZERO);
    }
    PC = PC + 4;
}

/*
format:
    or ri, rj, rk
    or ri, rj, #imm TODO
*/
void op_or(struct __instruction__ *pinst)
{
    assert(pinst->am_dst  == AM_REG_DIRECT);

    assert(pinst->am_src1 == AM_REG_DIRECT);

    assert(pinst->am_src2 == AM_REG_DIRECT);

    R(pinst->dst)  = R(pinst->src1) | R(pinst->src2);

    if (R(pinst->dst) == 0x0) {
        FLAG |=  (0x1 << FG_ZERO);
    } else {
        FLAG &= ~(0x1 << FG_ZERO);
    }
    PC = PC + 4;
}

/*
format:
    xor ri, rj, rk
    xor ri, rj, #imm TODO
*/
void op_xor(struct __instruction__ *pinst)
{
    assert(pinst->am_dst  == AM_REG_DIRECT);

    assert(pinst->am_src1 == AM_REG_DIRECT);

    assert(pinst->am_src2 == AM_REG_DIRECT);

    R(pinst->dst)  = R(pinst->src1) ^ R(pinst->src2);

    if (R(pinst->dst) == 0x0) {
        FLAG |=  (0x1 << FG_ZERO);
    } else {
        FLAG &= ~(0x1 << FG_ZERO);
    }
    PC = PC + 4;
}

/*
format:
    jmp ri      (mov pc, ri)
    jmp #imm TODO

    jmpn ri     (if (neg) { mov pc, ri })
    jmpn #imm TODO

    jmpz ri
    jmpz #imm TODO

    jmpo ri
    jmpo #imm TODO

    jmpnn ri
    jmpnn #imm TODO

    jmpnz ri
    jmpnz #imm TODO

    jmpno ri
    jmpno #imm TODO
*/
void op_jmp(struct __instruction__ *pinst)
{
    assert(pinst->dst     == RINDEX(PC));
    assert(pinst->am_dst  == AM_REG_DIRECT);

    assert(pinst->am_src1 == AM_REG_DIRECT);

    assert(pinst->src2    == 0);
    assert(pinst->am_src2 == 0);
    switch (pinst->op_type) {
        case (JMP):
            PC = R(pinst->src1);
            break;
        case (JMPN):
            if (get_bit(FLAG, FG_NEG)) {
                PC = R(pinst->src1);
            } else {
                PC = PC + 4;
            }
            break;
        case (JMPZ):
            if (get_bit(FLAG, FG_ZERO)) {
                PC = R(pinst->src1);
            } else {
                PC = PC + 4;
            }
            break;
        case (JMPO):
            if (get_bit(FLAG, FG_OVFW)) {
                PC = R(pinst->src1);
            } else {
                PC = PC + 4;
            }
            break;
        case (JMPNN):
            if (!get_bit(FLAG, FG_NEG)) {
                PC = R(pinst->src1);
            } else {
                PC = PC + 4;
            }
            break;
        case (JMPNZ):
            if (!get_bit(FLAG, FG_ZERO)) {
                PC = R(pinst->src1);
            } else {
                PC = PC + 4;
            }
            break;
        case (JMPNO):
            if (!get_bit(FLAG, FG_OVFW)) {
                PC = R(pinst->src1);
            } else {
                PC = PC + 4;
            }
            break;
        default:
            error();
    }
}

struct __instruction_set__ is[] = {
    {"mov",   MOV,   op_mov},
    {"ldr",   LDR,   op_ldr},
    {"str",   STR,   op_str},

    {"push",  PUSH,  op_push},
    {"pop",   POP,   op_pop},

    {"call",  CALL,  op_call},
    {"ret",   RET,   op_ret},

    {"add",   ADD,   op_add},
    {"sub",   SUB,   op_sub},
    {"mul",   MUL,   op_mul},
    {"div",   DIV,   op_div},
    {"and",   AND,   op_and},
    {"or",    OR,    op_or},
    {"xor",   XOR,   op_xor},

    {"jmp",   JMP,   op_jmp},
    {"jmpn",  JMPN,  op_jmp},
    {"jmpz",  JMPZ,  op_jmp},
    {"jmpo",  JMPO,  op_jmp},
    {"jmpnn", JMPNN, op_jmp},
    {"jmpnz", JMPNZ, op_jmp},
    {"jmpno", JMPNO, op_jmp},
};

/* basic sanity check */
s32 is_legal(struct __instruction__ *pinst)
{
    u32 i;

    if (pinst->reserved != 0) {
        return -1;
    }

    if ( pinst->am_dst  >= AM_MAX
        || pinst->am_src1 >= AM_MAX
        || pinst->am_src2 >= AM_MAX) {
        return -1;
    }

    for(i=0;i<sizeof(is)/sizeof(is[0]);i++) {
        if ((pinst->op_type) == is[i].op_type) {
            return i;
        }
    }
    return -1;
}

void cpu_write_mem(u32 addr, u32 data)
{
    u32 word;
    assert((addr % 4) == 0);
    assert(addr < MEM_SIZE);

    *((u32 *)(&cpu_mem[addr])) = data;
}

u32 cpu_read_mem(u32 addr)
{
    u32 word;
    printf("%x \n", addr);
    assert((addr % 4) == 0);
    assert(addr < MEM_SIZE);
    word = *((u32 *)(&cpu_mem[addr]));

    printf("get 0x%08x\n", word);
    return word;
}

/* run 1 cycle, we always treat that 1 instruction consume 1 cycle */
void cpu_run()
{
    u32 i;
    u32 word;
    struct __instruction__ *pinst;

    /* IF */
    word = cpu_read_mem(PC);
    pinst = (struct __instruction__ *)&word;

    /* ID, EX */
    assert((i = is_legal(pinst)) != -1);
    is[i].hander(pinst);
}

void dump_regs()
{
    u32 i;
    for(i=0;i<4;i++) {
        printf("[R%d]: 0x%08x  ", i, R(i));
    }
    printf("[FLAG]: 0x%08x\n", FLAG);
}

/* simutor my cpu */
int main(int argc, char **argv)
{
    struct stat st;

    if (argc != 2) {
        printf("%s [code.bin]\n", argv[0]);
        exit(-1);
    }

    if ((ifd = open(argv[1], O_RDONLY)) == -1) {
        perror("open");
        exit(-1);
    }

    if (fstat(ifd, &st) == -1) {
        perror("fstat");
        exit(-1);
    }

    printf("file size: %d \n", st.st_size);
    if ((st.st_size % 4) != 0) {
        printf("%s size not multiple by 4 \n");
        exit(-1);
    }

    assert(st.st_size == MEM_SIZE);

    read(ifd, cpu_mem, MEM_SIZE);


    /* PC = cpu_read_mem(RESET_HANDLER); */
    PC = 0x0;
    while (1) {
        cpu_run();
        dump_regs();
        cpu_cycles++;
    }
    return 0;
}
