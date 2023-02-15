#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>



typedef char byte;
typedef int val;
typedef enum tag tag;
typedef struct exp exp;
typedef exp (*fun)(exp, exp);
typedef struct ops ops;

exp box(val v, tag t);
val equ(exp x, exp y);
val not(exp x);
exp atom(const char *s);
exp cons(exp x, exp y);
exp car(exp p);
exp cdr(exp p);
exp pair(exp n, exp x, exp e);
exp clos(exp n, exp x, exp e);
exp assoc(exp n, exp e);
exp eval(exp x, exp e);
exp apply(exp f, exp t, exp e);
exp reduce(exp f, exp t, exp e);
exp evlis(exp t, exp e);
exp bind(exp v, exp t, exp e);

exp f_eval(exp t, exp e);
exp f_quote(exp t, exp e);
exp f_cons(exp t, exp e);
exp f_car(exp t, exp e);
exp f_cdr(exp t, exp e);
exp f_add(exp t, exp e);
exp f_sub(exp t, exp e);
exp f_mul(exp t, exp e);
exp f_div(exp t, exp e);
exp f_mod(exp t, exp e);
exp f_gt(exp t, exp e);
exp f_lt(exp t, exp e);
exp f_eq(exp t, exp e);
exp f_not(exp t, exp e);
exp f_and(exp t, exp e);
exp f_or(exp t, exp e);
exp f_cond(exp t, exp e);
exp f_if(exp t, exp e);
exp f_leta(exp t, exp e);
exp f_lambda(exp t, exp e);
exp f_define(exp t, exp e);
exp f_env(exp t, exp e);
exp f_debug(exp t, exp e);
exp f_print_list(exp t, exp e);

void print(exp x);
void printlist(exp t);

void error(const char *s);
void gc(void);
void init(void);
void repl(void);



enum tag {
    ATOM,
    PRIM,
    CONS,
    CLOS,
    NIL,
    INT,
};

struct exp {
    val v;
    tag t;
};

struct ops {
    const char *s;
    fun f;
};



#define MEM_SIZE (8192<<3)
byte mem[MEM_SIZE];
val hp = 0, sp = MEM_SIZE;
exp nil, tru, err, env;

ops prim[] = {
    {"eval", f_eval},
    {"quote", f_quote},
    {"cons", f_cons},
    {"car", f_car},
    {"cdr", f_cdr},
    {"+", f_add},
    {"-", f_sub},
    {"*", f_mul},
    {"/", f_div},
    {"%", f_mod},
    {">", f_gt},
    {"<", f_lt},
    {"eq?", f_eq},
    {"not", f_not},
    {"and", f_and},
    {"or", f_or},
    {"cond", f_cond},
    {"if", f_if},
    {"let*", f_leta},
    {"lambda", f_lambda},
    {"define", f_define},
    {"env", f_env},
    {"debug", f_debug},
    {"print-list", f_print_list},
    {0}
};



exp box(val v, tag t) {
    exp e;
    e.v = v;
    e.t = t;
    return e;
}

val equ(exp x, exp y) {
    return x.v == y.v && x.t == y.t;
}

val not(exp x) {
    return x.t == NIL;
}

exp atom(const char *s) {
    val i = 0;

    while(i < hp && strcmp(mem+i, s))
        i += strlen(mem+i)+1;

    if(i == hp) {
        if(hp+(strlen(s)+1) > sp)
            error("Heap overflow");

        memcpy(mem+i, s, strlen(s)+1);
        hp += strlen(s)+1;
    }

    return box(i, ATOM);
}

exp cons(exp x, exp y) {
    if(sp-(2*sizeof(exp)) < hp)
        error("Stack overflow");

    sp -= sizeof(exp);
    *(exp*)(mem+sp) = x;
    sp -= sizeof(exp);
    *(exp*)(mem+sp) = y;

    return box(sp, CONS);
}

exp car(exp p) {
    return p.t == CONS || p.t == CLOS ? *(exp*)(mem+p.v+sizeof(exp)) : err;
}

exp cdr(exp p) {
    return p.t == CONS || p.t == CLOS ? *(exp*)(mem+p.v) : err;
}

exp pair(exp n, exp x, exp e) {
    return cons(cons(n, x), e);
}

