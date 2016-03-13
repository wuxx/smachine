#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "cpu.h"

#define POOL_SIZE (10240)

enum TOKEN_TYPE_E {
    TOKEN_KW_IF = 0,         
    TOKEN_KW_ELSE,       
    TOKEN_KW_WHILE,      
    TOKEN_KW_FOR,        
    TOKEN_KW_DO,         
    TOKEN_KW_RETURN,     

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

struct __token__ {
    int   type;
    char *t;
    int   value;
};

int ifd, ofd;
int ioffset = 0;

struct __token__ id_token_pool[POOL_SIZE];

struct __token__ c_token_pool[] = {
    {TOKEN_KW_IF,            "if",       0},
    {TOKEN_KW_ELSE,          "else",     0},
    {TOKEN_KW_WHILE,         "while",    0},
    {TOKEN_KW_FOR,           "for",      0},
    {TOKEN_KW_DO,            "do",       0},
    {TOKEN_KW_RETURN,        "return",   0},

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
    {TOKEN_SP_RRNDBRACKET,   "(",        0},
    {TOKEN_SP_SEMICOLON,     ";",        0},
    {TOKEN_SP_COMMA,         ",",        0},

    {TOKEN_ID,               "",         0},
    {TOKEN_NUM,              "",         0},
}; 

int is_keyword(char c)
{

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
        (c = '_') ) {
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
        (c = '_') ) {
        return 1;
    } else {
        return 0;
    }
}

int put_token(int type, int value)
{
    return 0;
}

int put_id(char *id, int len)
{
    return 0;
}

char get_char()
{
    char c;
    if (read(ifd, &c, 1) == 0) {
        return -1;
    } else {
        ioffset++;
        return c;
    }
}

void unget_char()
{
    ioffset--;
    lseek(ifd, ioffset, SEEK_SET);
}

int parse_token()
{
    char c, cn;
    c = get_char();
    while (c != -1) {
        if (is_id_head(c)) {        /* keyword id, type id, or user-defined id */

        } else if (is_digit(c)) {
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
        } else {
            error();
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    struct stat st;

    if (argc != 2) {
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
    return 0;
}
