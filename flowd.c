#include "./include/flow.h"
#include "./include/papi.h"
#include <dlfcn.h>
#include <qhash.h>
#include <ndc.h>
#include <stdlib.h>
#include <Python.h>

#define MAX_STACK 524288

enum FLOW_FLAG {
	FF_PYTHON = 1,
};

struct flow_class {
	int flags;
	void *data;
};

struct ndc_config ndc_config = { .flags = 0 };
unsigned class_hd, class_rhd, inst_hd, link_hd, link_rhd,
	 tran_hd, tran_rhd, data_hd;
__thread unsigned me, has_init = 0;
__thread unsigned stack[MAX_STACK], stack_n;
struct flow flow;
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

void stack_debug(unsigned *key, unsigned n) {
	unsigned i = 0;
	fprintf(stderr, " [");
	while (i < n) {
		fprintf(stderr, " %u", key[i]);
		i++;
	}
	fprintf(stderr, " ]");
}

int flow_write(unsigned oport, void *data, size_t size) {
	unsigned link_key[2] = { me, oport }, link_value[2];
	unsigned key[MAX_STACK], id, n;
	struct hash_cursor c;

	memcpy(key + 1, stack, stack_n);
	fprintf(stderr, "(%u:%u write %u to", me, oport, * (unsigned *) data);

	link_key[0] = me;
	link_key[1] = oport;
	c = hash_iter(link_hd, link_key, sizeof(link_key));
	while (hash_next(link_key, link_value, &c)) {
		link_key[0] = me;
		link_key[1] = oport;
		memcpy(key, stack, sizeof(key));
		key[0] = link_value[1];
		key[1] = link_value[0];
		fprintf(stderr, " %u:%u", key[1], key[0]);
		stack_debug(stack, stack_n);
		stack_debug(key, stack_n + 1);
		hash_put(data_hd, key, sizeof(unsigned) * (stack_n + 1), data, size);
	}
	fprintf(stderr, ")\n");

	return 0;
}

int flow_read(unsigned iport, void *target, size_t size) {
	unsigned key[MAX_STACK];
	memcpy(key + 1, stack, sizeof(unsigned) * stack_n);
	key[0] = iport;
	hash_get(data_hd, target, key, sizeof(unsigned) * (stack_n + 1));
	fprintf(stderr, "(%u:%u read %u", me, iport, * (unsigned *) target);
	stack_debug(stack, stack_n);
	stack_debug(key, stack_n + 1);
	fprintf(stderr, ")\n");
	return 0;
}

/*
 * recursive auto-transitions linking
 */
void flow_link_tran_rec(unsigned oinst, unsigned iinst) {
	struct hash_cursor c;
	unsigned dinst, n = 0, ign;

	c = hash_iter(link_rhd, &iinst, sizeof(iinst));

	while (hash_next(&ign, &dinst, &c)) if (dinst != oinst) {
		flow_link_tran_rec(dinst, iinst);
		n++;
	}

	uhash_put(tran_hd, oinst, &iinst, sizeof(iinst));
	if (!n)
		uhash_put(tran_hd, -1, &oinst, sizeof(oinst));
}

int flow_link_tran(unsigned oinst, unsigned iinst) {
	flow_link_tran_rec(oinst, iinst);
	return 0;
}

int flow_link(unsigned oinst, unsigned oport, unsigned iinst, unsigned iport) {
	unsigned key[2] = { oinst, oport },
		 value[2] = { iinst, iport };

	/* fprintf(stderr, "link %u %u -> %u %u\n", oinst, oport, iinst, iport); */
	hash_put(link_hd, key, sizeof(key), value, sizeof(value));
	/* fprintf(stderr, "rlink %u -> %u\n", iinst, oinst); */
	hash_put(link_rhd, &iinst, sizeof(iinst), &oinst, sizeof(oinst));
	return 0;
}

int py_run(PyObject *module, char *name) {
	PyObject *func, *result;
	func = PyObject_GetAttrString(module, name);

	if (!func || !PyCallable_Check(func)) {
		PyErr_Print();
		fprintf(stderr, "Failed to find Python function '%s'\n", name);
		Py_XDECREF(func);
		return 1;
	}

	result = PyObject_CallObject(func, NULL);
	Py_DECREF(func);

	if (!result) {
		PyErr_Print();
		fprintf(stderr, "Python function '%s' failed\n", name);
		exit(1);
	}
}

