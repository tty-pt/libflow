#include <flow.h>
#include <stdio.h>

unsigned number_gen,
	 number_print,
	 numbers_print,
	 number_gen_0,
	 number_gen_1,
	 number_print_0,
	 numbers_print_0;

int flow_init() {
	number_gen = flow_node("number_gen");
	number_print = flow_node("number_print");
	numbers_print = flow_node("numbers_print");
	number_gen_0 = flow_inst(number_gen);
	number_gen_1 = flow_inst(number_gen);
	number_print_0 = flow_inst(number_print);
	numbers_print_0 = flow_inst(numbers_print);
	flow_link(number_gen_0, 0, number_print_0, 0);
	flow_link(number_gen_0, 0, numbers_print_0, 0);
	flow_link(number_gen_1, 0, numbers_print_0, 1);
	fprintf(stderr, "gen0 to nums0\n");
	flow_link_tran(number_gen_0, number_print_0);
	fprintf(stderr, "gen0 to num0\n");
	flow_link_tran(number_gen_0, numbers_print_0);
	// notice there's no transition to number_gen_1
	return 0;
}

int flow_run() {
	// auto-transition? for some
	/* flow_tran(number_gen_0); */
	return 0;
}
