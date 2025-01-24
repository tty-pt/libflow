#include "./include/flow.h"
#include "./include/papi.h"

struct flow flow;

int flow_write(unsigned oport, void *data, size_t size) {
	return flow.write(oport, data, size);
}

int flow_read(unsigned iport, void *target, size_t size) {
	return flow.read(iport, target, size);
}

int flow_link_tran(unsigned oinst, unsigned iinst) {
	return flow.link_tran(oinst, iinst);
}

int flow_link(unsigned oinst, unsigned oport, unsigned iinst, unsigned iport) {
	return flow.link(oinst, oport, iinst, iport);
}

unsigned flow_node(char *name) {
	return flow.node(name);
}

unsigned flow_inst(unsigned class_id) {
	return flow.inst(class_id);
}

unsigned flow_tran(unsigned inst_id) {
	return flow.tran(inst_id);
}
