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
unsigned class_hd, class_rhd, inst_hd, link_hd,
	 tran_hd, data_hd, stack_hd;
unsigned me;
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

void stack_debug() {
	unsigned key[MAX_STACK], n = 0;
	uhash_get(stack_hd, key, me);
	fprintf(stderr, "stack_debug %u /", me);
	while (key[n] != (unsigned) -1) {
		fprintf(stderr, " %u", key[n]);
		n++;
	}
	fprintf(stderr, "\n");
}

int flow_write(unsigned oport, void *data, size_t size) {
	unsigned key[MAX_STACK], id, n;
	key[0] = oport;
	uhash_get(stack_hd, key + 1, me);
	n = 1;
	while (key[n] != (unsigned) -1)
		n++;
	/* stack_debug(); */
	hash_put(data_hd, key, sizeof(id) * n, data, size);
	return 0;
}

int flow_read(unsigned iport, void *target, size_t size) {
	unsigned key[MAX_STACK], id, n = 0;
	uhash_get(stack_hd, key, me);
	key[0] = iport;
	n = 1;
	while (key[n] != (unsigned) -1)
		n++;
	/* stack_debug(); */
	hash_get(data_hd, target, key, sizeof(id) * n);
	return 0;
}

int flow_link_tran(unsigned oinst, unsigned iinst) {
	uhash_put(tran_hd, oinst, &iinst, sizeof(iinst));
	return 0;
}

int flow_link(unsigned oinst, unsigned oport, unsigned iinst, unsigned iport) {
	unsigned key[2] = { oinst, oport },
		 value[2] = { iinst, iport };

	hash_put(link_hd, key, sizeof(key), value, sizeof(value));
	return 0;
}

void py_run(PyObject *module, char *name) {
	PyObject *func, *result;
	func = PyObject_GetAttrString(module, name);

	if (!func || !PyCallable_Check(func)) {
		PyErr_Print();
		fprintf(stderr, "Failed to find Python function '%s'\n", name);
		Py_XDECREF(func);
		exit(1);
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

		py_run(flow_class.data, "flow_init");
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

	return ret;
}

unsigned flow_inst(unsigned class_id) {
	struct flow_class flow_class;
	if (uhash_get(class_hd, &flow_class, class_id))
		return -1;
	return lhash_new(inst_hd, &class_id);
}

void *flow_tran_thread(void *arg) {
	unsigned iclass;
	struct flow_class flow_class;

	uhash_get(inst_hd, &iclass, *((unsigned *) arg));
	uhash_get(class_hd, &flow_class, iclass);

	me = *((unsigned *) arg);
	if (flow_class.flags & FF_PYTHON)
		py_run(flow_class.data, "flow_run");
	else {
		flow_run_t *flow_run = dlsym(flow_class.data, "flow_run");
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

	if (uhash_get(stack_hd, key + 1, me))
		key[1] = -1;
	key[0] = iinst;
	n = 1;
	while (key[n] != (unsigned) -1)
		n++;
	key[n++] = -1;
	uhash_put(stack_hd, iinst, key, sizeof(id) * n);

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
	else
		flow_tran(me);
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
	data_hd = hash_init();
	stack_hd = hash_init();
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
