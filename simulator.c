#include <stdio.h>
#include <assert.h>

typedef  unsigned int u32;
typedef  signed   int s32;

typedef void (*op_handler)(u32 op);

enum OP_TYPE_E {
    OP_ADDRESSING = 0,
    OP_STACK      = 1,
    OP_FUNC_CALL  = 2,
    OP_ARITHMETIC = 3,
    OP_JMP        = 4,
};

enum SUB_TYPE_E {

    MOV = OP_ADDRESSING << 16 |  0,
    LDR = OP_ADDRESSING << 16 |  1,
    STR = OP_ADDRESSING << 16 |  2,

    PUSH = OP_STACK << 16 | 3,
    POP  = OP_STACK << 16 | 4,

    CALL = OP_FUNC_CALL << 16 |  5,
    RET  = OP_FUNC_CALL << 16 |  6,

    ADD = OP_ARITHMETIC << 16 |  7,
    SUB = OP_ARITHMETIC << 16 |  9,
    DIV = OP_ARITHMETIC << 16 |  8,
    MUL = OP_ARITHMETIC << 16 | 10,
    AND = OP_ARITHMETIC << 16 | 11,
    OR  = OP_ARITHMETIC << 16 | 12,
    XOR = OP_ARITHMETIC << 16 | 13,

    JMP   = OP_JMP << 16 | 14,
    JMPN  = OP_JMP << 16 | 15,
    JMPZ  = OP_JMP << 16 | 16,
    JMPO  = OP_JMP << 16 | 17,
    JMPNN = OP_JMP << 16 | 18,
    JMPNZ = OP_JMP << 16 | 19,
    JMPNO = OP_JMP << 16 | 20,
    
};

enum ADDRESS_MODE_E {
    IMM          = 0,
    REG_DIRECT   = 1,
    REG_INDIRECT = 2,
};

struct __instruction__ {
    u32 sub_type: 16;
    u32 op_type:   3;
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
    {"", MOV, op_mov},
    {"", LDR, op_ldr},
    {"", STR, op_str},

    {"", PUSH, op_push},
    {"", POP,  op_pop},

    {"", CALL, op_call},
    {"", RET,  op_ret},

    {"", ADD, op_add},
    {"", SUB, op_sub},
    {"", MUL, op_mul},
    {"", DIV, op_div},
    {"", AND, op_and},
    {"", OR,  op_or},
    {"", XOR, op_xor},

    {"", JMP,   op_jmp},
    {"", JMPN,  op_jmpn},
    {"", JMPZ,  op_jmpz},
    {"", JMPO,  op_jmpo},
    {"", JMPNN, op_jmpnn},
    {"", JMPNZ, op_jmpnz},
    {"", JMPNO, op_jmpno},
};

s32 is_legal(u32 op)
{
    return 0;
}

u32 get_word()
{
    return 0;
}

/* simutor my cpu */
int main()
{
    return 0;
}
