#include <flow.h>
#include <stdio.h>

unsigned number_gen,
	 number_print,
	 number_gen_0,
	 number_print_0;

int flow_init() {
	number_gen = flow_node("number_gen");
	number_print = flow_node("number_print");
	number_gen_0 = flow_inst(number_gen);
	number_print_0 = flow_inst(number_print);
	flow_link(number_gen_0, 0, number_print_0, 0);
	flow_link_tran(number_gen_0, number_print_0);
	return 0;
}

int flow_run() {
	// auto-transition?
	flow_tran(number_gen_0);
	return 0;
}
