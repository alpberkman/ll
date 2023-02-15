

#define FALSE 0
#define TRUE -1


char *strdup(const char *s) {
    int len = strlen(s)+1;
    char *p = malloc(len);
    return p == NULL ? NULL : memcpy(p, s, len);
}


typedef struct atom atom;
typedef struct pair pair;
typedef enum error error;
typedef error (*builtin)(atom args, atom *result);


struct atom {
    enum { NIL, PAIR, SYMBOL, INTEGER, BUILTIN, CLOSURE, MACRO } t;
    union {
        pair *p;
        const char *s;
        long i;
        builtin b;
    } v;
};

struct pair {
    atom car;
    atom cdr;
};

enum error {
    ERROR_OK,
    ERROR_SYNTAX,
    ERROR_UNBOUND,
    ERROR_ARGS,
    ERROR_TYPE,
};


#define isnil(x) ((x).t == NIL)
#define car(x) ((x).v.p->car)
#define cdr(x) ((x).v.p->cdr)


void init(void);
void repl(void);


/* Make atom */
atom cons(atom car, atom cdr);
atom make_int(long i);
atom make_sym(const char *s);
atom make_builtin(builtin b);
error make_clos(atom env, atom args, atom body, atom *result);


/* Print functions */
void print_expr(atom a);
void print_err(error err);


/* Lexer, parser */
error lex(const char *s, const char **start, const char **end);
error parse(const char *start, const char *end, atom *result);
error read_list(const char *start, const char **end, atom *result);
error read_expr(const char *input, const char **end, atom *result);


/* eval */
atom env_create(atom parent);
error env_get(atom env, atom symbol, atom *result);
error env_set(atom env, atom symbol, atom value);
int islist(atom a);
error eval_expr(atom expr, atom env, atom *result);
atom copy_list(atom list);
error apply(atom a, atom args, atom *result);


/* Builtins */
error builtin_car(atom args, atom *result);
error builtin_cdr(atom args, atom *result);
error builtin_cons(atom args, atom *result);

error builtin_add(atom args, atom *result);
error builtin_sub(atom args, atom *result);
error builtin_mul(atom args, atom *result);
error builtin_div(atom args, atom *result);
error builtin_mod(atom args, atom *result);
error builtin_eq(atom args, atom *result);
error builtin_gt(atom args, atom *result);
error builtin_lt(atom args, atom *result);
error builtin_apply(atom args, atom *result);
error builtin_isequ(atom args, atom *result);
error builtin_ispair(atom args, atom *result);


/* Library functions */
char *read_file(const char *path);
void load_file(atom env, const char *path);









