void pyflow_dlopen() {
	void *sl = dlopen("./pyflow.so", RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
	struct flow *iflow = dlsym(sl, "flow");
	if (iflow)
		*iflow = flow;
}

unsigned flow_node(char *name) {
	static char buf[BUFSIZ];
	struct flow_class flow_class = { .flags = 0, .data = NULL };
	unsigned ret;

	sprintf(buf, "./%s.so", name);

	if (!shash_get(class_rhd, &ret, name))
		return ret;

	flow_class.data = dlopen(buf, RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);

	if (!flow_class.data) {
		PyObject *func, *module_name = PyUnicode_FromString(name);
		flow_class.flags = FF_PYTHON;
		flow_class.data = PyImport_Import(module_name);

		if (!flow_class.data) {
			PyErr_Print();
			fprintf(stderr, "Failed to load.\n");
			return -1;
		}

		Py_DECREF(module_name);
		Py_DECREF(flow_class.data);

		ret = lhash_new(class_hd, &flow_class);
		/* fprintf(stderr, "flow_node %s %u %p %d (py)\n", name, ret, flow_class.data, flow_class.flags); */
		suhash_put(class_rhd, name, ret);

		has_init = !py_run(flow_class.data, "flow_init");
		return ret;
	}

	ret = lhash_new(class_hd, &flow_class);
	suhash_put(class_rhd, name, ret);

	struct flow *iflow = dlsym(flow_class.data, "flow");
	if (iflow)
		*iflow = flow;

	flow_run_t *flow_init = dlsym(flow_class.data, "flow_init");
	if (flow_init)
		(*flow_init)();
	has_init = !!flow_init;

	return ret;
}

unsigned flow_inst(unsigned class_id) {
	struct flow_class flow_class;
	if (uhash_get(class_hd, &flow_class, class_id))
		return -1;
	unsigned ret = lhash_new(inst_hd, &class_id);
	/* fprintf(stderr, "flow_inst %u %u\n", class_id, ret); */
	return ret;
}

void *flow_tran_thread(void *arg) {
	unsigned iclass;
	struct flow_class flow_class;

	uhash_get(inst_hd, &iclass, *((unsigned *) arg));
	uhash_get(class_hd, &flow_class, iclass);
	stack[stack_n] = *((unsigned *) arg);
	stack_n ++; // FIXME should be on node instantiation
	me = *((unsigned *) arg);
	if (flow_class.flags & FF_PYTHON)
		py_run(flow_class.data, "flow_run");
	else {
		flow_run_t *flow_run = dlsym(flow_class.data, "flow_run");
		if (flow_run)
			(*flow_run)();
	}

	free(arg);
}

void flow_tran_next(unsigned oinst);

unsigned flow_tran(unsigned iinst) {
	struct flow_class flow_class;
	unsigned iclass;

	uhash_get(inst_hd, &iclass, iinst);
	uhash_get(class_hd, &flow_class, iclass);

	unsigned *arg = malloc(sizeof(unsigned)), key[MAX_STACK], id, n = 1;
	*arg = iinst;

	pthread_t thread;
	pthread_create(&thread, NULL, flow_tran_thread, arg);
	pthread_detach(thread);

	flow_tran_next(iinst);
}


void flow_tran_next(unsigned oinst) {
	struct hash_cursor c = hash_iter(tran_hd, &oinst, sizeof(oinst));
	unsigned iinst, iclass;

	while (hash_next(&oinst, &iinst, &c))
		flow_tran(iinst);
}

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
	else {
		flow_tran(-1);
	}
}

int main(int argc, char *argv[]) {
	/* ndc_config.flags = NDC_DETACH; */
	ndc_config.port = 4201;

	ndc_init(&ndc_config);

	class_hd = lhash_init(sizeof(struct flow_class));
	class_rhd = hash_init();
	inst_hd = lhash_init(sizeof(unsigned));
	link_hd = hash_init();
	tran_hd = hash_init();
	tran_rhd = hash_init();
	data_hd = hash_init();
	Py_Initialize();
	PyRun_SimpleString("import sys; sys.path.insert(0, '')");

	flow.read = flow_read;
	flow.write = flow_write;
	flow.link_tran = flow_link_tran;
	flow.link = flow_link;
	flow.node = flow_node;
	flow.inst = flow_inst;
	flow.tran = flow_tran;
	pyflow_dlopen();

	ndc_main();
	return 0;
}
