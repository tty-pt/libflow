#include <mov.h>
#include <stdio.h>

unsigned number_gen,
	 number_print,
	 number_gen_0,
	 number_print_0;

int mov_init() {
	number_gen = mov_node("number_gen");
	number_print = mov_node("number_print");
	number_gen_0 = mov_inst(number_gen);
	number_print_0 = mov_inst(number_print);
	mov_link(number_gen_0, 0, number_print_0, 0);
	mov_link_tran(number_gen_0, number_print_0);
	return 0;
}

int mov_run() {
	// auto-transition?
	mov_tran(number_gen_0);
	return 0;
}
