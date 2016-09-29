// c4.c - C in four functions

// char, int, and pointer types
// if, while, return, and expression statements
// just enough features to allow self-compilation and a bit more

// Written by Robert Swierczek

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>

#define ASM_EMIT(fmt, ...)     do {                                             \
    len = sprintf(&ee[eindex], fmt, ##__VA_ARGS__);   \
    eindex += len;                              \
    if(strncmp(fmt, "ldrb", 4) == 0) {          \
        lc = 1;                                 \
    } else {                                    \
        lc = 0;                                 \
    }                                           \
    if(strncmp(fmt, "ldr", 3) == 0) {           \
        li = 1;                                 \
    } else {                                    \
        li = 0;                                 \
    }                                           \
} while(0)

char *ee;
int eindex = 0;
int lc, li;
int len;

char *p, *lp, // current position in source code
     *data;   // data/bss pointer

int *e, *le,  // current position in emitted code
    *id,      // currently parsed identifier
    *sym,     // symbol table (simple list of identifiers)
    tk,       // current token
    ival,     // current token value
    ty,       // current expression type
    loc,      // local variable offset
    line;     // current line number

int label_index = 0;

// tokens and classes (operators last and in precedence order)
enum {
    Num = 128, Fun, Sys, Glo, Loc, Id,
    Char, Else, Enum, If, Int, Return, Sizeof, While,
    Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

// opcodes
enum { LEA ,IMM ,JMP ,JSR ,BZ  ,BNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PSH ,
    OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
    OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT };

// types
enum { CHAR, INT, PTR };

// identifier offsets (since we can't create an ident struct)
enum { Tk, Hash, Name, Class, Type, Val, HClass, HType, HVal, Idsz };

void next()
{
    char *pp;

    while (tk = *p) {
        ++p;
        if (tk == '\n') {
            ++line;
        }
        else if (tk == '#') {
            while (*p != 0 && *p != '\n') ++p;
        }
        else if ((tk >= 'a' && tk <= 'z') || (tk >= 'A' && tk <= 'Z') || tk == '_') {
            pp = p - 1;
            while ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == '_')
                tk = tk * 147 + *p++;
            tk = (tk << 6) + (p - pp);
            id = sym;
            while (id[Tk]) {
                if (tk == id[Hash] && !memcmp((char *)id[Name], pp, p - pp)) { tk = id[Tk]; return; }
                id = id + Idsz;
            }
            id[Name] = (int)pp;
            id[Hash] = tk;
            tk = id[Tk] = Id;
            return;
        }
        else if (tk >= '0' && tk <= '9') {
            if (ival = tk - '0') { while (*p >= '0' && *p <= '9') ival = ival * 10 + *p++ - '0'; }
            else if (*p == 'x' || *p == 'X') {
                while ((tk = *++p) && ((tk >= '0' && tk <= '9') || (tk >= 'a' && tk <= 'f') || (tk >= 'A' && tk <= 'F')))
                    ival = ival * 16 + (tk & 15) + (tk >= 'A' ? 9 : 0);
            }
            else { while (*p >= '0' && *p <= '7') ival = ival * 8 + *p++ - '0'; }
            tk = Num;
            return;
        }
        else if (tk == '/') {
            if (*p == '/') {
                ++p;
                while (*p != 0 && *p != '\n') ++p;
            }
            else {
                tk = Div;
                return;
            }
        }
        else if (tk == '\'' || tk == '"') {
            pp = data;
            while (*p != 0 && *p != tk) {
                if ((ival = *p++) == '\\') {
                    if ((ival = *p++) == 'n') ival = '\n';
                }
                if (tk == '"') *data++ = ival;
            }
            ++p;
            if (tk == '"') ival = (int)pp; else tk = Num;
            return;
        }
        else if (tk == '=') { if (*p == '=') { ++p; tk = Eq; } else tk = Assign; return; }
        else if (tk == '+') { if (*p == '+') { ++p; tk = Inc; } else tk = Add; return; }
        else if (tk == '-') { if (*p == '-') { ++p; tk = Dec; } else tk = Sub; return; }
        else if (tk == '!') { if (*p == '=') { ++p; tk = Ne; } return; }
        else if (tk == '<') { if (*p == '=') { ++p; tk = Le; } else if (*p == '<') { ++p; tk = Shl; } else tk = Lt; return; }
        else if (tk == '>') { if (*p == '=') { ++p; tk = Ge; } else if (*p == '>') { ++p; tk = Shr; } else tk = Gt; return; }
        else if (tk == '|') { if (*p == '|') { ++p; tk = Lor; } else tk = Or; return; }
        else if (tk == '&') { if (*p == '&') { ++p; tk = Lan; } else tk = And; return; }
        else if (tk == '^') { tk = Xor; return; }
        else if (tk == '%') { tk = Mod; return; }
        else if (tk == '*') { tk = Mul; return; }
        else if (tk == '[') { tk = Brak; return; }
        else if (tk == '?') { tk = Cond; return; }
        else if (tk == '~' || tk == ';' || tk == '{' || tk == '}' || tk == '(' || tk == ')' || tk == ']' || tk == ',' || tk == ':') return;
    }
}

