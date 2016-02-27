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
#define DEBUG(fmt, ...)     printf(fmt, ##__VA_ARGS__)
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
    u32 addr;
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
                    "LOCATE", /* locate the mem of instruction */
                    };

u8 cpu_mem[MEM_SIZE] = {0}; 
u32 tindex = 0, iindex = 0;
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
    for(i=0;i<sizeof(keyword);i++) {
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
    id_pool[iindex].buf = malloc(len+1);
    memcpy(id_pool[iindex].buf, s, len);
    id_pool[iindex].buf[len] = '\0';

    id_pool[iindex].addr = 0;
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
                put_token(AM_REG_INDIRECT << 16 | TOKEN_KEYWORD, j);
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

s32 build_inst(u32 op_type, u32 am_dst, u32 dst, u32 am_src1, u32 src1, u32 am_src2, u32 src2)
{
    return 0;
}

s32 gen_code()
{
    u32 addr = 0;
    s32 op_type = -1;
    struct __instruction__ inst;
    for(tindex=0;tindex<POOL_SIZE;) {
        switch (token_pool[tindex].type) {
            case (KW_MOV):
                break;
            case (KW_LDR):
                break;
            case (KW_STR):
                break;
            case (KW_PUSH):
                break;
            case (KW_POP):
                break;
            case (KW_CALL):
                break;
            case (KW_RET):
                break;
            case (KW_ADD):
                break;
            case (KW_SUB):
                break;
            case (KW_DIV):
                break;
            case (KW_MUL):
                break;
            case (KW_AND):
                break;
            case (KW_OR):
                break;
            case (KW_XOR):
                break;
            case (KW_JMP):
                break;
            case (KW_JMPN):
                break;
            case (KW_JMPZ):
                break;
            case (KW_JMPO):
                break;
            case (KW_JMPNN):
                break;
            case (KW_JMPNZ):
                break;
            case (KW_JMPNO):
                break;
            case (KW_R0):
                break;
            case (KW_R1):
                break;
            case (KW_R2):
                break;
            case (KW_R3):
                break;
            case (KW_SP):
                break;
            case (KW_PC):
                break;
            case (KW_LOCATE):
                break;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("%s [foo.s]\n", argv[0]);
        exit(-1);
    }

    parse_token(argv[1]);
    dump_token();
    gen_code();
    return 0;
}
