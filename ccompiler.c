#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "cpu.h"

#define POOL_SIZE (10240)

enum TOKEN_TYPE_E {
    TOKEN_NULL = 0,         
    TOKEN_KW_IF,
    TOKEN_KW_ELSE,       
    TOKEN_KW_WHILE,      
    TOKEN_KW_FOR,        
    TOKEN_KW_DO,         
    TOKEN_KW_RETURN,     
    TOKEN_KW_MAIN,

    TOKEN_TP_VOID,       
    TOKEN_TP_INT,        
    TOKEN_TP_CHAR,       
    TOKEN_TP_ENUM,       

    TOKEN_OP_PLUS,       
    TOKEN_OP_SUB,        
    TOKEN_OP_MUL,        
    TOKEN_OP_DIV,        
    TOKEN_OP_EQ,         
    TOKEN_OP_EM,         
    TOKEN_OP_TILDE,      
    TOKEN_OP_XOR,        
    TOKEN_OP_PERCENT,    
    TOKEN_OP_ADDR,       
    TOKEN_OP_LAND,       
    TOKEN_OP_OR,         
    TOKEN_OP_LOR,        
    TOKEN_OP_PP,         
    TOKEN_OP_SS,         
    TOKEN_OP_EE,         
    TOKEN_OP_LT,         
    TOKEN_OP_GT,         
    TOKEN_OP_LE,         
    TOKEN_OP_GE,         

    TOKEN_SP_LCURBRACE,  
    TOKEN_SP_RCURBRACE,  
    TOKEN_SP_LSQRBRACKET,
    TOKEN_SP_RSQRBRACKET,
    TOKEN_SP_LRNDBRACKET,
    TOKEN_SP_RRNDBRACKET,
    TOKEN_SP_SEMICOLON,  
    TOKEN_SP_COMMA,      

    TOKEN_ID,            
    TOKEN_NUM,           
    TOKEN_MAX,
}; 

char * tk_desc[] = {
    "TOKEN_NULL",         
    "TOKEN_KW_IF",
    "TOKEN_KW_ELSE",       
    "TOKEN_KW_WHILE",      
    "TOKEN_KW_FOR",        
    "TOKEN_KW_DO",         
    "TOKEN_KW_RETURN",     
    "TOKEN_KW_MAIN",

    "TOKEN_TP_VOID",       
    "TOKEN_TP_INT",        
    "TOKEN_TP_CHAR",       
    "TOKEN_TP_ENUM",       

    "TOKEN_OP_PLUS",       
    "TOKEN_OP_SUB",        
    "TOKEN_OP_MUL",        
    "TOKEN_OP_DIV",        
    "TOKEN_OP_EQ",         
    "TOKEN_OP_EM",         
    "TOKEN_OP_TILDE",      
    "TOKEN_OP_XOR",        
    "TOKEN_OP_PERCENT",    
    "TOKEN_OP_ADDR",       
    "TOKEN_OP_LAND",       
    "TOKEN_OP_OR",         
    "TOKEN_OP_LOR",        
    "TOKEN_OP_PP",         
    "TOKEN_OP_SS",         
    "TOKEN_OP_EE",         
    "TOKEN_OP_LT",         
    "TOKEN_OP_GT",         
    "TOKEN_OP_LE",         
    "TOKEN_OP_GE",         

    "TOKEN_SP_LCURBRACE",  
    "TOKEN_SP_RCURBRACE",  
    "TOKEN_SP_LSQRBRACKET",
    "TOKEN_SP_RSQRBRACKET",
    "TOKEN_SP_LRNDBRACKET",
    "TOKEN_SP_RRNDBRACKET",
    "TOKEN_SP_SEMICOLON",  
    "TOKEN_SP_COMMA",      

    "TOKEN_ID",            
    "TOKEN_NUM",           
    "TOKEN_MAX",
}; 

struct __token__ {
    int   type;
    char *t;
    int   value;
};

int ifd, ofd;
int ifoffset = 0;

int id_index = 0;
char * id_pool[POOL_SIZE];

int tk_index = 0;
struct __token__ token_pool[POOL_SIZE] = {{0, NULL, 0}};

struct __token__ * ptoken = NULL;

struct __token__ c_token_pool[] = {
    {TOKEN_NULL,             "",         0},
    {TOKEN_KW_IF,            "if",       0},
    {TOKEN_KW_ELSE,          "else",     0},
    {TOKEN_KW_WHILE,         "while",    0},
    {TOKEN_KW_FOR,           "for",      0},
    {TOKEN_KW_DO,            "do",       0},
    {TOKEN_KW_RETURN,        "return",   0},
    {TOKEN_KW_MAIN,          "main",     0},