void expr(int lev)
{
    int t, *d;
    int l;

    if (!tk) { printf("%d: unexpected eof in expression\n", line); exit(-1); }
    else if (tk == Num) { ASM_EMIT("mov r0, #0x%x\n", ival); next(); ty = INT; }
    else if (tk == '"') {
        ASM_EMIT("mov r0, #0x%x\n", ival); next();
        while (tk == '"') next();
        data = (char *)((int)data + sizeof(int) & -sizeof(int)); ty = PTR;
    }
    else if (tk == Sizeof) {
        next(); if (tk == '(') next(); else { printf("%d: open paren expected in sizeof\n", line); exit(-1); }
        ty = INT; if (tk == Int) next(); else if (tk == Char) { next(); ty = CHAR; }
        while (tk == Mul) { next(); ty = ty + PTR; }
        if (tk == ')') next(); else { printf("%d: close paren expected in sizeof\n", line); exit(-1); }
        ASM_EMIT("mov r0, #0x%x\n", (ty == CHAR) ? sizeof(char) : sizeof(int));
        ty = INT;
    }
    else if (tk == Id) {
        d = id; next();
        if (tk == '(') {
            next();
            t = 0;
            while (tk != ')') { expr(Assign); ASM_EMIT("push r0\n"); ++t; if (tk == ',') next(); }
            next();
            if (d[Class] == Sys) *++e = d[Val];
            else if (d[Class] == Fun) { ASM_EMIT("call #0x%x\n", d[Val]); }
            else { printf("%d: bad function call\n", line); exit(-1); }
            if (t) { ASM_EMIT("mov sp, #0x%x\n", t); }
            ty = d[Type];
        }
        else if (d[Class] == Num) { ASM_EMIT("mov r0, #0x%x\n", d[Val]); ty = INT; }
        else {
            if (d[Class] == Loc) { ASM_EMIT("ldr r0, [fp, #%d]\n", loc - d[Val]); }
            else if (d[Class] == Glo) { ASM_EMIT("mov r0, #0x%x\n", d[Val]); }
            else { printf("%d: undefined variable\n", line); exit(-1); }
            if ((ty = d[Type]) == CHAR) {
                ASM_EMIT("ldrb r0, [r0]\n");  /* FIXME: ldrb */
            } else {
                ASM_EMIT("ldr r0, [r0]\n");
            }
        }
    }
    else if (tk == '(') {
        next();
        if (tk == Int || tk == Char) {
            t = (tk == Int) ? INT : CHAR; next();
            while (tk == Mul) { next(); t = t + PTR; }
            if (tk == ')') next(); else { printf("%d: bad cast\n", line); exit(-1); }
            expr(Inc);
            ty = t;
        }
        else {
            expr(Assign);
            if (tk == ')') next(); else { printf("%d: close paren expected\n", line); exit(-1); }
        }
    }
    else if (tk == Mul) {
        next(); expr(Inc);
        if (ty > INT) ty = ty - PTR; else { printf("%d: bad dereference\n", line); exit(-1); }
        if ((ty = d[Type]) == CHAR) {
            ASM_EMIT("ldrb r0, [r0]\n");  /* FIXME: ldrb */
        } else {
            ASM_EMIT("ldr r0, [r0]\n");
        }
    }
    else if (tk == And) {
        next(); expr(Inc);
        if (lc == 1 || li == 1) eindex -= len; else { printf("%d: bad address-of\n", line); exit(-1); }
        ty = ty + PTR;
    }
    else if (tk == '!') { 
        next(); expr(Inc); 
        ASM_EMIT("add r0, r0, #0\n");
        ASM_EMIT("jmpnz L%d\n", label_index);
        ASM_EMIT("mov r0, #0x0\n"); 
        ASM_EMIT("jmp L%d\n", label_index + 1);
        ASM_EMIT("L%d:\n", label_index);
        ASM_EMIT("mov r0, #0x1\n"); 
        ASM_EMIT("L%d:\n", label_index + 1);
        label_index += 2;
        ty = INT; 
    }
    else if (tk == '~') { 
        next(); expr(Inc); 
        ASM_EMIT("push r0\n"); 
        ASM_EMIT("pop r1"); 
        ASM_EMIT("mov r0, #0xFFFFFFFF\n"); 
        ASM_EMIT("xor r0, r0, r1\n"); 
        ty = INT; 
    }
    else if (tk == Add) { next(); expr(Inc); ty = INT; }
    else if (tk == Sub) {
        next(); ASM_EMIT("mov r0, #");
        if (tk == Num) { 
            ASM_EMIT("%d\n", -ival); next(); 
        } else { 
            ASM_EMIT("-1\n"); 
            ASM_EMIT("push r0\n"); 
            expr(Inc);
            ASM_EMIT("pop r1\n"); 
            ASM_EMIT("mul r0, r0, r1\n"); 

        }
        ty = INT;
    }
    else if (tk == Inc || tk == Dec) {
        t = tk; next(); expr(Inc);
        if (lc == 1) { 
            ASM_EMIT("push r0\n");
            ASM_EMIT("ldrb r0, [r0]\n");
        }
        else if (li == 1) { 
            ASM_EMIT("push r0\n");
            ASM_EMIT("ldr  r0, [r0]\n");
        }
        else { printf("%d: bad lvalue in pre-increment\n", line); exit(-1); }
        ASM_EMIT("push r0\n");
        ASM_EMIT("mov r0, #0x%x\n", (ty > PTR) ? sizeof(int) : sizeof(char));
        ASM_EMIT("pop r1\n");
        if (t == Inc) {
            ASM_EMIT("add r0, r0, r1\n");
        } else {
            ASM_EMIT("add r0, r1, r0\n");
        }
        if (t == Inc) {
            ASM_EMIT("add r0, r0, r1\n");
        } else {
            ASM_EMIT("sub r0, r1, r0\n");
        }
        ASM_EMIT("pop r1\n");
        if (ty == CHAR) {
            ASM_EMIT("strb r0, [r1]\n");
        } else {
            ASM_EMIT("str r0, [r1]\n");
        }
    }
    else { printf("%d: bad expression\n", line); exit(-1); }

    while (tk >= lev) { // "precedence climbing" or "Top Down Operator Precedence" method
        t = ty;
        if (tk == Assign) {
            next();
            if (lc == 1 || li == 1) ASM_EMIT("push r0\n"); else { printf("%d: bad lvalue in assignment\n", line); exit(-1); }
            expr(Assign); 
            ASM_EMIT("pop r1\n");
            if (((ty = t) == CHAR)) {
                ASM_EMIT("strb r0, [r1]\n");
            } else {
                ASM_EMIT("str r0, [r1]\n");
            }
        }
        else if (tk == Cond) {
            next();
            l = label_index;
            label_index += 2;
            ASM_EMIT("add r0, r0, #0x0\n");
            ASM_EMIT("jmpz L%d\n", l);
            expr(Assign);
            if (tk == ':') next(); else { printf("%d: conditional missing colon\n", line); exit(-1); }
            ASM_EMIT("jmp L%d\n", l+1);
            ASM_EMIT("L%d:\n", l);
            expr(Cond);
            ASM_EMIT("L%d:\n", l+1);
        }
        else if (tk == Lor) { 
            next(); 
            l = label_index;
            label_index += 1;

            ASM_EMIT("add r0, r0, #0x0\n");
            ASM_EMIT("jmpnz L%d\n", l);

            expr(Lan); 
            ASM_EMIT("L%d:\n", l);
            ty = INT; 
        }
        else if (tk == Lan) { 
            next(); 
            l = label_index;
            label_index += 1;

            ASM_EMIT("add r0, r0, #0x0\n");
            ASM_EMIT("jmpz L%d\n", l);
            expr(Or);  
            ASM_EMIT("L%d:\n", l);
            ty = INT; 
        }
        else if (tk == Or)  { 
            next(); 
            ASM_EMIT("push r0\n");
            expr(Xor); 
            ASM_EMIT("pop r1\n");
            ASM_EMIT("or r0, r0, r1\n");
            ty = INT; 
        }
        else if (tk == Xor) { 
            next(); 
            ASM_EMIT("push r0\n");
            expr(And); 
            ASM_EMIT("pop r1\n");
            ASM_EMIT("xor r0, r0, r1\n");
            ty = INT; 
        }
        else if (tk == And) { 
            next(); 
            ASM_EMIT("push r0\n");
            expr(Eq);  
            ASM_EMIT("pop r1\n");
            ASM_EMIT("and r0, r0, r1\n");
            ty = INT; 
        }
        else if (tk == Eq)  { 
            next(); 
            ASM_EMIT("push r0\n");
            expr(Lt);  
            ASM_EMIT("pop r1");
            ASM_EMIT("sub r0, r0, r1\n");
            ASM_EMIT("jmpnz L%d\n", label_index);
            ASM_EMIT("mov r0, #0x1\n"); 
            ASM_EMIT("jmp L%d\n", label_index + 1);
            ASM_EMIT("L%d:\n", label_index);
            ASM_EMIT("mov r0, #0x0\n"); 
            ASM_EMIT("L%d:\n", label_index + 1);
            label_index += 2;
            ty = INT; 
        }
        else if (tk == Ne)  { 
            next(); 
            ASM_EMIT("push r0\n");
            expr(Lt);  
            ASM_EMIT("pop r1");
            ASM_EMIT("sub r0, r0, r1\n");
            ASM_EMIT("jmpnz L%d\n", label_index);
            ASM_EMIT("mov r0, #0x0\n"); 
            ASM_EMIT("jmp L%d\n", label_index + 1);
            ASM_EMIT("L%d:\n", label_index);
            ASM_EMIT("mov r0, #0x1\n"); 
            ASM_EMIT("L%d:\n", label_index + 1);
            label_index += 2;
            ty = INT; 
        }
        else if (tk == Lt)  { 
            next(); 
            ASM_EMIT("push r0\n");
            expr(Shl); 
            ASM_EMIT("pop r1\n");
            ASM_EMIT("sub r0, r1, r0\n");
            ASM_EMIT("jmpn L%d\n", label_index);
            ASM_EMIT("mov r0, #0\n");
            ASM_EMIT("jmp L%d\n", label_index + 1);
            ASM_EMIT("L%d\n", label_index);
            ASM_EMIT("mov r0, #1\n");
            ASM_EMIT("L%d\n", label_index + 1);
            label_index += 2;
            ty = INT; 
        }
        else if (tk == Gt)  { 
            next(); 
            ASM_EMIT("push r0\n");
            expr(Shl); 
            ASM_EMIT("pop r1\n");
            ASM_EMIT("sub r0, r0, r1\n");
            ASM_EMIT("jmpn L%d\n", label_index);
            ASM_EMIT("mov r0, #0\n");
            ASM_EMIT("jmp L%d\n", label_index + 1);
            ASM_EMIT("L%d\n", label_index);
            ASM_EMIT("mov r0, #1\n");
            ASM_EMIT("L%d\n", label_index + 1);
            label_index += 2;
            ty = INT; 
        }
        else if (tk == Le)  { 
            next(); 
            ASM_EMIT("push r0\n");
            expr(Shl); 
            ASM_EMIT("pop r1\n");
            ASM_EMIT("sub r0, r1, r0\n");
            ASM_EMIT("jmpn L%d\n", label_index);
            ASM_EMIT("jmpz L%d\n", label_index);
            ASM_EMIT("mov r0, #0\n");
            ASM_EMIT("jmp L%d\n", label_index + 1);
            ASM_EMIT("L%d\n", label_index);
            ASM_EMIT("mov r0, #1\n");
            ASM_EMIT("L%d\n", label_index + 1);
            label_index += 2;
            ty = INT; 
        }
        else if (tk == Ge)  { 
            next(); 
            ASM_EMIT("push r0\n");
            expr(Shl); 
            ASM_EMIT("pop r1\n");
            ASM_EMIT("sub r0, r1, r0\n");
            ASM_EMIT("jmpn L%d\n", label_index);
            ASM_EMIT("mov r0, #1\n");
            ASM_EMIT("jmp L%d\n", label_index + 1);
            ASM_EMIT("L%d\n", label_index);
            ASM_EMIT("mov r0, #0\n");
            ASM_EMIT("L%d\n", label_index + 1);
            label_index += 2;
            ty = INT; 
        }
        else if (tk == Shl) { 
            next(); 
            ASM_EMIT("push r0\n");
            expr(Add); 
            ASM_EMIT("pop r1\n");
            ASM_EMIT("lol r0, r1, r0\n");
            ty = INT; 
        }
        else if (tk == Shr) { 
            next(); 
            ASM_EMIT("push r0\n");
            expr(Add); 
            ASM_EMIT("pop r1\n");
            ASM_EMIT("lor r0, r1, r0\n");
            ty = INT; 
        }
        else if (tk == Add) {
            next(); 
            ASM_EMIT("push r0\n");
            expr(Mul);
            if ((ty = t) > PTR) { 
                ASM_EMIT("push r0\n");
                ASM_EMIT("mov r0, #0x%x\n", sizeof(int));
                ASM_EMIT("pop r1\n");
                ASM_EMIT("mul r0, r0, r1\n");
            }
            ASM_EMIT("pop r1\n");
            ASM_EMIT("add r0, r0, r1\n");
        }
        else if (tk == Sub) {
            next(); 
            ASM_EMIT("push r0\n");
            expr(Mul);
            if (t > PTR && t == ty) { 
                ASM_EMIT("pop r1\n");
                ASM_EMIT("sub r0, r1, r0\n");
                ASM_EMIT("push r0\n");
                ASM_EMIT("mov r0, #0x%x\n", sizeof(int));
                ASM_EMIT("pop r1\n");
                ASM_EMIT("div r0, r1, r0\n");
                ty = INT; 
            }
            else if ((ty = t) > PTR) { 
                ASM_EMIT("push r0\n");
                ASM_EMIT("mov r0, #0x%x\n", sizeof(int));
                ASM_EMIT("pop r1\n");
                ASM_EMIT("mul r0, r0, r1\n");
                ASM_EMIT("pop r1\n");
                ASM_EMIT("sub r0, r1, r0\n");
            }
            else  {
                ASM_EMIT("pop r1\n");
                ASM_EMIT("sub r0, r1, r0\n");
            }
        }
        else if (tk == Mul) { 
            next(); 
            ASM_EMIT("push r0\n");
            expr(Inc); 
            ASM_EMIT("pop r1\n");
            ASM_EMIT("mul r0, r0, r1\n");
            ty = INT; 
        }
        else if (tk == Div) { 
            next(); 
            ASM_EMIT("push r0\n");
            expr(Inc); 
            ASM_EMIT("pop r1\n");
            ASM_EMIT("div r0, r1, r0\n");
            ty = INT; 
        }
        else if (tk == Mod) { 
            next(); 
            ASM_EMIT("push r0\n");
            expr(Inc); 
            ASM_EMIT("pop r1\n");
            ASM_EMIT("div r0, r1, r0\n");
            ASM_EMIT("mov r0, r1\n");
            ty = INT; 
        }
        else if (tk == Inc || tk == Dec) {
            if (lc == 1) { 
                ASM_EMIT("push r0\n");
                ASM_EMIT("ldrb r0, [r0]\n");
            }
            else if (li == 1) { 
                ASM_EMIT("push r0\n");
                ASM_EMIT("ldr r0, [r0]\n");
            }
            else { printf("%d: bad lvalue in post-increment\n", line); exit(-1); }
            ASM_EMIT("push r0\n");
            ASM_EMIT("mov r0, #0x%x\n", (ty > PTR) ? sizeof(int) : sizeof(char));
            if (tk == Inc) {
                ASM_EMIT("pop r1\n");
                ASM_EMIT("add r0, r0, r1\n");
            } else {
                ASM_EMIT("pop r1\n");
                ASM_EMIT("sub r0, r1, r0\n");
            }

            if (ty == CHAR) {
                ASM_EMIT("ldrb r0, [r0]\n");
            } else {
                ASM_EMIT("ldr r0, [r0]\n");
            }

            ASM_EMIT("push r0\n");
            ASM_EMIT("mov r0, #0x%x\n", (ty > PTR) ? sizeof(int) : sizeof(char));
            if (tk == Inc) {
                ASM_EMIT("pop r1\n");
                ASM_EMIT("sub r0, r1, r0\n");

            } else {
                ASM_EMIT("pop r1\n");
                ASM_EMIT("add r0, r0, r1\n");

            }
            next();
        }
        else if (tk == Brak) {
            next(); 
            ASM_EMIT("push r0\n");
            expr(Assign);
            if (tk == ']') next(); else { printf("%d: close bracket expected\n", line); exit(-1); }
            if (t > PTR) { 
                ASM_EMIT("push r0\n");
                ASM_EMIT("mov r0, #0x%x\n", sizeof(int));
                ASM_EMIT("pop r1\n");
                ASM_EMIT("mul r0, r0, r1\n");
            }
            else if (t < PTR) { printf("%d: pointer type expected\n", line); exit(-1); }
            ASM_EMIT("pop r1\n");
            ASM_EMIT("add r0, r0, r1\n");
            if ((ty = t - PTR) == CHAR) {
                ASM_EMIT("ldrb r0, [r0]\n");
            } else {
                ASM_EMIT("ldr r0, [r0]\n");
            }

        }
        else { printf("%d: compiler error tk=%d\n", line, tk); exit(-1); }
    }
}

