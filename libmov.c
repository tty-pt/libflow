#include "./include/mov.h"
#include "./include/papi.h"

struct mov mov;

int mov_write(unsigned oport, void *data, size_t size) {
	return mov.write(oport, data, size);
}

int mov_read(unsigned iport, void *target, size_t size) {
	return mov.read(iport, target, size);
}

int mov_link_tran(unsigned oinst, unsigned iinst) {
	return mov.link_tran(oinst, iinst);
}

int mov_link(unsigned oinst, unsigned oport, unsigned iinst, unsigned iport) {
	return mov.link(oinst, oport, iinst, iport);
}

unsigned mov_node(char *name) {
	return mov.node(name);
}

unsigned mov_inst(unsigned class_id) {
	return mov.inst(class_id);
}

unsigned mov_tran(unsigned inst_id) {
	return mov.tran(inst_id);
}
