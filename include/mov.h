#ifndef MOV_H
#define MOV_H

#include <stddef.h>

typedef int mov_io_t(unsigned port,
		void *data, size_t len);

mov_io_t mov_write, mov_read;

typedef int mov_link_tran_t(unsigned oinst, unsigned iinst);

mov_link_tran_t mov_link_tran;

typedef int mov_link_t(unsigned oinst, 
		unsigned oport,
		unsigned iinst,
		unsigned iport);

mov_link_t mov_link;

typedef unsigned mov_node_t(char *name);

mov_node_t mov_node;

typedef unsigned mov_inst_t(unsigned);

mov_inst_t mov_inst, mov_tran;

typedef int mov_run_t();

extern mov_run_t mov_run, mov_init;

#endif
