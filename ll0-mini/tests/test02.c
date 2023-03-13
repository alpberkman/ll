#include <stdio.h>
#include "../ll.h"
#define NL puts("");


void test2(void) {
	print_expr(make_int(42));NL
	print_expr(make_sym("FOO"));NL
	print_expr(cons(make_sym("X"), make_sym("Y")));NL
	print_expr(cons(make_int(1), cons(make_int(2), cons(make_int(3), nil))));NL
}


int main() {
	test2();
	return 0;
}

