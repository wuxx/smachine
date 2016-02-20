#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MEM_SIZE (10*1024)
#define error() do {printf("error: [%s][%d]\n", __func__, __LINE__); exit(-1);} while(0)
#define inst_illegal(pinst)  do {printf("[%s][%d] illegal instruction [%x]\n", __func__, __LINE__, *((u32*)pinst)); exit(-1);} while(0)

#define R(i)  (cpu.r[i])

#define R0 (cpu.r[0])
#define R1 (cpu.r[1])
#define SP (cpu.r[2])
#define PC (cpu.r[3])
#define FLAG (cpu.flag)

typedef  unsigned char u8;
typedef  signed   char s8;
typedef  unsigned int  u32;
typedef  signed   int  s32;

u32 cpu_read_mem(u32 addr);

enum ADDRESS_MODE_E {
    IMM          = 0,
    REG_DIRECT   = 1,
    REG_INDIRECT = 2,
    AM_MAX,
};

enum OP_TYPE_E {
    OP_DATA_TRANSFER = 0,
    OP_STACK         = 1,
    OP_FUNC_CALL     = 2,
    OP_ARITHMETIC    = 3,
    OP_JMP           = 4,
};

enum VIC_TABLE_E {
    RESET_HANDLER = 0x0,
    IRQ_HANDLER   = 0x4,
    EXC_HANDLER   = 0x8,
};

enum SUB_TYPE_E {

    MOV = OP_DATA_TRANSFER << 8 |  0,
    LDR = OP_DATA_TRANSFER << 8 |  1,
    STR = OP_DATA_TRANSFER << 8 |  2,

    PUSH = OP_STACK << 8 | 3,
    POP  = OP_STACK << 8 | 4,

    CALL = OP_FUNC_CALL << 8 |  5,
    RET  = OP_FUNC_CALL << 8 |  6,

    ADD = OP_ARITHMETIC << 8 |  7,
    SUB = OP_ARITHMETIC << 8 |  9,
    DIV = OP_ARITHMETIC << 8 |  8,
    MUL = OP_ARITHMETIC << 8 | 10,
    AND = OP_ARITHMETIC << 8 | 11,
    OR  = OP_ARITHMETIC << 8 | 12,
    XOR = OP_ARITHMETIC << 8 | 13,

    JMP   = OP_JMP << 8 | 14,
    JMPN  = OP_JMP << 8 | 15,
    JMPZ  = OP_JMP << 8 | 16,
    JMPO  = OP_JMP << 8 | 17,
    JMPNN = OP_JMP << 8 | 18,
    JMPNZ = OP_JMP << 8 | 19,
    JMPNO = OP_JMP << 8 | 20,
    
};

struct __instruction__ {
    u32 op_type:  16;
    u32 reserved:  4;
    u32 src2:      2;
    u32 am_src2:   2;
    u32 src1:      2;
    u32 am_src1:   2;
    u32 dst:       2;
    u32 am_dst:    2;
};

struct __cpu__ {
    u32 r[4];
    u32 flag;
};

typedef void (*op_handler)(struct __instruction__ *inst);

struct __instruction_set__ {
    char *desc;
    u32 op_type;
    op_handler hander;
};

struct __cpu__ cpu = 
{
    .r    = {0, 0, 0, 0},
    .flag = 0,
};

u8 cpu_mem[MEM_SIZE] = {0};
int ifd = 0, cpu_cycles = 0;


/*
format:
   mov ri, rj
   mov ri, #imm
*/
void op_mov(struct __instruction__ *pinst)
{
    u32 imm;
    assert(pinst->src2 == 0);
    assert(pinst->am_src2 == 0);

    assert(pinst->am_dst == REG_DIRECT);

    switch (pinst->am_src1) {
        case (IMM):
            imm = cpu_read_mem(PC + 4);
            R(pinst->dst) = imm;
            PC = PC + 8;
            break;
        case (REG_DIRECT):
            R(pinst->dst) = R(pinst->src1);
            PC = PC + 4;
            break;
        case (REG_INDIRECT):
            inst_illegal(pinst);
            break;
        default:
            inst_illegal(pinst);
            break;
    }
}

/*
format:
    ldr ri, [rj]
*/
void op_ldr(struct __instruction__ *pinst)
{
}

/*
format:
    str ri, [rj]
*/
void op_str(struct __instruction__ *pinst)
{
}

/*
format:
    push ri
*/
void op_push(struct __instruction__ *pinst)
{
}

/*
format:
    pop ri
*/
void op_pop(struct __instruction__ *pinst)
{
}

/*
format:
    call ri
    call #imm
*/
void op_call(struct __instruction__ *pinst)
{
}

/*
format:
    ret
*/
void op_ret(struct __instruction__ *pinst)
{
}

/*
format:
    add ri, rj, rk
    add ri, rj, #imm
*/
void op_add(struct __instruction__ *pinst)
{
}

/*
format:
    sub ri, rj, rk
    sub ri, rj, #imm
*/
void op_sub(struct __instruction__ *pinst)
{
}

/*
format:
    mul ri, rj, rk
    mul ri, rj, #imm
*/
void op_mul(struct __instruction__ *pinst)
{
}

/*
format:
    div ri, rj, rk
    div ri, rj, #imm
*/
void op_div(struct __instruction__ *pinst)
{
}

/*
format:
    and ri, rj, rk
    and ri, rj, #imm
*/
void op_and(struct __instruction__ *pinst)
{
}

/*
format:
    or ri, rj, rk
    or ri, rj, #imm
*/
void op_or(struct __instruction__ *pinst)
{
}

/*
format:
    xor ri, rj, rk
    xor ri, rj, #imm
*/
void op_xor(struct __instruction__ *pinst)
{
}

/*
format:
    jmp ri
    jmp #imm
*/
void op_jmp(struct __instruction__ *pinst)
{
}

/*
format:
    jmpn ri
    jmpn #imm
*/
void op_jmpn(struct __instruction__ *pinst)
{
}

/*
format:
    jmpz ri
    jmpz #imm
*/
void op_jmpz(struct __instruction__ *pinst)
{
}

/*
format:
    jmpo ri
    jmpo #imm
*/
void op_jmpo(struct __instruction__ *pinst)
{
}

/*
format:
    jmpnn ri
    jmpnn #imm
*/
void op_jmpnn(struct __instruction__ *pinst)
{
}

/*
format:
    jmpnz ri
    jmpnz #imm
*/
void op_jmpnz(struct __instruction__ *pinst)
{
}

/*
format:
    jmpno ri
    jmpno #imm
*/
void op_jmpno(struct __instruction__ *pinst)
{
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
    {"jmpn",  JMPN,  op_jmpn},
    {"jmpz",  JMPZ,  op_jmpz},
    {"jmpo",  JMPO,  op_jmpo},
    {"jmpnn", JMPNN, op_jmpnn},
    {"jmpnz", JMPNZ, op_jmpnz},
    {"jmpno", JMPNO, op_jmpno},
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

u32 cpu_read_mem(u32 addr)
{
    u32 word;
    assert((addr % 4) == 0);
    assert(addr < MEM_SIZE);
    word = *((u32 *)(&cpu_mem[addr]));

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

    assert(st.st_size < MEM_SIZE);

    PC = cpu_read_mem(RESET_HANDLER);
    while (1) {
        cpu_run();
        cpu_cycles++;
    }
    return 0;
}
