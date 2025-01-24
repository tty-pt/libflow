#include "./include/flow.h"
#include "./include/papi.h"
#include <dlfcn.h>
#include <qhash.h>
#include <stdlib.h>

unsigned class_hd, class_rhd, inst_hd, link_hd, tran_hd, data_hd;
struct flow flow;
unsigned me, parent;
DB_TXN *txnid;

int flow_write(unsigned oport, void *data, size_t size) {
	unsigned key[2] = { me, oport };
	/* fprintf(stderr, "flow_write %u %u\n", me, oport); */
	hash_put(data_hd, key, sizeof(key), data, size);
	return 0;
}

int flow_read(unsigned iport, void *target, size_t size) {
	unsigned key[2] = { parent, iport };
	/* fprintf(stderr, "flow_read %u %u\n", parent, iport); */
	hash_get(data_hd, target, key, sizeof(key));
	return 0;
}

int flow_link_tran(unsigned oinst, unsigned iinst) {
	uhash_put(tran_hd, oinst, &iinst, sizeof(iinst));
	return 0;
}

int flow_link(unsigned oinst, unsigned oport, unsigned iinst, unsigned iport) {
	unsigned key[2], value[2];
	hash_put(link_hd, key, sizeof(key), value, sizeof(value));
	return 0;
}

unsigned flow_node(char *name) {
	static char buf[BUFSIZ];
	unsigned ret;

	sprintf(buf, "./%s.so", name);

	if (!shash_get(class_rhd, &ret, name))
		return ret;

	void *sl = dlopen(buf, RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
	if (!sl) {
		fprintf(stderr, "dlopen failed: %s\n", dlerror());
		exit(1);
	}
	ret = lhash_new(class_hd, &sl);
	/* fprintf(stderr, "flow_node new %s %p %u\n", buf, sl, ret); */
	suhash_put(class_rhd, name, ret);

	struct flow *iflow = dlsym(sl, "flow");
	*iflow = flow;

	flow_run_t *flow_init = dlsym(sl, "flow_init");
	if (flow_init)
		(*flow_init)();

	return ret;
}

unsigned flow_inst(unsigned class_id) {
	unsigned ret = lhash_new(inst_hd, &class_id);
	/* fprintf(stderr, "flow_inst %u %u\n", class_id, ret); */
	return ret;
}

void flow_tran_next(unsigned oinst);

unsigned flow_tran(unsigned iinst) {
	void *sl;
	unsigned iclass;
	/* fprintf(stderr, "flow_tran -> %u\n", iinst); */

	uhash_get(inst_hd, &iclass, iinst);
	uhash_get(class_hd, &sl, iclass);
	flow_run_t *flow_run = dlsym(sl, "flow_run");
	me = iinst;
	(*flow_run)();

	flow_tran_next(iinst);
}

void flow_tran_next(unsigned oinst) {
	struct hash_cursor c = hash_iter(tran_hd, &oinst, sizeof(oinst));
	unsigned iinst, iclass;
	/* fprintf(stderr, "flow_tran_next %u\n", oinst); */

	while (hash_next(&oinst, &iinst, &c)) {
		parent = oinst;
		flow_tran(iinst);
	}
}

int main(int argc, char *argv[]) {
	class_hd = lhash_init(sizeof(void *));
	class_rhd = hash_init();
	inst_hd = lhash_init(sizeof(unsigned));
	link_hd = hash_init();
	tran_hd = hash_init();
	data_hd = hash_init();

	flow.read = flow_read;
	flow.write = flow_write;
	flow.link_tran = flow_link_tran;
	flow.link = flow_link;
	flow.node = flow_node;
	flow.inst = flow_inst;
	flow.tran = flow_tran;

	unsigned me_class = flow_node(argv[1]);
	parent = me = flow_inst(me_class);

	flow_tran(me);
	return 0;
}
