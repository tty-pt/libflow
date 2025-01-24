#ifndef MOV_PAPI_H
#define MOV_PAPI_H

#include "./mov.h"

struct mov {
	mov_io_t *write, *read;
	mov_link_tran_t *link_tran;
	mov_link_t *link;
	mov_node_t *node;
	mov_inst_t *inst, *tran;
};

#endif