void stmt()
{
    int *a, *b;
    int l;

    if (tk == If) {
        next();
        if (tk == '(') next(); else { printf("%d: open paren expected\n", line); exit(-1); }
        expr(Assign);
        if (tk == ')') next(); else { printf("%d: close paren expected\n", line); exit(-1); }
        l = label_index;
        label_index += 2;
        ASM_EMIT("add r0, r0, #0x0\n");
        ASM_EMIT("jmpz L%d\n", l);
        stmt();
        if (tk == Else) {
            ASM_EMIT("jmp L%d\n", l+1);
            ASM_EMIT("L%d:\n", l);
            next();
            stmt();
        }
        ASM_EMIT("L%d:\n", l+1);
    }
    else if (tk == While) {
        l = label_index;
        label_index += 2;
        next();
        ASM_EMIT("L%d:\n", l);
        if (tk == '(') next(); else { printf("%d: open paren expected\n", line); exit(-1); }
        expr(Assign);
        if (tk == ')') next(); else { printf("%d: close paren expected\n", line); exit(-1); }
        ASM_EMIT("add r0, r0, #0x0\n");
        ASM_EMIT("jmpz L%d\n", l+1);
        stmt();
        ASM_EMIT("jmp L%d\n", l);
        ASM_EMIT("L%d:\n", l+1);
    }
    else if (tk == Return) {
        next();
        if (tk != ';') expr(Assign);
        ASM_EMIT("ldr fp, [fp]\n");
        ASM_EMIT("ret\n");
        if (tk == ';') next(); else { printf("%d: semicolon expected\n", line); exit(-1); }
    }
    else if (tk == '{') {
        next();
        while (tk != '}') stmt();
        next();
    }
    else if (tk == ';') {
        next();
    }
    else {
        expr(Assign);
        if (tk == ';') next(); else { printf("%d: semicolon expected\n", line); exit(-1); }
    }
}

