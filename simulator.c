#include <stdio.h>

typedef  unsigned int u32;
typedef  signed   int s32;

enum op_type {
    OP_ADDRESSING = 0,
    OP_STACK      = 1,
    OP_FUNC_CALL  = 2,
    OP_ARITHMETIC = 3,
    OP_JMP        = 4,
};

enum sub_type {

    MOV = OP_ADDRESSING << 8 |  0,
    LDR = OP_ADDRESSING << 8 |  1,
    STR = OP_ADDRESSING << 8 |  2,

    PUSH = OP_STACK << 8 | 3,
    POP  = OP_STACK << 8 | 4,

    CALL = OP_FUNC_CALL << 8 |  5,
    RET  = OP_FUNC_CALL << 8 |  6,

    ADD = OP_ARITHMETIC << 8 |  7,
    DIV = OP_ARITHMETIC << 8 |  8,
    SUB = OP_ARITHMETIC << 8 |  9,
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
    u32 sub_type: 23;
    u32 op_src2:   2;
    u32 op_src1:   2;
    u32 op_dst:    2;
    u32 op_type:   3;
};

struct __cpu__ {
    u32 r0;
    u32 r1;
    u32 sp;
    u32 pc;
    u32 flag;
};

/* simutor my cpu */
int main()
{
    return 0;
}
