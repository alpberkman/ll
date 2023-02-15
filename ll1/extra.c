void debug_heap(void) {
    int i;
    pf("hp: %i\n", hp);
    i = 0;
    while(i < hp)
        puts(mem+i), i += strlen(mem+i)+1;
}
void debug_stack(void) {
    int i;
    pf("hp: %i\n", sp);
    i = MEM_SIZE - 2*sizeof(exp);
    while(i >= sp) {
        exp y = *(exp*)(mem+i);
        exp x = *(exp*)(mem+i+sizeof(exp));
        printf("x: %i %i ", x.v, x.t);
        printf("y: %i %i\n", y.v, y.t);
        i -= 2*sizeof(exp);
    }
}
void debug(void) {
    debug_heap();
    debug_stack();
}

void gc(void) {
	sp = env.v;
	/*val i = env.v;
	sp = env.v;
	/*for(hp = 0; i < MEM_SIZE; ++i)
		if((*(exp *)(mem+i)).t == ATOM && (*(exp *)(mem+i)).v > hp)
			hp = (*(exp *)(mem+i)).v;
	hp += strlen(mem+i)+1;*/
}