int main(int argc, char **argv)
{
    int fd, bt, ty, poolsz, *idmain;
    int *pc, *sp, *bp, a, cycle; // vm registers
    int i, *t; // temps

    --argc; ++argv;
    if (argc < 1) { printf("usage: c4 [-s] [-d] file ...\n"); return -1; }

    if ((fd = open(*argv, 0)) < 0) { printf("could not open(%s)\n", *argv); return -1; }

    poolsz = 256*1024; // arbitrary size
    if (!(sym = malloc(poolsz))) { printf("could not malloc(%d) symbol area\n", poolsz); return -1; }
    if (!(le = e = malloc(poolsz))) { printf("could not malloc(%d) text area\n", poolsz); return -1; }
    if (!(ee = malloc(poolsz))) { printf("could not malloc(%d) text area\n", poolsz); return -1; }
    if (!(data = malloc(poolsz))) { printf("could not malloc(%d) data area\n", poolsz); return -1; }
    if (!(sp = malloc(poolsz))) { printf("could not malloc(%d) stack area\n", poolsz); return -1; }

    memset(sym,  0, poolsz);
    memset(e,    0, poolsz);
    memset(data, 0, poolsz);

    p = "char else enum if int return sizeof while "
        "open read close printf malloc memset memcmp exit void main";
    i = Char; while (i <= While) { next(); id[Tk] = i++; } // add keywords to symbol table
    i = OPEN; while (i <= EXIT) { next(); id[Class] = Sys; id[Type] = INT; id[Val] = i++; } // add library to symbol table
    next(); id[Tk] = Char; // handle void type
    next(); idmain = id; // keep track of main

    if (!(lp = p = malloc(poolsz))) { printf("could not malloc(%d) source area\n", poolsz); return -1; }
    if ((i = read(fd, p, poolsz-1)) <= 0) { printf("read() returned %d\n", i); return -1; }
    p[i] = 0;
    close(fd);

    ASM_EMIT("LOCATE #0x0\n");
    ASM_EMIT("mov r0, #0x4000\n");
    ASM_EMIT("_start:\n");
    ASM_EMIT("call main\n");
    ASM_EMIT("jmp _exit\n");
    ASM_EMIT("main:\n");

    // parse declarations
    line = 1;
    next();
    while (tk) {
        bt = INT; // basetype
        if (tk == Int) next();
        else if (tk == Char) { next(); bt = CHAR; }
        else if (tk == Enum) {
            next();
            if (tk != '{') next();
            if (tk == '{') {
                next();
                i = 0;
                while (tk != '}') {
                    if (tk != Id) { printf("%d: bad enum identifier %d\n", line, tk); return -1; }
                    next();
                    if (tk == Assign) {
                        next();
                        if (tk != Num) { printf("%d: bad enum initializer\n", line); return -1; }
                        i = ival;
                        next();
                    }
                    id[Class] = Num; id[Type] = INT; id[Val] = i++;
                    if (tk == ',') next();
                }
                next();
            }
        }
        while (tk != ';' && tk != '}') {
            ty = bt;
            while (tk == Mul) { next(); ty = ty + PTR; }
            if (tk != Id) { printf("%d: bad global declaration\n", line); return -1; }
            if (id[Class]) { printf("%d: duplicate global definition\n", line); return -1; }
            next();
            id[Type] = ty;
            if (tk == '(') { // function
                id[Class] = Fun;
                id[Val] = (int)(e + 1);
                next(); i = 0;
                while (tk != ')') {
                    ty = INT;
                    if (tk == Int) next();
                    else if (tk == Char) { next(); ty = CHAR; }
                    while (tk == Mul) { next(); ty = ty + PTR; }
                    if (tk != Id) { printf("%d: bad parameter declaration\n", line); return -1; }
                    if (id[Class] == Loc) { printf("%d: duplicate parameter definition\n", line); return -1; }
                    id[HClass] = id[Class]; id[Class] = Loc;
                    id[HType]  = id[Type];  id[Type] = ty;
                    id[HVal]   = id[Val];   id[Val] = i++;
                    next();
                    if (tk == ',') next();
                }
                next();
                if (tk != '{') { printf("%d: bad function definition\n", line); return -1; }
                loc = ++i;
                next();
                while (tk == Int || tk == Char) {
                    bt = (tk == Int) ? INT : CHAR;
                    next();
                    while (tk != ';') {
                        ty = bt;
                        while (tk == Mul) { next(); ty = ty + PTR; }
                        if (tk != Id) { printf("%d: bad local declaration\n", line); return -1; }
                        if (id[Class] == Loc) { printf("%d: duplicate local definition\n", line); return -1; }
                        id[HClass] = id[Class]; id[Class] = Loc;
                        id[HType]  = id[Type];  id[Type] = ty;
                        id[HVal]   = id[Val];   id[Val] = ++i;
                        next();
                        if (tk == ',') next();
                    }
                    next();
                }

                ASM_EMIT("push fp\n");
                ASM_EMIT("mov fp, sp\n");
                ASM_EMIT("sub sp, sp, #0x%x\n", i - loc);
                while (tk != '}') stmt();
                ASM_EMIT("mov sp, fp\n");
                ASM_EMIT("pop fp\n");
                ASM_EMIT("ret\n");
                id = sym; // unwind symbol table locals
                while (id[Tk]) {
                    if (id[Class] == Loc) {
                        id[Class] = id[HClass];
                        id[Type] = id[HType];
                        id[Val] = id[HVal];
                    }
                    id = id + Idsz;
                }
            }
            else {
                id[Class] = Glo;
                id[Val] = (int)data;
                data = data + sizeof(int);
            }
            if (tk == ',') next();
        }
        next();
    }

    if (!(pc = (int *)idmain[Val])) { printf("main() not defined\n"); return -1; }



    ASM_EMIT("_exit:\n");
    ASM_EMIT("halt\n");
    
    for(i = 0; ee[i] != '\0'; i++) {
        printf("%c", ee[i]);
    }
}
