#ifndef FLOW_H
#define FLOW_H

#include <stddef.h>

typedef int flow_io_t(unsigned port,
		void *data, size_t len);

flow_io_t flow_write, flow_read;

typedef int flow_link_tran_t(unsigned oinst, unsigned iinst);

flow_link_tran_t flow_link_tran;

typedef int flow_link_t(unsigned oinst, 
		unsigned oport,
		unsigned iinst,
		unsigned iport);

flow_link_t flow_link;

typedef unsigned flow_node_t(char *name);

flow_node_t flow_node;

typedef unsigned flow_inst_t(unsigned);

flow_inst_t flow_inst, flow_tran;

typedef int flow_run_t();

extern flow_run_t flow_run, flow_init;

#endif
