#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "ll.h"


#define err(x) { fprintf(stderr, "%s\n", x); exit(1); }
#define pint(x) printf("%i\n", x);


/*
#define byte unsigned char
#define val unsigned short
#define exp unsigned int



#define MEM_SIZE (4096)
byte mem[MEM_SIZE];
val hp = 0, sp = MEM_SIZE;
*/







atom nil = { NIL };
atom env;
atom sym_table = { NIL };



atom cons(atom car, atom cdr) {
    atom a;
    a.t = PAIR;
    a.v.p = malloc(sizeof(pair));
    car(a) = car;
    cdr(a) = cdr;
    return a;
}


atom make_int(long i) {
    atom a;
    a.t = INTEGER;
    a.v.i = i;
    return a;
}


atom make_sym(const char *s) {
    atom a, p;

    for(p = sym_table; !isnil(p); p = cdr(p)) {
        a = car(p);
        if(strcmp(a.v.s, s) == 0)
            return a;
    }

    a.t = SYMBOL;
    a.v.s = strdup(s);

    sym_table = cons(a, sym_table);

    return a;
}

atom make_builtin(builtin b) {
    atom a;
    a.t = BUILTIN;
    a.v.b = b;
    return a;
}

error make_clos(atom env, atom args, atom body, atom *result) {
    atom p;

    if(!islist(body))
        return ERROR_SYNTAX;

    p = args;
    while(!isnil(p)) {
        if(p.t == SYMBOL)
            break;
        else if(p.t != PAIR || car(p).t != SYMBOL)
            return ERROR_TYPE;

        p = cdr(p);
    }

    *result = cons(env, cons(args, body));
    result->t = CLOSURE;

    return ERROR_OK;
}


void print_expr(atom a) {
    switch(a.t) {
    case NIL:
        printf("NIL");
        break;
    case PAIR:
        putchar('(');
        print_expr(car(a));
        a = cdr(a);
        while(!isnil(a)) {
            if(a.t == PAIR) {
                putchar(' ');
                print_expr(car(a));
                a = cdr(a);
            } else {
                printf(" . ");
                print_expr(a);
                break;
            }
        }
        putchar(')');
        break;
    case SYMBOL:
        printf("%s", a.v.s);
        break;
    case INTEGER:
        printf("%ld", a.v.i);
        break;
    case BUILTIN:
        printf("#<BUILTIN:%p>", a.v.b);
        break;
    case CLOSURE:
        printf("#<CLOSURE:%p>", a.v.b);
        break;
    case MACRO:
        printf("#<MACRO:%p>", a.v.p);
        break;
    }
}


void print_err(error err) {
    switch(err) {
    case ERROR_OK:
        putchar('\n');
        break;
    case ERROR_SYNTAX:
        puts("Syntax error");
        break;
    case ERROR_UNBOUND:
        puts("Symbol not bound");
        break;
    case ERROR_ARGS:
        puts("Wrong number of arguments");
        break;
    case ERROR_TYPE:
        puts("Wrong type");
        break;
    default:
        puts("Unknown error");
        break;
    }
}


error lex(const char *s, const char **start, const char **end) {
    while(isspace(*s))
        s += 1;

    if(*s == '\0') {
        *start = NULL;
        *end = NULL;
        return ERROR_SYNTAX;
    }

    *start = s;

    if(*s == '(' || *s == ')' || *s == '\'' || *s == '`')
        *end = s+1;
    else if(*s == ',')
        *end = s + (*(s+1) == '@' ? 2 : 1);
    else {
        /*while(isgraph(*s) && *s != '(' && *s != ')')
            s += 1;
        *end = s;*/
        *end = s + strcspn(s, "() \t\n");
    }

    return ERROR_OK;
}


error parse(const char *start, const char *end, atom *result) {
    char *p;
    long i = strtol(start, &p, 10);
    if(p == end) {
        *result = make_int(i);
        return ERROR_OK;
    }

    char *buf = malloc(end-start+1);
    p = buf;
    while(start != end)
        *p++ = toupper(*start++);
    *p = '\0';

    if(strcmp(buf, "NIL") == 0)
        *result = nil;
    else
        *result = make_sym(buf);

    free(buf);
    return ERROR_OK;
}


