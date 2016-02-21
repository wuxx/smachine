#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/*
   the assembler of smachine
*/

#define POOL_SIZE 1024

typedef unsigned char u8;
typedef signed   char s8;
typedef unsigned int  u32;
typedef signed   int  s32;

enum TOKEN_TYPE_E {
    TOKEN_KEYWORD = 0,
    TOKEN_LOCATE  = 1,  /* [locate 0x1234] */
    TOKEN_COMMA   = 2,  
    TOKEN_STRING  = 3,  /* str: "hello, world" */
    TOKEN_COLON   = 4,  /* a: jmp a */
    TOKEN_ID      = 5,
    TOKEN_IMM     = 6,
    TOKEN_MAX,
};

struct __token__ {
    u32 type;
    u32 value;
};

char *keyword[] = { "mov",  "ldr",  "str",
                    "push", "pop",
                    "call", "ret",
                    "add",  "sub",  "div",  "mul",  "and",   "or",    "xor",
                    "jmp",  "jmpn", "jmpz", "jmpo", "jmpnn", "jmpnz", "jmpno",
                    };

struct __token__ token_pool[POOL_SIZE];

int parse_token(char *ifile)
{
    FILE *fp;
    char line[1024];
    if((fp = fopen(ifile, "r")) == NULL)
    { 
        perror("fopen"); 
        exit(-1);
    } 

    while (!feof(fp)) 
    { 
        fgets(line,sizeof(line),fp);
        printf("%s\n", line);
    } 

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
