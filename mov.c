#include "./include/mov.h"
#include "./include/papi.h"
#include <dlfcn.h>
#include <qhash.h>
#include <stdlib.h>

unsigned class_hd, class_rhd, inst_hd, link_hd, tran_hd, data_hd;
struct mov mov;
unsigned me, parent;
DB_TXN *txnid;

int mov_write(unsigned oport, void *data, size_t size) {
	unsigned key[2] = { me, oport };
	/* fprintf(stderr, "mov_write %u %u\n", me, oport); */
	hash_put(data_hd, key, sizeof(key), data, size);
	return 0;
}

int mov_read(unsigned iport, void *target, size_t size) {
	unsigned key[2] = { parent, iport };
	/* fprintf(stderr, "mov_read %u %u\n", parent, iport); */
	hash_get(data_hd, target, key, sizeof(key));
	return 0;
}

int mov_link_tran(unsigned oinst, unsigned iinst) {
	uhash_put(tran_hd, oinst, &iinst, sizeof(iinst));
	return 0;
}

int mov_link(unsigned oinst, unsigned oport, unsigned iinst, unsigned iport) {
	unsigned key[2], value[2];
	hash_put(link_hd, key, sizeof(key), value, sizeof(value));
	return 0;
}

unsigned mov_node(char *name) {
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
	/* fprintf(stderr, "mov_node new %s %p %u\n", buf, sl, ret); */
	suhash_put(class_rhd, name, ret);

	struct mov *imov = dlsym(sl, "mov");
	*imov = mov;

	mov_run_t *mov_init = dlsym(sl, "mov_init");
	if (mov_init)
		(*mov_init)();

	return ret;
}

unsigned mov_inst(unsigned class_id) {
	unsigned ret = lhash_new(inst_hd, &class_id);
	/* fprintf(stderr, "mov_inst %u %u\n", class_id, ret); */
	return ret;
}

void mov_tran_next(unsigned oinst);

unsigned mov_tran(unsigned iinst) {
	void *sl;
	unsigned iclass;
	/* fprintf(stderr, "mov_tran -> %u\n", iinst); */

	uhash_get(inst_hd, &iclass, iinst);
	uhash_get(class_hd, &sl, iclass);
	mov_run_t *mov_run = dlsym(sl, "mov_run");
	me = iinst;
	(*mov_run)();

	mov_tran_next(iinst);
}

void mov_tran_next(unsigned oinst) {
	struct hash_cursor c = hash_iter(tran_hd, &oinst, sizeof(oinst));
	unsigned iinst, iclass;
	/* fprintf(stderr, "mov_tran_next %u\n", oinst); */

	while (hash_next(&oinst, &iinst, &c)) {
		parent = oinst;
		mov_tran(iinst);
	}
}

int main(int argc, char *argv[]) {
	class_hd = lhash_init(sizeof(void *));
	class_rhd = hash_init();
	inst_hd = lhash_init(sizeof(unsigned));
	link_hd = hash_init();
	tran_hd = hash_init();
	data_hd = hash_init();

	mov.read = mov_read;
	mov.write = mov_write;
	mov.link_tran = mov_link_tran;
	mov.link = mov_link;
	mov.node = mov_node;
	mov.inst = mov_inst;
	mov.tran = mov_tran;

	unsigned me_class = mov_node(argv[1]);
	parent = me = mov_inst(me_class);

	mov_tran(me);
	return 0;
}