error read_list(const char *start, const char **end, atom *result) {
    atom p = nil;

    *end = start;
    *result = nil;

    for(;;) {
        const char *token;
        atom item;
        error err;

        err = lex(*end, &token, end);
        if(err != ERROR_OK)
            return err;

        if(*token == ')')
            return ERROR_OK;

        if(*token == '.' && *end - token == 1) {
            if(isnil(p))
                return ERROR_SYNTAX;

            err = read_expr(*end, end, &item);
            if(err != ERROR_OK)
                return err;

            cdr(p) = item;

            err = lex(*end, &token, end);
            if(err == ERROR_OK && *token != ')')
                err = ERROR_SYNTAX;

            return err;
        }

        err = read_expr(token, end, &item);
        if(err != ERROR_OK)
            return err;

        if(isnil(p)) {
            *result = cons(item, nil);
            p = *result;
        } else {
            cdr(p) = cons(item, nil);
            p = cdr(p);
        }
    }
}


error read_expr(const char *input, const char **end, atom *result) {
    const char *token;
    error err;

    err = lex(input, &token, end);
    if(err != ERROR_OK)
        return err;

    if(*token == '(')
        return read_list(*end, end, result);
    else if(*token == ')')
        return ERROR_SYNTAX;
    else if(*token == '\'') {
        *result = cons(make_sym("QUOTE"), cons(nil, nil));
        return read_expr(*end, end, &car(cdr(*result)));
    } else if(*token == '`') {
        *result = cons(make_sym("QUASIQUOTE"), cons(nil, nil));
        return read_expr(*end, end, &car(cdr(*result)));
    } else if(*token == ',') {
        *result = cons(make_sym(
                           token[1] == '@' ? "UNQUOTE-SPLICING" : "UNQUOTE"),
                       cons(nil, nil));
        return read_expr(*end, end, &car(cdr(*result)));
    } else
        return parse(token, *end, result);
}


atom env_create(atom parent) {
    return cons(parent, nil);
}


error env_get(atom env, atom symbol, atom *result) {
    atom parent = car(env);
    atom bs = cdr(env);

    while(!isnil(bs)) {
        atom b = car(bs);
        if(car(b).v.s == symbol.v.s) {
            *result = cdr(b);
            return ERROR_OK;
        }
        bs = cdr(bs);
    }

    if(isnil(parent))
        return ERROR_UNBOUND;

    return env_get(parent, symbol, result);
}


error env_set(atom env, atom symbol, atom value) {
    atom bs = cdr(env);
    atom b = nil;

    while(!isnil(bs)) {
        b = car(bs);
        if(car(b).v.s == symbol.v.s) {
            cdr(b) = value;
            return ERROR_OK;
        }
        bs = cdr(bs);
    }

    b = cons(symbol, value);
    cdr(env) = cons(b, cdr(env));

    return ERROR_OK;
}


int islist(atom a) {
    while(!isnil(a))
        if(a.t != PAIR)
            return FALSE;
        else
            return TRUE;

    return TRUE;
}


