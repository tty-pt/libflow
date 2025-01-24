#ifndef FLOW_PAPI_H
#define FLOW_PAPI_H

#include "./flow.h"

struct flow {
	flow_io_t *write, *read;
	flow_link_tran_t *link_tran;
	flow_link_t *link;
	flow_node_t *node;
	flow_inst_t *inst, *tran;
};

#endif
