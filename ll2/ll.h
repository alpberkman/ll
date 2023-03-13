#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MEM_SIZE 0x10000
#define err(S) { fprintf(stderr, "%s\n", (S)); abort(); }


typedef struct VM VM;

typedef int val;
typedef enum tag tag;
typedef struct exp exp;
typedef unsigned char byte;

typedef exp (*fun)(exp, exp);


enum tag {
    ATOM,
    PRIM,
    CONS,
    CLOS,
    NIL,
    INT,
    FLT,
};

struct exp {
    val v;
    tag t;
};

struct VM {
    byte mem[MEM_SIZE];
    val hp;
    val sp;

    exp nil;
    exp tru;
    exp err;
    exp env;
};


void init(VM *vm);
exp atom(VM *vm, const char *s);
exp cons(VM *vm, exp x, exp y);
exp car(VM *vm, exp p);
exp cdr(VM *vm, exp p);
exp pair(VM *vm, exp v, exp x, exp e);

exp box(val v, tag t);
val equ(exp x, exp y);
val not(exp x);


exp box(val v, tag t) {
    exp e = {.v = v, .t = t};
    return e;
}

val equ(exp x, exp y) {
    return x.v == y.v && x.t == y.t;
}

val not(exp x) {
    return x.t == NIL;
}




void init(VM *vm) {
    int i;
    
    for(i = 0; i < MEM_SIZE; ++i)
        vm->mem[i] = 0;
    vm->hp = 0;
    vm->sp = MEM_SIZE;

    vm->nil = box(NIL, 0);
    vm->tru = atom(vm, "#t");
    vm->err = atom(vm, "ERR");
    vm->env = pair(vm, vm->tru, vm->tru, vm->nil);
}

exp atom(VM *vm, const char *s) {
    val addr = 0;
    val len = strlen(s);

    while(addr < vm->hp && strcmp((char *)&vm->mem[addr], s))
        addr += strlen((char *)&vm->mem[addr])+1;

    if(addr == vm->hp) {
        if(vm->hp+len+1 > vm->sp)
            err("Out of heap space");

        memcpy(&vm->mem[addr], s, len+1);
        vm->hp += len+1;
    }

    return box(addr, ATOM);
}

exp cons(VM *vm, exp x, exp y) {
    if(vm->sp-((val)sizeof(exp)*2) < vm->hp)
        err("Out of stack space");

    vm->sp -= sizeof(exp);
    *(exp*)&vm->mem[vm->sp] = x;
    vm->sp -= sizeof(exp);
    *(exp*)&vm->mem[vm->sp] = y;

    return box(vm->sp, CONS);
}

exp car(VM *vm, exp p) {
    return p.t == CONS || p.t == CLOS ? *(exp*)&vm->mem[p.v+sizeof(exp)] : vm->err;
}

exp cdr(VM *vm, exp p) {
    return p.t == CONS || p.t == CLOS ? *(exp*)&vm->mem[p.v] : vm->err;
}

exp pair(VM *vm, exp v, exp x, exp e) {
    return cons(vm, cons(vm, v, x), e);
}

exp clos(VM *vm, exp v, exp x, exp e) {
    return box(pair(vm, v, x, equ(e, vm->env) ? vm->nil : e).v, CLOS);
}