error eval_expr(atom expr, atom env, atom *result) {
    atom op, args, p;
    error err;

    if(expr.t == SYMBOL)
        return env_get(env, expr, result);
    else if(expr.t != PAIR) {
        *result = expr;
        return ERROR_OK;
    }

    if(!islist(expr))
        return ERROR_SYNTAX;

    op = car(expr);
    args = cdr(expr);

    if(op.t == SYMBOL) {
        if(strcmp(op.v.s, "QUOTE") == 0) {
            if(isnil(args) || !isnil(cdr(args)))
                return ERROR_ARGS;

            *result = car(args);
            return ERROR_OK;
        } else if(strcmp(op.v.s, "DEFINE") == 0) {
            atom sym, val;

            if(isnil(args) || isnil(cdr(args)))
                return ERROR_ARGS;

            sym = car(args);
            if(sym.t == PAIR) {
                err = make_clos(env, cdr(sym), cdr(args), &val);
                sym = car(sym);
                if(sym.t != SYMBOL)
                    return ERROR_TYPE;
            } else if(sym.t == SYMBOL) {
                if(!isnil(cdr(cdr(args))))
                    return ERROR_ARGS;
                err = eval_expr(car(cdr(args)), env, &val);
            } else
                return ERROR_TYPE;

            if(err != ERROR_OK)
                return err;

            *result = sym;
            return env_set(env, sym, val);
        } else if (strcmp(op.v.s, "LAMBDA") == 0) {
            if(isnil(args) || isnil(cdr(args)))
                return ERROR_ARGS;

            return make_clos(env, car(args), cdr(args), result);
        } else if(strcmp(op.v.s, "IF") == 0) {
            atom cond, val;

            if(isnil(args) || isnil(cdr(args)) || isnil(cdr(cdr(args))) ||
                    !isnil(cdr(cdr(cdr(args)))))
                return ERROR_ARGS;

            err = eval_expr(car(args), env, &cond);
            if(err != ERROR_OK)
                return err;

            val = isnil(cond) ? car(cdr(cdr(args))) : car(cdr(args));
            return eval_expr(val, env, result);
        } else if(strcmp(op.v.s, "DEFMACRO") == 0) {
            atom name, macro;
            error err;

            if(isnil(args) || isnil(cdr(args)))
                return ERROR_ARGS;

            if(car(args).t != PAIR)
                return ERROR_SYNTAX;

            name = car(car(args));
            if(name.t != SYMBOL)
                return ERROR_TYPE;

            err = make_clos(env, cdr(car(args)), cdr(args), &macro);
            if(err != ERROR_OK)
                return err;

            macro.t = MACRO;
            *result = name;
            return env_set(env, name, macro);
        }
    }

    err = eval_expr(op, env, &op);
    if(err != ERROR_OK)
        return err;

    if(op.t == MACRO) {
        atom expansion;
        op.t = CLOSURE;

        err = apply(op, args, &expansion);
        if(err != ERROR_OK)
            return err;

        return eval_expr(expansion, env, result);
    }

    args = copy_list(args);
    p = args;
    while(!isnil(p)) {
        err = eval_expr(car(p), env, &car(p));
        if(err != ERROR_OK)
            return err;

        p = cdr(p);
    }

    return apply(op, args, result);
}


atom copy_list(atom list) {
    atom a, p;

    if(isnil(list))
        return nil;

    a = cons(car(list), nil);
    p = a;
    list = cdr(list);

    while(!isnil(list)) {
        cdr(p) = cons(car(list), nil);
        p = cdr(p);
        list = cdr(list);
    }

    return a;
}


error apply(atom a, atom args, atom *result) {
    atom env, arg_names, body;

    if(a.t == BUILTIN)
        return (*a.v.b)(args, result);
    else if(a.t != CLOSURE)
        return ERROR_TYPE;

    env = env_create(car(a));
    arg_names = car(cdr(a));
    body = cdr(cdr(a));

    while(!isnil(arg_names)) {
        if(arg_names.t == SYMBOL) {
            env_set(env, arg_names, args);
            args = nil;
            break;
        }

        if(isnil(args))
            return ERROR_ARGS;

        env_set(env, car(arg_names), car(args));
        arg_names = cdr(arg_names);
        args = cdr(args);
    }

    if(!isnil(args))
        return ERROR_ARGS;

    while(!isnil(body)) {
        error err = eval_expr(car(body), env, result);
        if(err != ERROR_OK)
            return err;

        body = cdr(body);
    }

    return ERROR_OK;
}


error builtin_car(atom args, atom *result) {
    if(isnil(args) || !isnil(cdr(args)))
        return ERROR_ARGS;

    if(isnil(car(args)))
        *result = nil;
    else if(car(args).t != PAIR)
        return ERROR_TYPE;
    else
        *result = car(car(args));

    return ERROR_OK;
}


error builtin_cdr(atom args, atom *result) {
    if(isnil(args) || !isnil(cdr(args)))
        return ERROR_ARGS;

    if(isnil(car(args)))
        *result = nil;
    else if(car(args).t != PAIR)
        return ERROR_TYPE;
    else
        *result = cdr(car(args));

    return ERROR_OK;
}