    {TOKEN_TP_VOID,          "void",     0},
    {TOKEN_TP_INT,           "int",      0},
    {TOKEN_TP_CHAR,          "char",     0},
    {TOKEN_TP_ENUM,          "enum",     0},

    {TOKEN_OP_PLUS,          "+",        0},
    {TOKEN_OP_SUB,           "-",        0},
    {TOKEN_OP_MUL,           "*",        0},
    {TOKEN_OP_DIV,           "/",        0},
    {TOKEN_OP_EQ,            "=",        0},
    {TOKEN_OP_EM,            "!",        0},
    {TOKEN_OP_TILDE,         "~",        0},
    {TOKEN_OP_XOR,           "^",        0},
    {TOKEN_OP_PERCENT,       "%",        0},
    {TOKEN_OP_ADDR,          "&",        0},
    {TOKEN_OP_LAND,          "&&",       0},
    {TOKEN_OP_OR,            "|",        0},
    {TOKEN_OP_LOR,           "||",       0},
    {TOKEN_OP_PP,            "++",       0},
    {TOKEN_OP_SS,            "--",       0},
    {TOKEN_OP_EE,            "==",       0},
    {TOKEN_OP_LT,            "<",        0},
    {TOKEN_OP_GT,            ">",        0},
    {TOKEN_OP_LE,            "<=",       0},
    {TOKEN_OP_GE,            ">=",       0},

    {TOKEN_SP_LCURBRACE,     "{",        0},
    {TOKEN_SP_RCURBRACE,     "}",        0},
    {TOKEN_SP_LSQRBRACKET,   "[",        0},
    {TOKEN_SP_RSQRBRACKET,   "]",        0},
    {TOKEN_SP_LRNDBRACKET,   "(",        0},
    {TOKEN_SP_RRNDBRACKET,   ")",        0},
    {TOKEN_SP_SEMICOLON,     ";",        0},
    {TOKEN_SP_COMMA,         ",",        0},

    {TOKEN_ID,               "",         0},
    {TOKEN_NUM,              "",         0},
}; 

int is_keyword(char *id)
{
    int i;
    for(i=0;i<(sizeof(c_token_pool)/sizeof(c_token_pool[0]));i++) {
        if (strcmp(c_token_pool[i].t, id) == 0) {
            return i;
        }
    }
    return 0;
}

int is_type(char c)
{

}

int is_operator(char c)
{

}

int is_separator(char c)
{

}

int is_digit(char c)
{
    if (c >= '0' && c <= '9') {
        return 1;
    } else {
        return 0;
    }

}

int is_id_head(char c)
{
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c == '_') ) {
        return 1;
    } else {
        return 0;
    }
}

