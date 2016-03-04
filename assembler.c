#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "cpu.h"

#if 1
#define DEBUG(fmt, ...)     printf("[%s][%d]" fmt,  __func__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

/*
   the assembler of smachine
*/

#define POOL_SIZE 1024

enum TOKEN_TYPE_E {
    TOKEN_INVALID = 0,
    TOKEN_KEYWORD = 1,
    TOKEN_COMMA   = 2,  
    TOKEN_COLON   = 3,  /* a: jmp a */
    TOKEN_ID      = 4,
    TOKEN_IMM     = 5,
    TOKEN_MAX,
};
char *type_desc[] = {
    "TOKEN_INVALID",
    "TOKEN_KEYWORD",
    "TOKEN_COMMA  ",
    "TOKEN_COLON  ",
    "TOKEN_ID     ",
    "TOKEN_IMM    ",
    "TOKEN_MAX    "
};

struct __token__ {
    u32 type;
    u32 value;
};

struct __id__ {
    char *buf;
    s32 addr;
};

enum KEYWORD_E {
    KW_INVALID = 0,
    KW_MOV,  KW_LDR,  KW_STR,
    KW_PUSH, KW_POP,
    KW_CALL, KW_RET,
    KW_ADD,  KW_SUB,  KW_DIV,  KW_MUL,  KW_AND,   KW_OR,    KW_XOR,
    KW_JMP,  KW_JMPN, KW_JMPZ, KW_JMPO, KW_JMPNN, KW_JMPNZ, KW_JMPNO,
    KW_R0,   KW_R1,   KW_R2,   KW_R3,
    KW_SP, /* ALIAS OF R2 */
    KW_PC, /* ALIAS OF R3 */
    KW_LOCATE, /* LOCATE THE MEM OF INSTRUCTION */
};

char *keyword[] = { "NULL",
                    "mov",  "ldr",  "str",
                    "push", "pop",
                    "call", "ret",
                    "add",  "sub",  "div",  "mul",  "and",   "or",    "xor",
                    "jmp",  "jmpn", "jmpz", "jmpo", "jmpnn", "jmpnz", "jmpno",
                    "r0",   "r1",   "r2",   "r3",
                    "sp", /* alias of r2 */
                    "pc", /* alias of r3 */
                    "LOCATE",   /* locate the mem of instruction */
                    };

u32 cpu_addr = 0;
u8  cpu_mem[MEM_SIZE] = {0}; 
u32 tindex = 0, iindex = 0; /* token index, id index */
struct __token__ token_pool[POOL_SIZE];
struct __id__    id_pool[POOL_SIZE];