error builtin_cons(atom args, atom *result) {
    if(isnil(args) || isnil(cdr(args)) || !isnil(cdr(cdr(args))))
        return ERROR_ARGS;

    *result = cons(car(args), car(cdr(args)));

    return ERROR_OK;
}


error builtin_add(atom args, atom *result) {
    atom a, b;

    if(isnil(args) || isnil(cdr(args)) || !isnil(cdr(cdr(args))))
        return ERROR_ARGS;

    a = car(args);
    b = car(cdr(args));

    if(a.t != INTEGER || b.t != INTEGER)
        return ERROR_TYPE;

    *result = make_int(a.v.i + b.v.i);

    return ERROR_OK;
}


error builtin_sub(atom args, atom *result) {
    atom a, b;

    if(isnil(args) || isnil(cdr(args)) || !isnil(cdr(cdr(args))))
        return ERROR_ARGS;

    a = car(args);
    b = car(cdr(args));

    if(a.t != INTEGER || b.t != INTEGER)
        return ERROR_TYPE;

    *result = make_int(a.v.i - b.v.i);

    return ERROR_OK;
}


error builtin_mul(atom args, atom *result) {
    atom a, b;

    if(isnil(args) || isnil(cdr(args)) || !isnil(cdr(cdr(args))))
        return ERROR_ARGS;

    a = car(args);
    b = car(cdr(args));

    if(a.t != INTEGER || b.t != INTEGER)
        return ERROR_TYPE;

    *result = make_int(a.v.i * b.v.i);

    return ERROR_OK;
}


error builtin_div(atom args, atom *result) {
    atom a, b;

    if(isnil(args) || isnil(cdr(args)) || !isnil(cdr(cdr(args))))
        return ERROR_ARGS;

    a = car(args);
    b = car(cdr(args));

    if(a.t != INTEGER || b.t != INTEGER)
        return ERROR_TYPE;

    *result = make_int(a.v.i / b.v.i);

    return ERROR_OK;
}


error builtin_mod(atom args, atom *result) {
    atom a, b;

    if(isnil(args) || isnil(cdr(args)) || !isnil(cdr(cdr(args))))
        return ERROR_ARGS;

    a = car(args);
    b = car(cdr(args));

    if(a.t != INTEGER || b.t != INTEGER)
        return ERROR_TYPE;

    *result = make_int(a.v.i % b.v.i);

    return ERROR_OK;
}


error builtin_eq(atom args, atom *result) {
    atom a, b;

    if(isnil(args) || isnil(cdr(args)) || !isnil(cdr(cdr(args))))
        return ERROR_ARGS;

    a = car(args);
    b = car(cdr(args));

    if(a.t != INTEGER || b.t != INTEGER)
        return ERROR_TYPE;

    *result = a.v.i == b.v.i ? make_sym("T") : nil;

    return ERROR_OK;
}


error builtin_gt(atom args, atom *result) {
    atom a, b;

    if(isnil(args) || isnil(cdr(args)) || !isnil(cdr(cdr(args))))
        return ERROR_ARGS;

    a = car(args);
    b = car(cdr(args));

    if(a.t != INTEGER || b.t != INTEGER)
        return ERROR_TYPE;

    *result = a.v.i > b.v.i ? make_sym("T") : nil;

    return ERROR_OK;
}


error builtin_lt(atom args, atom *result) {
    atom a, b;

    if(isnil(args) || isnil(cdr(args)) || !isnil(cdr(cdr(args))))
        return ERROR_ARGS;

    a = car(args);
    b = car(cdr(args));

    if(a.t != INTEGER || b.t != INTEGER)
        return ERROR_TYPE;

    *result = a.v.i < b.v.i ? make_sym("T") : nil;

    return ERROR_OK;
}


error builtin_apply(atom args, atom *result) {
    atom a;

    if(isnil(args) || isnil(cdr(args)) || !isnil(cdr(cdr(args))))
        return ERROR_ARGS;

    a = car(args);
    args = car(cdr(args));

    if(!islist(args))
        return ERROR_SYNTAX;

    return apply(a, args, result);
}