exp clos(exp n, exp x, exp e) {
    return box(pair(n, x, equ(e, env) ? nil : e).v, CLOS);
}

exp assoc(exp n, exp e) {
    while(e.t == CONS && !equ(n, car(car(e))))
        e = cdr(e);

    return e.t == CONS ? cdr(car(e)) : err;
}

exp eval(exp x, exp e) {
    if(x.t == ATOM)
        return assoc(x, e);
    else if(x.t == CONS)
        return apply(eval(car(x), e), cdr(x), e);
    else
        return x;
}

exp apply(exp f, exp t, exp e) {
    if(f.t == PRIM)
        return prim[f.v].f(t, e);
    else if(f.t == CLOS)
        return reduce(f, t, e);
    else
        return err;
}

exp reduce(exp f, exp t, exp e) {
    return eval(cdr(car(f)), bind(car(car(f)), evlis(t, e), not(cdr(f)) ? env : cdr(f)));
}

exp evlis(exp t, exp e) {
    if(t.t == CONS)
        return cons(eval(car(t), e), evlis(cdr(t), e));
    else if(t.t == ATOM)
        return assoc(t, e);
    else
        return nil;
}

exp bind(exp v, exp t, exp e) {
    if(v.t == NIL)
        return e;
    else if(v.t == CONS)
        return bind(cdr(v), cdr(t), pair(car(v), car(t), e));
    else
        return pair(v, t, e);
}


exp f_eval(exp t, exp e) {
    return eval(car(evlis(t, e)), e);
}

exp f_quote(exp t, exp e) {
    return car(t);
}

exp f_cons(exp t, exp e) {
    t = evlis(t, e);
    return cons(car(t), car(cdr(t)));
}

exp f_car(exp t, exp e) {
    return car(car(evlis(t, e)));
}

exp f_cdr(exp t, exp e) {
    return cdr(car(evlis(t, e)));
}

exp f_add(exp t, exp e) {
    exp n;
    t = evlis(t, e);
    n = car(t);
    while(!not(t = cdr(t)))
        n.v += car(t).v;
    return n;
}

exp f_sub(exp t, exp e) {
    exp n;
    t = evlis(t, e);
    n = car(t);
    while(!not(t = cdr(t)))
        n.v -= car(t).v;
    return n;
}

exp f_mul(exp t, exp e) {
    exp n;
    t = evlis(t, e);
    n = car(t);
    while(!not(t = cdr(t)))
        n.v *= car(t).v;
    return n;
}

exp f_div(exp t, exp e) {
    exp n;
    t = evlis(t, e);
    n = car(t);
    while(!not(t = cdr(t)))
        n.v /= car(t).v;
    return n;
}

exp f_mod(exp t, exp e) {
    exp n;
    t = evlis(t, e);
    n = car(t);
    while(!not(t = cdr(t)))
        n.v %= car(t).v;
    return n;
}

exp f_gt(exp t, exp e) {
    t = evlis(t, e);
    return car(t).v > car(cdr(t)).v ? tru : nil;
}

exp f_lt(exp t, exp e) {
    t = evlis(t, e);
    return car(t).v < car(cdr(t)).v ? tru : nil;
}

exp f_eq(exp t, exp e) {
    t = evlis(t, e);
    return equ(car(t), car(cdr(t))) ? tru : nil;
}

exp f_not(exp t, exp e) {
    return not(car(evlis(t, e))) ? tru : nil;
}

exp f_and(exp t, exp e) {
    exp x = nil;
    while(t.t != NIL && !not(x = eval(car(t), e)))
        t = cdr(t);
    return x;
}

exp f_or(exp t, exp e) {
    exp x = nil;
    while(t.t != NIL && not(x = eval(car(t), e)))
        t = cdr(t);
    return x;
}

exp f_cond(exp t, exp e) {
    while(t.t != NIL && not(eval(car(car(t)), e)))
        t = cdr(t);
    return eval(car(cdr(car(t))), e);
}

exp f_if(exp t, exp e) {
    return eval(car(cdr(not(eval(car(t), e)) ? cdr(t) : t)), e);
}

exp f_leta(exp t, exp e) {
    for(; t.t != NIL && !not(cdr(t)); t = cdr(t))
        e = pair(car(car(t)), eval(car(cdr(car(t))), e), e);
    return eval(car(t), e);
}

