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
    .r    = {0, 0, 0, 0, 0},
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
    ldr ri, [rj, #offset]
    ldr ri, [rj] (process as ldr ri, [rj, #0])
*/
void op_ldr(struct __instruction__ *pinst)
{
    u32 data;
    u32 offset;

    assert(pinst->src2 == 0);
    assert(pinst->am_src2 == 0);

    assert(pinst->am_dst == AM_REG_DIRECT);

    offset = cpu_read_mem(PC + 4);

    switch (pinst->am_src1) {
        case (AM_REG_INDIRECT):
            data = cpu_read_mem(R(pinst->src1) + offset);
            R(pinst->dst) = data;
            break;
        default:
            inst_illegal(pinst);
            break;
    }

    PC = PC + 8;
}

/*
format: op src, dst
    str ri, [rj, #offset] (rj is op_dst, ri is op_src1)
    str ri, [rj]  (process as str ri, [rj, #0])
*/
void op_str(struct __instruction__ *pinst)
{
    u32 addr, data;
    u32 offset;

    assert(pinst->src2 == 0);
    assert(pinst->am_src2 == 0);

    assert(pinst->am_src1 == AM_REG_DIRECT);
    assert(pinst->am_dst  == AM_REG_INDIRECT);

    addr = R(pinst->dst);
    data = R(pinst->src1);
    offset = cpu_read_mem(PC + 4);

    cpu_write_mem(addr + offset, data);

    PC = PC + 8;
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

    SP = SP - 4;

    addr = SP;
    data = R(pinst->src1);

    cpu_write_mem(addr, data);
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

    R(pinst->dst) = cpu_read_mem(SP);

    SP = SP + 4;
    PC = PC + 4;

}

/*
format: op, src1
    call ri
equal:
    push (PC + 4)
    mov pc, ri

    call #imm   TODO
equal:
    push (PC + 8)
    mov pc #imm
*/
void op_call(struct __instruction__ *pinst)
{
    u32 addr, data;

    assert(pinst->dst == RINDEX(PC));
    assert(pinst->am_dst == AM_REG_DIRECT);

    assert(pinst->src2    == 0);
    assert(pinst->am_src2 == 0);

    SP = SP - 4;
    switch (pinst->am_src1) {
        case (AM_REG_DIRECT):
            cpu_write_mem(SP, PC + 4);
            PC = R(pinst->src1);
            break;
        case (AM_IMM):
            cpu_write_mem(SP, PC + 8);
            PC = cpu_read_mem(PC + 4);
            break;
        default:
            inst_illegal(pinst);
            break;
    }

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

void update_flag(u32 n, u32 z, u32 o)
{
    FLAG = n << FG_NEG_BIT | z << FG_ZERO_BIT | o << FG_OVFW_BIT; 
}

/* 
format:
    add ri, rj, rk
    add ri, rj, #imm 

    sub ri, rj, rk
    sub ri, rj, #imm 

    mul ri, rj, rk
    mul ri, rj, #imm 

    div ri, rj, rk
    div ri, rj, #imm 

    and ri, rj, rk
    and ri, rj, #imm 

    or ri, rj, rk
    or ri, rj, #imm

    xor ri, rj, rk
    xor ri, rj, #imm
*/
void op_alu(struct __instruction__ *pinst)
{
    u32 imm;
    u32 flag_n = 0, flag_z = 0, flag_o = 0;

    assert(pinst->am_dst  == AM_REG_DIRECT);

    assert(pinst->am_src1 == AM_REG_DIRECT);

    switch (pinst->am_src2) {
        case (AM_IMM):
            imm = cpu_read_mem(PC + 4);
            PC = PC + 4;
            break;
        case (AM_REG_DIRECT):
            imm = R(pinst->src2);   /* treat as imm */
            break;
        default:
            error();
    }

    switch (pinst->op_type) {
        case (ADD):
            R(pinst->dst) = R(pinst->src1) + imm;

            if (R(pinst->dst) == 0x0) {
                flag_z = 1;
            }

            if (R(pinst->dst) < R(pinst->src1) ||
                R(pinst->dst) < R(pinst->src1)) {
                flag_o = 1;
            }
            break;
        case (SUB):
            R(pinst->dst) = R(pinst->src1) - imm;

            if (R(pinst->dst) == 0x0) {
                flag_z = 1;
            }

            if (R(pinst->src1) < R(pinst->src2)) {
                flag_n = 1;
            }

            break;
        case (MUL):
            R(pinst->dst) = R(pinst->src1) * imm;

            if (R(pinst->dst) == 0x0) {
                flag_z = 1;
            }

            if (R(pinst->dst) < R(pinst->src1) ||
                    R(pinst->dst) < R(pinst->src1)) {
                flag_o = 1;
            }
            break;
        case (DIV):
            assert(R(pinst->src2) != 0);

            R(pinst->dst)  = R(pinst->src1) / imm;
            R(pinst->src1) = R(pinst->src1) % imm;

            if (R(pinst->dst) == 0x0) {
                flag_z = 1;
            }
            break;
        case (AND):
            R(pinst->dst)  = R(pinst->src1) & imm;

            if (R(pinst->dst) == 0x0) {
                flag_z = 1;
            }
            break;
        case (OR):
            R(pinst->dst)  = R(pinst->src1) | imm;

            if (R(pinst->dst) == 0x0) {
                flag_z = 1;
            }
            break;
        case (XOR):
            R(pinst->dst)  = R(pinst->src1) ^ imm;

            if (R(pinst->dst) == 0x0) {
                flag_z = 1;
            }
            break;
        default:
            error();
    }

    update_flag(flag_n, flag_z, flag_o);
    PC   = PC + 4;
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
    u32 imm;

    assert(pinst->dst     == RINDEX(PC));
    assert(pinst->am_dst  == AM_REG_DIRECT);

    assert(pinst->src2    == 0);
    assert(pinst->am_src2 == 0);

    assert(pinst->am_src1 == AM_REG_DIRECT || pinst->am_src1 == AM_IMM);
    if (pinst->am_src1 == AM_IMM) {
        assert(pinst->src1 == 0);
        imm = cpu_read_mem(PC + 4);
        assert(((imm % 4) == 0) && (imm < MEM_SIZE));
        PC = PC + 4;
    } else {
        imm = R(pinst->src1);
    }

    switch (pinst->op_type) {
        case (JMP):
            PC = imm;
            break;
        case (JMPN):
            if (get_bit(FLAG, FG_NEG_BIT)) {
                PC = imm;
            } else {
                PC = PC + 4;
            }
            break;
        case (JMPZ):
            if (get_bit(FLAG, FG_ZERO_BIT)) {
                PC = imm;
            } else {
                PC = PC + 4;
            }
            break;
        case (JMPO):
            if (get_bit(FLAG, FG_OVFW_BIT)) {
                PC = imm;
            } else {
                PC = PC + 4;
            }
            break;
        case (JMPNN):
            if (!get_bit(FLAG, FG_NEG_BIT)) {
                PC = imm;
            } else {
                PC = PC + 4;
            }
            break;
        case (JMPNZ):
            if (!get_bit(FLAG, FG_ZERO_BIT)) {
                PC = imm;
            } else {
                PC = PC + 4;
            }
            break;
        case (JMPNO):
            if (!get_bit(FLAG, FG_OVFW_BIT)) {
                PC = imm;
            } else {
                PC = PC + 4;
            }
            break;
        default:
            error();
    }
}

void smachine_exit()
{
    printf("smachine exit, cycles: %d;  R0: 0x%08x (%d)\n", cpu_cycles, R0, R0);
    exit(R0);
}

void op_halt(struct __instruction__ *pinst)
{
    assert(pinst->src1    == 0);
    assert(pinst->am_src1 == 0);
    assert(pinst->src2    == 0);
    assert(pinst->am_src2 == 0);
    assert(pinst->dst     == 0);
    assert(pinst->am_dst  == 0);
    smachine_exit(); 
}


struct __instruction_set__ is[] = {
    {"mov",   MOV,   op_mov},
    {"ldr",   LDR,   op_ldr},
    {"str",   STR,   op_str},

    {"push",  PUSH,  op_push},
    {"pop",   POP,   op_pop},

    {"call",  CALL,  op_call},
    {"ret",   RET,   op_ret},

    {"add",   ADD,   op_alu},
    {"sub",   SUB,   op_alu},
    {"mul",   MUL,   op_alu},
    {"div",   DIV,   op_alu},
    {"and",   AND,   op_alu},
    {"or",    OR,    op_alu},
    {"xor",   XOR,   op_alu},

    {"jmp",   JMP,   op_jmp},
    {"jmpn",  JMPN,  op_jmp},
    {"jmpz",  JMPZ,  op_jmp},
    {"jmpo",  JMPO,  op_jmp},
    {"jmpnn", JMPNN, op_jmp},
    {"jmpnz", JMPNZ, op_jmp},
    {"jmpno", JMPNO, op_jmp},

    {"halt",  HALT,  op_halt},
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
    assert((addr % 4) == 0);
    assert(addr < MEM_SIZE);
    word = *((u32 *)(&cpu_mem[addr]));

    printf("[0x%08x]: 0x%08x\n", addr, word);
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
    is[i].handler(pinst);
}

void dump_regs()
{
    u32 i;
    printf("[R0]: 0x%08x  ", R(0));
    printf("[R1]: 0x%08x  ", R(1));
    printf("[FP]: 0x%08x  ", R(2));
    printf("[SP]: 0x%08x  ", R(3));
    printf("[PC]: 0x%08x  ", R(4));
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

    if ((st.st_size % 4) != 0) {
        printf("%s size not multiple by 4 \n");
        exit(-1);
    }

    assert(st.st_size == MEM_SIZE);

    read(ifd, cpu_mem, MEM_SIZE);


    PC = RESET_HANDLER;
    PC = 0x0;
    printf("smachine start\n");
    while (1) {
        cpu_run();
        dump_regs();
        cpu_cycles++;
    }
    return 0;
}