error builtin_isequ(atom args, atom *result) {
    atom a, b;
    int equ;

    if(isnil(args) || isnil(cdr(args)) || !isnil(cdr(cdr(args))))
        return ERROR_ARGS;

    a = car(args);
    b = car(cdr(args));

    if(a.t == b.t) {
        switch(a.t) {
        case NIL:
            equ = TRUE;
            break;
        case PAIR:
        case CLOSURE:
        case MACRO:
            equ = a.v.p == b.v.p ? TRUE : FALSE;
            break;
        case SYMBOL:
            equ = a.v.s == b.v.s ? TRUE : FALSE;
            break;
        case INTEGER:
            equ = a.v.i == b.v.i ? TRUE : FALSE;
            break;
        case BUILTIN:
            equ = a.v.b == b.v.b ? TRUE : FALSE;
            break;
        }
    } else
        equ = FALSE;

    *result = equ ? make_sym("T") : nil;

    return ERROR_OK;
}


error builtin_ispair(atom args, atom *result) {
    if(isnil(args) || !isnil(cdr(args)))
        return ERROR_ARGS;

    *result = (car(args).t == PAIR) ? make_sym("T") : nil;
    return ERROR_OK;
}



char *read_file(const char *path) {
    FILE *file;
    char *buf;
    long len;

    file = fopen(path, "r");
    if (!file)
        return NULL;
    fseek(file, 0, SEEK_END);
    len = ftell(file);
    fseek(file, 0, SEEK_SET);

    buf = malloc(len + 1);
    if (!buf)
        return NULL;

    fread(buf, 1, len, file);
    buf[len] = 0;
    fclose(file);

    return buf;
}


void load_file(atom env, const char *path) {
    char *txt;

    /*printf("Reading %s...\n", path);*/
    txt = read_file(path);
    if(txt != NULL) {
        const char *p = txt;
        atom expr;
        while(read_expr(p, &p, &expr) == ERROR_OK) {
            atom result;
            error err = eval_expr(expr, env, &result);
            if(err != ERROR_OK) {
                printf("Error in expression:\n\t");
                print_expr(expr);
                putchar('\n');
            }
            /* Stay silent if there are no problems
            else {
            	print_expr(result);
            	putchar('\n');
            }
            */
        }
        free(txt);
    }
}







void init(void) {
    env = env_create(nil);
    env_set(env, make_sym("CAR"), make_builtin(builtin_car));
    env_set(env, make_sym("CDR"), make_builtin(builtin_cdr));
    env_set(env, make_sym("CONS"), make_builtin(builtin_cons));
    env_set(env, make_sym("+"), make_builtin(builtin_add));
    env_set(env, make_sym("-"), make_builtin(builtin_sub));
    env_set(env, make_sym("*"), make_builtin(builtin_mul));
    env_set(env, make_sym("/"), make_builtin(builtin_div));
    env_set(env, make_sym("%"), make_builtin(builtin_mod));
    env_set(env, make_sym("T"), make_sym("T"));
    env_set(env, make_sym("="), make_builtin(builtin_eq));
    env_set(env, make_sym(">"), make_builtin(builtin_gt));
    env_set(env, make_sym("<"), make_builtin(builtin_lt));
    env_set(env, make_sym("APPLY"), make_builtin(builtin_apply));
    env_set(env, make_sym("EQ?"), make_builtin(builtin_isequ));
    env_set(env, make_sym("PAIR?"), make_builtin(builtin_ispair));

    load_file(env, "./lib.l");
    load_file(env, "../lib.l");
}


void repl(void) {

    char *buf = malloc(1024);
    while(fgets(buf, 1023, stdin) != NULL) {
        atom expr, result;
        const char *p = buf;

        error err = read_expr(p, &p, &expr);

        if(err == ERROR_OK)
            err = eval_expr(expr, env, &result);

        if(err == ERROR_OK)
            print_expr(result);

        print_err(err);

    }
    free(buf);
}











int main() {
    init();
    repl();

    return 0;
}


