exp f_lambda(exp t, exp e) {
    return clos(car(t), car(cdr(t)), e);
}

exp f_define(exp t, exp e) {
    env = pair(car(t), eval(car(cdr(t)), e), env);
    return car(t);
}

exp f_env(exp t, exp e) {
    return env;
}

exp f_debug(exp t, exp e) {
    int i;
    for(i = 0; i < hp; i+= strlen(mem+i)+1)
        printf("%s ", mem+i);
    return nil;
}

exp f_print_list(exp t, exp e) {
    print(car(evlis(t, e)));
    return nil;
}

/*char buf[40];
int c;
char get(void) {
	int c = getchar();
	if(c == EOF)
		exit(0);
	return c;
}
char scan(void) {
	c = getchar();
	if(c == EOF)
		exit(0);

	while(isspace(c))
		c = getchar();

}*/



/* tokenization buffer and the next character that we are looking at */
char buf[40], see = ' ';

/* advance to the next character */
void look() {
    int c = getchar();
    see = c;
    if (c == EOF)
        exit(0);
}

/* return nonzero if we are looking at character c, ' ' means any white space */
val seeing(char c) {
    return c == ' ' ? see > 0 && see <= c : see == c;
}

/* return the look ahead character from standard input, advance to the next */
char get() {
    char c = see;
    look();
    return c;
}

/* tokenize into buf[], return first character of buf[] */
char scan() {
    val i = 0;
    while (seeing(' '))
        look();
    if (seeing('(') || seeing(')') || seeing('\''))
        buf[i++] = get();
    else
        do
            buf[i++] = get();
        while (i < 39 && !seeing('(') && !seeing(')') && !seeing(' '));
    buf[i] = 0;
    return *buf;
}

/* return the Lisp expression read from standard input */
exp parse();
exp read() {
    scan();
    return parse();
}

/* return a parsed Lisp list */
exp list() {
    exp x;
    if (scan() == ')')
        return nil;
    if (!strcmp(buf, ".")) {
        x = read();
        scan();
        return x;
    }
    x = parse();
    return cons(x, list());
}

/* return a parsed Lisp expression x quoted as (quote x) */
exp quote() {
    return cons(atom("quote"), cons(read(), nil));
}

/* return a parsed atomic Lisp expression (a number or an atom) */
exp atomic() {
    exp n;
    val i;
    n.t = INT;
    return (sscanf(buf, "%d%n", &(n.v), &i) > 0 && !buf[i]) ? n :
           atom(buf);
}

/* return a parsed Lisp expression */
exp parse() {
    return *buf == '(' ? list() :
           *buf == '\'' ? quote() :
           atomic();
}





void print(exp x) {
    switch(x.t) {
    case ATOM:
        printf("%s", mem + x.v);
        break;
    case PRIM:
        printf("<%s>", prim[x.v].s);
        break;
    case CONS:
        printlist(x);
        break;
    case CLOS:
    	putchar('{');
        printlist(x);
        putchar('}');
        break;
    case NIL:
        printf("()");
        break;
    case INT:
        printf("[%i]", x.v);
        break;
    default:
        error("Unknown tag in print");
    }
}

void printlist(exp t) {
    putchar('(');
    for(;;) {
        print(car(t));
        t = cdr(t);
        if(not(t))
            break;
        if(t.t != CONS) {
            printf(" . ");
            print(t);
            break;
        }
        putchar(' ');
    }
    putchar(')');
}


void error(const char *s) {
    fprintf(stderr, "%s\n", s);
    printf("sp:%i hp:%i\n ", sp, hp);
    print(env);
    f_debug(nil, nil);
    exit(1);
}

void gc(void) {
    sp = env.v;
}

void init(void) {
    nil = box(0, NIL);
    tru = atom("#t");
    err = atom("ERR");
    env = pair(tru, tru, nil);

    int i;
    for(i = 0; prim[i].s != NULL; ++i)
        env = pair(atom(prim[i].s), box(i, PRIM), env);
}

void repl(void) {
    for(;;) {
        printf("\n%i>>> ", sp-hp);
        print(eval(read(), env));
        gc();
    }
}


int main() {
    init();
    repl();
    return 0;
}





