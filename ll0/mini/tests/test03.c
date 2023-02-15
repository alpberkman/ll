#include <stdio.h>
#include "../ll.h"
#define NL puts("");


void test3(void) {
    atom expr;
    const char *p1 = "42";
    read_expr(p1, &p1, &expr);
    print_expr(expr);NL
    const char *p2 = "(foo bar)";
    read_expr(p2, &p1, &expr);
    print_expr(expr);NL
    const char *p3 = "(s (t . u) v . (w . nil))";
    read_expr(p3, &p1, &expr);
    print_expr(expr);NL
    const char *p4 = "()";
    read_expr(p4, &p1, &expr);
    print_expr(expr);NL
    
}


int main() {
	test3();
	return 0;
}