u32 _atoi(char *str)
{
    u32 i;
    u32 len; 
    u32 sum = 0;
    len = strlen(str);
    if (len == 0) {
        return 0;
    }   

    if (len >= 2 && str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {    /* hex */
        i = 2;
        while(i < len) {
            switch(str[i]) {

                case ('a'): case ('b'): case ('c'): case ('d'): case ('e'): case ('f'):
                    sum = sum*16 + (str[i] - 'a' + 10);
                    break;

                case ('A'): case ('B'): case ('C'): case ('D'): case ('E'): case ('F'):
                    sum = sum*16 + (str[i] - 'A' + 10);
                    break;

                case ('0'): case ('1'): case ('2'): case ('3'): case ('4'): 
                case ('5'): case ('6'): case ('7'): case ('8'): case ('9'): 
                    sum = sum*16 + (str[i] - '0');
                    break;

                default:
                    return 0;

            }   
            i++;
        }   
    } else {    /* dec */
        i = 0;
        while(i < len) {
            switch(str[i]) {

                case ('0'): case ('1'): case ('2'): case ('3'): case ('4'):
                case ('5'): case ('6'): case ('7'): case ('8'): case ('9'):
                    sum = sum*10 + (str[i] - '0');
                    break;

                default:
                    return 0;

            }
            i++;
        }

    }
    return sum;
}

s32 is_letter(char c)
{
    if (c >= 'a' && c <= 'z' ||
        c >= 'A' && c <= 'Z') {
        return 1;
    } else {
        return 0;
    }
}

s32 is_id(char c)
{
    if ((c >= 'a' && c <= 'z') || 
        (c >= 'A' && c <= 'Z') || 
        (c >= '0' && c <= '9')) {
        return 1;
    } else {
        return 0;
    }
}

s32 is_digit(char c)
{
    if (c >= '0' && c <= '9' ||
        c >= 'A' && c <= 'F' || 
        c >= 'a' && c <= 'f') {
        return 1;
    } else {
        return 0;
    }
}

s32 is_keyword(char *s, u32 len)
{
    u32 i;   
    for(i=0;i<(sizeof(keyword) / sizeof(keyword[0]));i++) {
        if (strncmp(keyword[i], s, len) == 0) {
            return i;
        }
    }
    return 0;
}

s32 put_token(u32 _type, u32 _value)
{
    token_pool[tindex].type  = _type;
    token_pool[tindex].value = _value;
    tindex++;

    assert(tindex < POOL_SIZE);
}

s32 put_id(char *s, u32 len)
{
    u32 i;
    for(i=0;i<iindex;i++) {
        if (strncmp(id_pool[i].buf, s, len-1) == 0) {
            return i;

        }
    }

    id_pool[iindex].buf = malloc(len+1);
    memcpy(id_pool[iindex].buf, s, len);
    id_pool[iindex].buf[len] = '\0';

    id_pool[iindex].addr = -1;
    iindex++;
    assert(iindex < POOL_SIZE);
    return iindex - 1;
}

s32 parse_line(char *line)
{
    u32 i = 0, j;
    u32 radix, num;
    u32 start, end, len;
    char c;
    while((c = line[i]) != 0) {
        DEBUG("%d get [%c] \n", __LINE__, c);
        if (c == ' ') {
            i++;
        } else if (c == ';') {      /* the comment */
            return 0;
        } else if (is_letter(c)) {  /* keyword or id */
            start = i;
            i++;
            while (is_id(c)) {
                c = line[i++];
                DEBUG("%d get [%c] \n", __LINE__, c);
            }
            i--;
            end = i;
            DEBUG("start: %d; end: %d \n", start, end);
            if ((j = is_keyword(&line[start], end-start))) {
                put_token(TOKEN_KEYWORD, j);
            } else { /* id */
                j = put_id(&line[start], end-start);
                put_token(TOKEN_ID, j);
            }

        } else if (c == '#') {      /* immediate num */
            i++;
            start = i;
            if (line[i] == '0' && (line[i+1] == 'x' || line[i+1] == 'X')) { /* hex FIXME: i+1 i+2 may overstep the boundary */
                radix = 16;
                i = i + 2; /* skip '0' 'x' */
            } else { /* dec */
                radix = 10;
            }

            c = line[i++];
            while(is_digit(c)) {
                c = line[i++];
            }
            end = i-1;
            DEBUG("%x [%c] \n", line[end], line[end]);
            line[end] = '\0'; /* for atoi */

            num = _atoi(&line[start]);
            put_token(TOKEN_IMM, num);

        } else if (c == '[') { /* register indirect */
            start = ++i;
            end   = i+2;
            /* [r0|r1|r2|r3|sp|pc] */
            if (j = is_keyword(&line[start], end-start)) {
                put_token(AM_REG_INDIRECT << 16 | TOKEN_KEYWORD, j);    /* high 16bit store the addr mode info */
            } else {
                error();
            }
            i = i + 2;
            assert(line[i++] == ']');
        } else if (c == ',') {
            put_token(TOKEN_COMMA, 0);
            i++;
        } else if (c == ':') {
            put_token(TOKEN_COLON, 0);
            i++;
        } else if (c == '\n') {
            return 0;
        } else {
            printf("c: [%c] \n", c);
            error();
        }
    }
}

s32 parse_token(char *ifile)
{
    FILE *fp;
    char line[1024] = {0};

    if((fp = fopen(ifile, "r")) == NULL)
    { 
        perror("fopen"); 
        exit(-1);
    }

    while (!feof(fp))
    { 
        memset(line, 0, sizeof(line));
        fgets(line,sizeof(line),fp);
        DEBUG("%s\n", line);
        parse_line(line);
    } 

    return 0;

}

s32 dump_token()
{
    u32 i;
    for(i=0;i<POOL_SIZE;i++) {
        if (token_pool[i].type != TOKEN_INVALID) {
            printf("[%d]: [%s] ", i, type_desc[token_pool[i].type & 0xFFFF]);
            if ((token_pool[i].type & 0xFFFF) == TOKEN_KEYWORD) {
                printf("%s \n", keyword[token_pool[i].value]);
            } else {
                printf("0x%x (%d)\n", token_pool[i].value, token_pool[i].value);
            }
        }
    }
    return 0;
}

s32 put_inst(u32 op_type, u32 am_dst, u32 dst, u32 am_src1, u32 src1, u32 am_src2, u32 src2)
{
    struct __instruction__ inst;

    inst.op_type  = op_type;
    inst.reserved = 0;
    inst.src2     = src2;
    inst.am_src2  = am_src2;
    inst.src1     = src1;
    inst.am_src1  = am_src1;
    inst.dst      = dst;
    inst.am_dst   = am_dst;

    assert(sizeof(inst) == 4);
    memcpy(&cpu_mem[cpu_addr], &inst, 4);
    cpu_addr += 4;

    return 0;
}

s32 put_word(u32 word)
{
    memcpy(&cpu_mem[cpu_addr], &word, 4);
    cpu_addr += 4;
}

u32 get_operand(u32 index)
{
    u32 operand;
    switch (token_pool[index].value) {
        case (KW_R0):
            operand = 0;
            break;
        case (KW_R1):
            operand = 1;
            break;
        case (KW_R2):
            operand = 2;
            break;
        case (KW_R3):
            operand = 3;
            break;
        case (KW_SP):
            operand = 2;
            break;
        case (KW_PC):
            operand = 3;
            break;
        default:
            printf("%d error value %d \n",  __LINE__, token_pool[index].value);
            error();
            break;
    }
    return operand;
}

s32 op_mov()
{
    u32 op_type, am_dst, dst, am_src1, src1, am_src2, src2;
    u32 imm = 0;

    op_type = MOV;

    /* dst */
    am_dst = AM_REG_DIRECT;
    dst    = get_operand(tindex+1);

    assert(token_pool[tindex+2].type == TOKEN_COMMA);

    /* src1 */
    if (token_pool[tindex+3].type == TOKEN_IMM) {
        am_src1 = AM_IMM;
        src1    = 0;
        imm     = token_pool[tindex+3].value;
    } else {
        am_src1 = AM_REG_DIRECT;
        src1 = get_operand(tindex+3);
    }

    /* src2 */
    am_src2 = 0;
    src2    = 0;

    put_inst(op_type, am_dst, dst, am_src1, src1, am_src2, src2);
    if (am_src1 == AM_IMM) {
        put_word(imm);
    }
    tindex += 4;
    return 0;
}

s32 op_ldr()
{
    u32 op_type, am_dst, dst, am_src1, src1, am_src2, src2;

    op_type = LDR;

    am_dst = AM_REG_DIRECT;
    dst    = get_operand(tindex+1);

    assert(token_pool[tindex+2].type == TOKEN_COMMA);
    assert((token_pool[tindex+3].type >> 16) == AM_REG_INDIRECT);

    am_src1 = AM_REG_INDIRECT;
    src1    = get_operand(tindex+3);

    am_src2 = 0;
    src2    = 0;

    put_inst(op_type, am_dst, dst, am_src1, src1, am_src2, src2);
    tindex += 4;
    return 0;
}

s32 op_str()
{
    u32 op_type, am_dst, dst, am_src1, src1, am_src2, src2;

    op_type = LDR;

    am_src1 = AM_REG_DIRECT;
    src1    = get_operand(tindex+1);

    assert(token_pool[tindex+2].type == TOKEN_COMMA);

    assert((token_pool[tindex+3].type >> 16) == AM_REG_INDIRECT);

    am_dst  = AM_REG_INDIRECT;
    dst     = get_operand(tindex+3);

    am_src2 = 0;
    src2    = 0;

    put_inst(op_type, am_dst, dst, am_src1, src1, am_src2, src2);
    tindex += 4;
    return 0;
}

s32 op_push()
{
    u32 op_type, am_dst, dst, am_src1, src1, am_src2, src2;

    op_type = PUSH;

    am_dst  = AM_REG_INDIRECT;
    dst     = 2; /* SP */

    am_src1 = AM_REG_DIRECT;
    src1    = get_operand(tindex+1);

    am_src2 = 0;
    src2    = 0;

    put_inst(op_type, am_dst, dst, am_src1, src1, am_src2, src2);
    tindex += 2;
    return 0;
}

s32 op_pop()
{
    u32 op_type, am_dst, dst, am_src1, src1, am_src2, src2;

    op_type = POP;

    am_dst = AM_REG_DIRECT;
    dst    = get_operand(tindex+1);

    am_src1  = AM_REG_INDIRECT;
    src1     = 2; /* SP */

    am_src2 = 0;
    src2    = 0;

    put_inst(op_type, am_dst, dst, am_src1, src1, am_src2, src2);
    tindex += 2;
    return 0;
}

s32 op_call()
{
    u32 op_type, am_dst, dst, am_src1, src1, am_src2, src2;

    op_type = CALL;

    am_dst  = AM_REG_DIRECT;
    dst     = 3; /* PC */

    am_src1 = AM_REG_DIRECT;
    src1    = get_operand(tindex+1);

    am_src2 = 0;
    src2    = 0;

    put_inst(op_type, am_dst, dst, am_src1, src1, am_src2, src2);
    tindex += 2;
    return 0;
}

s32 op_ret()
{
    u32 op_type, am_dst, dst, am_src1, src1, am_src2, src2;

    op_type = RET;

    am_dst  = AM_REG_DIRECT;
    dst     = 3; /* PC */

    am_src1 = AM_REG_INDIRECT;
    src1    = 2; /* SP */

    am_src2 = 0;
    src2    = 0;

    put_inst(op_type, am_dst, dst, am_src1, src1, am_src2, src2);
    tindex += 1;
    return 0;
}

/* arithmetical logic  */
s32 op_al(u32 type)
{
    u32 op_type, am_dst, dst, am_src1, src1, am_src2, src2;

    switch (type) {
        case (KW_ADD):
            op_type = ADD;
            break;
        case (KW_SUB):
            op_type = SUB;
            break;
        case (KW_MUL):
            op_type = MUL;
            break;
        case (KW_DIV):
            op_type = DIV;
            break;
        case (KW_AND):
            op_type = AND;
            break;
        case (KW_OR):
            op_type = OR;
            break;
        case (KW_XOR):
            op_type = XOR;
            break;
        default:
            DEBUG("type: %x \n", type);
            error();
        break;
    }

    am_dst  = AM_REG_DIRECT;
    dst     = get_operand(tindex+1);

    assert(token_pool[tindex+2].type == TOKEN_COMMA);

    am_src1 = AM_REG_DIRECT;
    src1    = get_operand(tindex+3);

    assert(token_pool[tindex+4].type == TOKEN_COMMA);
    am_src2 = AM_REG_DIRECT;
    src2    = get_operand(tindex+5);

    put_inst(op_type, am_dst, dst, am_src1, src1, am_src2, src2);
    tindex += 6;
    return 0;
}

s32 op_jmp(u32 type)
{
    u32 imm;
    u32 op_type, am_dst, dst, am_src1, src1, am_src2, src2;

    switch (type) {
        case (KW_JMP):
            op_type = JMP;
            break;
        case (KW_JMPN):
            op_type = JMPN;
            break;
        case (KW_JMPZ):
            op_type = JMPZ;
            break;
        case (KW_JMPO):
            op_type = JMPO;
            break;
        case (KW_JMPNN):
            op_type = JMPNN;
            break;
        case (KW_JMPNZ):
            op_type = JMPNZ;
            break;
        case (KW_JMPNO):
            op_type = JMPNO;
            break;
        default:
            error();
        break;
    }

    am_dst  = AM_REG_DIRECT;
    dst     = 3; /* PC */

    if (token_pool[tindex+1].type == TOKEN_IMM) {
        src1    = 0;
        am_src1 = AM_IMM;
        imm     = token_pool[tindex+1].value;
    } else if (token_pool[tindex+1].type == TOKEN_KEYWORD) {
        am_src1 = AM_REG_DIRECT;
        src1    = get_operand(tindex+1);
    }  else if (token_pool[tindex+1].type == TOKEN_ID) {
        src1    = 0;
        am_src1 = AM_IMM;
        imm     = id_pool[token_pool[tindex+1].value].addr;
    }

    am_src2 = 0;
    src2    = 0;

    put_inst(op_type, am_dst, dst, am_src1, src1, am_src2, src2);
    if (am_src1 == AM_IMM) {
        put_word(imm);
    }

    tindex += 2;
    return 0;
}

/* pseudo instruction */
s32 op_locate()
{
    assert(token_pool[tindex+1].type == TOKEN_IMM);
    cpu_addr = token_pool[tindex+1].value;
    assert(cpu_addr < MEM_SIZE && (cpu_addr % 4 == 0));
    tindex += 2;
}

s32 gen_code()
{
    u32 i;
    s32 op_type = -1;
    for(tindex=0;tindex<POOL_SIZE;) {
        DEBUG(" %d type: %s; value: %d\n", tindex, type_desc[token_pool[tindex].type], token_pool[tindex].value);

        switch (token_pool[tindex].type) {
            case (TOKEN_INVALID):
                return 0;
            case (TOKEN_COMMA):
            case (TOKEN_IMM):
                error();
            case (TOKEN_ID):
                i = token_pool[tindex].value;
                assert(id_pool[i].addr == -1);

                id_pool[i].addr = cpu_addr;
                printf("[%s]: 0x%08x\n", id_pool[i].buf, cpu_addr);
                assert(token_pool[tindex+1].type ==TOKEN_COLON);
                tindex += 2;
                break;

            case (TOKEN_KEYWORD):
                switch (token_pool[tindex].value)  {
                    case (KW_MOV):
                        op_mov();
                        break;
                    case (KW_LDR):
                        op_ldr();
                        break;
                    case (KW_STR):
                        op_str();
                        break;
                    case (KW_PUSH):
                        op_push();
                        break;
                    case (KW_POP):
                        op_pop();
                        break;
                    case (KW_CALL):
                        op_call();
                        break;
                    case (KW_RET):
                        op_ret();
                        break;

                    case (KW_ADD):
                    case (KW_SUB):
                    case (KW_DIV):
                    case (KW_MUL):
                    case (KW_AND):
                    case (KW_OR):
                    case (KW_XOR):
                        op_al(token_pool[tindex].value);
                        break;

                    case (KW_JMP):
                    case (KW_JMPN):
                    case (KW_JMPZ):
                    case (KW_JMPO):
                    case (KW_JMPNN):
                    case (KW_JMPNZ):
                    case (KW_JMPNO):
                        op_jmp(token_pool[tindex].value);
                        break;
                    case (KW_LOCATE):
                        op_locate();
                        break;
                }
        }
    }

    return 0;
}

s32 dump_cpu_mem()
{
    u32 i;
    u32 *p;

    p = (u32*)(&cpu_mem[0]);

    for(i=0;i<sizeof(cpu_mem)/4;i++) {
        printf("[0x%08x]: 0x%08x\n", i*4, p[i]);
    }
    return 0;
}

int main(int argc, char **argv)
{
    int ofd;
    if (argc != 3) {
        printf("%s [foo.s] [bar.bin]\n", argv[0]);
        exit(-1);
    }

    if ((ofd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0664)) == -1) {
        perror("open");
    }

    parse_token(argv[1]);
    dump_token();
    gen_code();
    dump_cpu_mem();
    write(ofd, cpu_mem, sizeof(cpu_mem));
    return 0;
}