int is_id_body(char c)
{
    if ((c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        (c == '_') ) {
        return 1;
    } else {
        return 0;
    }
}

int put_token(int type, int value)
{
    DEBUG("put_token[%d]: [%s][%d]\n", tk_index, tk_desc[type], value);
    token_pool[tk_index].type    = type;
    token_pool[tk_index++].value = value;
    assert(tk_index < POOL_SIZE);
    return 0;
}

int put_id(char *id)
{
    int len;
    char *dst, *src;

    len = strlen(id);

    DEBUG("put_id: [%s] [%d]\n", id, len);
    if ((id_pool[id_index] = (char *)malloc(len+1)) == NULL) {
        perror("malloc");
        exit(-1);
    }

    dst = id_pool[id_index];
    src = id;
    DEBUG("%x %x %x \n", dst, src, len);
    memcpy(dst, src, len);
    dst[len] = '\0';
    id_index++;
    return id_index - 1;
}

char get_char()
{
    char c;
    if (read(ifd, &c, 1) == 0) {
        return 0xff;
    } else {
        ifoffset++;
        return c;
    }
}

void unget_char()
{
    ifoffset--;
    lseek(ifd, ifoffset, SEEK_SET);
}

int parse_token()
{
    char c, cn;
    int i, sum;
    char id[20];

    while ((c = get_char()) != 0xff) {
        DEBUG("c: [%c] \n", c);
        if (is_id_head(c)) {        /* keyword id, type id, or user-defined id */
            i = 0;
            id[i++] = c;
            c = get_char();
            while (is_id_body(c)) {
                id[i++] = c;
                c = get_char();
            }
            unget_char();
            id[i] = '\0';
            if ((i=is_keyword(id))) {
                put_token(i, 0);
            } else {    /* id */
                i = put_id(id);
                put_token(TOKEN_ID, i);
            }

        } else if (is_digit(c)) {
            sum = 0;
            while (is_digit(c)) {
                sum = sum * 10 + (c - '0');
                c = get_char();
            }
            unget_char();
            put_token(TOKEN_NUM, sum);
        } else if (c == '+') {      /* "+" or "++" */
            if ((cn = get_char()) == '+') {
                put_token(TOKEN_OP_PP, 0);
            } else {
                unget_char();
                put_token(TOKEN_OP_PLUS, 0);
            }
        } else if (c == '-') {      /* "-" or "--" */
            if ((cn = get_char()) == '-') {
                put_token(TOKEN_OP_SS, 0);
            } else {
                unget_char();
                put_token(TOKEN_OP_SUB, 0);
            }
        } else if (c == '*') {
            put_token(TOKEN_OP_MUL, 0);
        } else if (c == '/') {
            put_token(TOKEN_OP_DIV, 0);
        } else if (c == '=') {      /* "=" or "==" */
            if ((cn = get_char()) == '=') {
                put_token(TOKEN_OP_EE, 0);
            } else {
                unget_char();
                put_token(TOKEN_OP_EQ, 0);
            }
        } else if (c == '~') {
            put_token(TOKEN_OP_TILDE, 0);
        } else if (c == '|') {      /* "|" or "||" */
            if ((cn = get_char()) == '|') {
                put_token(TOKEN_OP_LOR, 0);
            } else {
                unget_char();
                put_token(TOKEN_OP_OR, 0);
            }
        } else if (c == '^') {
            put_token(TOKEN_OP_XOR, 0);
        } else if (c == '%') {
            put_token(TOKEN_OP_PERCENT, 0);
        } else if (c == '&') {      /* "&" or "&&" */
            if ((cn = get_char()) == '&') {
                put_token(TOKEN_OP_LAND, 0);
            } else {
                unget_char();
                put_token(TOKEN_OP_ADDR, 0);
            }
        } else if (c == '<') {      /* "<" or "<=" */
            if ((cn = get_char()) == '=') {
                put_token(TOKEN_OP_LE, 0);
            } else {
                unget_char();
                put_token(TOKEN_OP_LT, 0);
            }
        } else if (c == '>') {      /* ">" or ">=" */
            if ((cn = get_char()) == '=') {
                put_token(TOKEN_OP_GE, 0);
            } else {
                unget_char();
                put_token(TOKEN_OP_GT, 0);
            }
        } else if (c == '{') {
            put_token(TOKEN_SP_LCURBRACE, 0);
        } else if (c == '}') {
            put_token(TOKEN_SP_RCURBRACE, 0);
        } else if (c == '[') {
            put_token(TOKEN_SP_LSQRBRACKET, 0);
        } else if (c == ']') {
            put_token(TOKEN_SP_RSQRBRACKET, 0);
        } else if (c == '(') {
            put_token(TOKEN_SP_LRNDBRACKET, 0);
        } else if (c == ')') {
            put_token(TOKEN_SP_RRNDBRACKET, 0);
        } else if (c == ';') {
            put_token(TOKEN_SP_SEMICOLON, 0);
        } else if (c == ',') {
            put_token(TOKEN_SP_COMMA, 0);
        } else if (c == ' ' || c == '\n' || c == '\t'){
            
        } else {
            DEBUG("c [%x] [%d]\n", c, c);
            error();
        }
    }
    return 0;
}

struct __token__ * get_token()
{
    if (token_pool[tk_index].type != TOKEN_NULL) {
        return &token_pool[tk_index];
    } else {
        return NULL;
    }
}

void dump_token()
{
    int i = 0;
    while (token_pool[i].type != TOKEN_NULL) {
        printf("[%d]:", i);
        switch (token_pool[i].type) {
            case (TOKEN_ID):
                printf("[%s]\n", id_pool[token_pool[i].value]);
                break;
            case (TOKEN_NUM):
                printf("[%d]\n", token_pool[i].value);
                break;

            default:
                printf("[%s]\n", c_token_pool[token_pool[i].type].t);
                break;
        }

        i++;
    }
}

/* format:
   1. if () {}
   2. if () {} [else if() {}]* [else {}]*
*/
int stat_if()
{
    return 0;
}

/* format:
    while() {}
*/
int stat_while()
{
    return 0;
}

/* format:
    for(st1;st2;st3;) {}
*/
int stat_for()
{
    return 0;
}

/* format:
   1. op = expr;
   2. expr
*/
int expr()
{
    return 0;
}

int main(int argc, char **argv)
{
    struct stat st;

    if (argc != 3) {
        printf("%s [foo.c] [bar.bin]\n", argv[0]);
        exit(-1);
    }

    if ((ifd = open(argv[1], O_RDONLY)) == -1) {
        perror("open");
        exit(-1);
    }

    if ((ofd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0664)) == -1) {
        perror("open");
        exit(-1);
    }   

    if (fstat(ifd, &st) == -1) {
        perror("fstat");
        exit(-1);
    }

    parse_token();
    tk_index = 0;
    ptoken = get_token();
    dump_token();
    return 0;
}
