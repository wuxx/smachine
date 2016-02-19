#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MEM_SIZE (10*1024)
#define error() do {printf("error: [%s][%d]\n", __func__, __LINE__); exit(-1);} while(0)

typedef  unsigned char u8;
typedef  signed   char s8;
typedef  unsigned int  u32;
typedef  signed   int  s32;

typedef void (*op_handler)(u32 op);

enum OP_TYPE_E {
    OP_ADDRESSING = 0,
    OP_STACK      = 1,
    OP_FUNC_CALL  = 2,
    OP_ARITHMETIC = 3,
    OP_JMP        = 4,
};

enum SUB_TYPE_E {

    MOV = OP_ADDRESSING << 8 |  0,
    LDR = OP_ADDRESSING << 8 |  1,
    STR = OP_ADDRESSING << 8 |  2,

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

enum ADDRESS_MODE_E {
    IMM          = 0,
    REG_DIRECT   = 1,
    REG_INDIRECT = 2,
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
    u32 r0;
    u32 r1;
    u32 sp;
    u32 pc;
    u32 flag;
};

struct __instruction_set__ {
    char *desc;
    u32 operation;
    op_handler hander;
};

struct __cpu__ cpu = 
{
    .r0   = 0,
    .r1   = 0,
    .sp   = 0,
    .pc   = 0,
    .flag = 0,
};

u8 cpu_mem[MEM_SIZE] = {0};
int ifd = 0, cpu_cycles = 0;

void op_mov(u32 op)
{
}

void op_ldr(u32 op)
{
}

void op_str(u32 op)
{
}

void op_push(u32 op)
{
}

void op_pop(u32 op)
{
}

void op_call(u32 op)
{
}

void op_ret(u32 op)
{
}

void op_add(u32 op)
{
}

void op_sub(u32 op)
{
}

void op_mul(u32 op)
{
}

void op_div(u32 op)
{
}

void op_and(u32 op)
{
}

void op_or(u32 op)
{
}

void op_xor(u32 op)
{
}

void op_jmp(u32 op)
{
}

void op_jmpn(u32 op)
{
}

void op_jmpz(u32 op)
{
}

void op_jmpo(u32 op)
{
}

void op_jmpnn(u32 op)
{
}

void op_jmpnz(u32 op)
{
}

void op_jmpno(u32 op)
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

s32 is_legal(u32 op)
{
    u32 i;
    for(i=0;i<sizeof(is)/sizeof(is[0]);i++) {
        if ((op & 0xFFFF) == is[i].operation) {
            return 1;
        }
    }
    return 0;
}

u32 cpu_read_mem(u32 addr)
{
    u32 *word;
    assert((addr % 4) == 0);
    assert(addr < MEM_SIZE);
    word = (u32 *)(&cpu_mem[addr]);


    return *word;
}

/* run 1 cycle, we always treat that 1 instruction consume 1 cycle */
void cpu_run()
{
    u32 count;
    s32 op;
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

    while (1) {
        cpu_run();
        cpu_cycles++;
    }
    return 0;
}
