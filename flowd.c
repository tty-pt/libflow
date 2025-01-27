#include "./include/flow.h"
#include "./include/papi.h"
#include <dlfcn.h>
#include <qhash.h>
#include <ndc.h>
#include <stdlib.h>
#include <Python.h>

enum FLOW_FLAG {
	FF_PYTHON = 1,
};

struct flow_class {
	int flags;
	void *data;
};

struct ndc_config ndc_config = { .flags = 0 };
unsigned class_hd, class_rhd, inst_hd, link_hd, tran_hd, data_hd;
struct flow flow;
unsigned me, parent;
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
		Py_DECREF(module_name);

		if (!flow_class.data) {
			PyErr_Print();
			fprintf(stderr, "Failed to load.\n");
			exit(1);
		}

		Py_DECREF(flow_class.data);

		ret = lhash_new(class_hd, &flow_class);
		/* fprintf(stderr, "flow_node %s %u %p %d (py)\n", name, ret, flow_class.data, flow_class.flags); */
		suhash_put(class_rhd, name, ret);

		py_run(flow_class.data, "flow_init");
		return ret;
	}

	ret = lhash_new(class_hd, &flow_class);
	/* fprintf(stderr, "flow_node %s %u %p %d\n", name, ret, flow_class.data, flow_class.flags); */
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
	unsigned ret = lhash_new(inst_hd, &class_id);
	/* fprintf(stderr, "flow_inst %u %u\n", class_id, ret); */
	return ret;
}

void flow_tran_next(unsigned oinst);

unsigned flow_tran(unsigned iinst) {
	struct flow_class flow_class;
	unsigned iclass;
	/* fprintf(stderr, "flow_tran -> %u\n", iinst); */

	uhash_get(inst_hd, &iclass, iinst);
	uhash_get(class_hd, &flow_class, iclass);
	me = iinst;

	if (flow_class.flags & FF_PYTHON)
		py_run(flow_class.data, "flow_run");
	else {
		flow_run_t *flow_run = dlsym(flow_class.data, "flow_run");
		(*flow_run)();
	}

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

char *ndc_auth_check(int fd) { return NULL; }
void ndc_update(unsigned long long dt) {}
void ndc_command(int fd, int argc, char *argv[]) {}
void ndc_vim(int fd, int argc, char *argv[]) {}
int ndc_connect(int fd) {}
void ndc_disconnect(int fd) {}

void do_load(int fd, int argc, char *argv[]) {
	unsigned me_class = flow_node(argv[1]);
	ndc_writef(fd, "%u\n", me_class);
}

void do_run(int fd, int argc, char *argv[]) {
	unsigned me_class = strtoull(argv[1], NULL, 10);
	parent = me = flow_inst(me_class);
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
	Py_Initialize();

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
