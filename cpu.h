#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#if 1
#define DEBUG(fmt, ...)     printf("[%s][%d]" fmt,  __func__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

#define error(fmt, ...)  do { \
                        printf("error in [%s][%d]:["fmt"], exit.\n", __func__, __LINE__, ##__VA_ARGS__);  \
                        exit(-1); \
                    } while(0)

#define inst_illegal(pinst)  do { \
                                    printf("[%s][%d] illegal instruction [%x]\n", __func__, __LINE__, *((u32*)pinst));  \
                                    exit(-1); \
                                } while(0)


#define MEM_SIZE (0x4000)

#define get_bit(x, bit_index) ((x >> bit_index) & 0x1)

#define R(i)  (cpu.r[i])

#define R0 cpu.r[0]
#define R1 cpu.r[1]
#define R2 cpu.r[2]
#define R3 cpu.r[3]
#define R4 cpu.r[4]

/* alias */
#define FP cpu.r[2]
#define SP cpu.r[3]
#define PC cpu.r[4]

#define FLAG (cpu.flag)

#define RINDEX(rx) ((&rx - &cpu.r[0]))

typedef  unsigned char u8;
typedef  signed   char s8;
typedef  unsigned int  u32;
typedef  signed   int  s32;

u32 cpu_read_mem(u32 addr);
void cpu_write_mem(u32 addr, u32 data);

enum ADDRESS_MODE_E {
    AM_IMM          = 0,
    AM_REG_DIRECT   = 1,
    AM_REG_INDIRECT = 2,
    AM_MAX,
};

enum FLAG_E {
    FG_NEG_BIT  = 0,
    FG_ZERO_BIT = 1,
    FG_OVFW_BIT = 2,
    FG_MAX,
};

enum OP_TYPE_E {
    OP_DATA_TRANSFER = 0,
    OP_STACK         = 1,
    OP_FUNC_CALL     = 2,
    OP_ALU           = 3,
    OP_JMP           = 4,
    OP_SYS_CTRL      = 5,
};

enum VIC_TABLE_E {
    RESET_HANDLER = 0x0,
    IRQ_HANDLER   = 0x4,
    EXC_HANDLER   = 0x8,
};

enum SUB_TYPE_E {

    MOV   = OP_DATA_TRANSFER << 8 |  0,
    LDR   = OP_DATA_TRANSFER << 8 |  1,
    STR   = OP_DATA_TRANSFER << 8 |  2,
    LDRB  = OP_DATA_TRANSFER << 8 |  3,
    STRB  = OP_DATA_TRANSFER << 8 |  4,

    PUSH  = OP_STACK << 8 | 3,
    POP   = OP_STACK << 8 | 4,

    CALL  = OP_FUNC_CALL << 8 |  5,
    RET   = OP_FUNC_CALL << 8 |  6,

    ADD   = OP_ALU << 8 |  7,
    SUB   = OP_ALU << 8 |  9,
    DIV   = OP_ALU << 8 |  8,
    MUL   = OP_ALU << 8 | 10,
    AND   = OP_ALU << 8 | 11,
    OR    = OP_ALU << 8 | 12,
    XOR   = OP_ALU << 8 | 13,
    LOL   = OP_ALU << 8 | 14,
    LOR   = OP_ALU << 8 | 15,

    JMP   = OP_JMP << 8 | 14,
    JMPN  = OP_JMP << 8 | 15,
    JMPZ  = OP_JMP << 8 | 16,
    JMPO  = OP_JMP << 8 | 17,
    JMPNN = OP_JMP << 8 | 18,
    JMPNZ = OP_JMP << 8 | 19,
    JMPNO = OP_JMP << 8 | 20,

    HALT  = OP_SYS_CTRL << 8 | 21,
    
};

struct __instruction__ {
    u32 dst:       3;
    u32 am_dst:    2;
    u32 src1:      3;
    u32 am_src1:   2;
    u32 src2:      3;
    u32 am_src2:   2;
    u32 reserved:  1;
    u32 op_type:  16;
};

struct __cpu__ {
    u32 r[5];
    u32 flag;
};

typedef void (*op_handler)(struct __instruction__ *inst);

struct __instruction_set__ {
    char *desc;
    u32 op_type;
    op_handler handler;
};

