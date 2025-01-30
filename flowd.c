#include "./include/flow.h"
#include "./include/flowd.h"
#include <qhash.h>
#include <ndc.h>
#include <stdlib.h>

struct ndc_config ndc_config = { .flags = 0 };
DB_TXN *txnid;

void do_load(int fd, int argc, char *argv[]);
void do_run(int fd, int argc, char *argv[]);

struct cmd_slot cmds[] = {
	{
		.name = "LOAD",
		.cb = &do_load,
		.flags = CF_NOAUTH,
	}, {
		.name = "RUN",
		.cb = &do_run,
		.flags = CF_NOAUTH,
	}
};

void flow_tran_next(unsigned oinst);

char *ndc_auth_check(int fd) { return NULL; }
void ndc_update(unsigned long long dt) {}
void ndc_command(int fd, int argc, char *argv[]) {}
void ndc_vim(int fd, int argc, char *argv[]) {}
int ndc_connect(int fd) {}
void ndc_disconnect(int fd) {}

void do_load(int fd, int argc, char *argv[]) {
	unsigned me_class = flow_node(argv[1]);
	if (me_class == (unsigned) -1)
		ndc_writef(fd, "Failed to load class: %s\n", argv[1]);
	else
		ndc_writef(fd, "%u\n", me_class);
}

void do_run(int fd, int argc, char *argv[]) {
	unsigned me_class = strtoull(argv[1], NULL, 10);
	unsigned me = flow_inst(me_class);
	if (me == (unsigned) -1)
		ndc_writef(fd, "No such class id: %u\n", me_class);
	else
		flow_tran_next(-1);
}

int main(int argc, char *argv[]) {
	ndc_config.port = 4201;
	ndc_init(&ndc_config);
	flowd_init();
	ndc_main();
	return 0;
}
