#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "cpu.h"

/*
   the assembler of smachine
*/

#define POOL_SIZE 1024

enum TOKEN_TYPE_E {
    TOKEN_KEYWORD = 0,
    TOKEN_COMMA   = 1,  
    TOKEN_STRING  = 2,  /* str: "hello, world" */
    TOKEN_COLON   = 3,  /* a: jmp a */
    TOKEN_ID      = 4,
    TOKEN_IMM     = 5,
    TOKEN_MAX,
};

struct __token__ {
    u32 type;
    u32 value;
};

char *keyword[] = { "NULL",
                    "mov",  "ldr",  "str",
                    "push", "pop",
                    "call", "ret",
                    "add",  "sub",  "div",  "mul",  "and",   "or",    "xor",
                    "jmp",  "jmpn", "jmpz", "jmpo", "jmpnn", "jmpnz", "jmpno",
                    "LOCATE",
                    "r0",   "r1",   "r2",   "r3",
                    "sp", /* alias of r2 */
                    "pc", /* alias of r3 */
                    "LOCATE", /* locate the mem of instruction */
                    };

s32 is_letter(char c)
{
    if (c >= 'a' && c <= 'z') {
        return 1;
    } else {
        return 0;
    }
}

s32 is_id(char c)
{
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) {
        return 1;
    } else {
        return 0;
    }
}

s32 is_digit(char c)
{
    if (c >= '0' && c <= '9') {
        return 1;
    } else {
        return 0;
    }
}

s32 is_keyword(char *s, u32 len)
{
    u32 i;   
    for(i=0;i<sizeof(keyword);i++) {
        if (strcmp(keyword[i], s) == 0) {
            return i;
        }
    }
    return 0;
}

u32 tindex = 0;
struct __token__ token_pool[POOL_SIZE];
s32 put_token(u32 t, u32 v)
{
    token_pool[tindex].type  = t;
    token_pool[tindex].value = v;
    tindex++;
    assert(tindex < POOL_SIZE);
}

s32 parse_line(char *line)
{
    u32 i = 0, j;
    u32 radix, num;
    u32 start, end, len;
    char c;
    while((c = line[i]) != 0) {
        if (c == ' ') {
            i++;
        } else if (c == ';') {      /* the comment */
            return 0;
        } else if (is_letter(c)) {  /* keyword or id */
            start = i;
            i++;
            while (is_id(c)) {
                c = line[i++];
            }
            end = i;
            if ((j = is_keyword(&line[start], end-start))) {
                put_token(TOKEN_KEYWORD, j);
            } else { /* id */
                /* id_pool */
            }

        } else if (c == '#') {      /* immediate num */
            if (line[i+1] == '0' && line[i+2] == 'x') { /* hex FIXME: i+1 i+2 may overstep the boundary */
                radix = 16;
            } else { /* dec */
                radix = 10;
            }

            i++;
            start = i;
            c = line[i];
            while(is_digit(c)) {
                c = line[i++];
            }
            end = i;
            assert(line[end] == ' ');
            line[end] = '\0'; /* for atoi */
            i++;    /* skip the ' ' */

            num = atoi(&line[start]);
            put_token(TOKEN_IMM, num);

        } else if (c == '[') { /* register indirect */
            start = ++i;
            end   = i+2;
            /* [r0|r1|r2|r3|sp|pc] */
            if (j = is_keyword(&line[start], end-start)) {
                put_token(TOKEN_KEYWORD, j);
            } else {
                error();
            }
            i = i + 2;
            if (line[i] != ']') {
                error();
            }
        } else if (c == ',') {
            put_token(TOKEN_COMMA, 0);
        } else if (c == ':') {
            put_token(TOKEN_COLON, 0);
        } else if (c == '\"') {   /* string */
            /* str_pool */
            /* expect('\"'); */
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
        fgets(line,sizeof(line),fp);
        printf("%s\n", line);
        parse_line(line);
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
    return 0;
}
