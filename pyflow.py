from cffi import FFI
import cffi
import os

ffi = FFI()
print(cffi.__version__)

ffi.dlopen(None)

ffi.cdef("""
    int flow_init();
    int flow_run();
    int flow_write(unsigned port, void *data, size_t len);
    int flow_read(unsigned port, void *data, size_t len);
    int flow_link_tran(unsigned oinst, unsigned iinst);
    int flow_link(unsigned oinst, unsigned oport, unsigned iinst, unsigned oport);
    unsigned flow_node(char* name);
    unsigned flow_inst(unsigned class_id);
    unsigned flow_tran(unsigned instance_id);
""")

ffi.set_source(
    "pyflow",
    """
    #include <Python.h>
    #include <stdio.h>
    #include <flow.h>

    static void initialize_python() {
        if (!Py_IsInitialized()) {
            Py_Initialize();
        }
    }

    static PyObject* call_python_function(const char* function_name) {
        PyObject *module_name, *module, *func, *result;

        // Import the Python module (assumes the script is called "pyfun.py")
        module_name = PyUnicode_FromString("pyfun");
        module = PyImport_Import(module_name);
        Py_DECREF(module_name);

        if (!module) {
            PyErr_Print();
            fprintf(stderr, "Failed to load Python module\\n");
            return NULL;
        }

        // Get the function from the module
        func = PyObject_GetAttrString(module, function_name);
        Py_DECREF(module);

        if (!func || !PyCallable_Check(func)) {
            PyErr_Print();
            fprintf(stderr, "Failed to find Python function '%s'\\n", function_name);
            Py_XDECREF(func);
            return NULL;
        }

        // Call the function
        result = PyObject_CallObject(func, NULL);
        Py_DECREF(func);

        if (!result) {
            PyErr_Print();
            fprintf(stderr, "Python function '%s' failed\\n", function_name);
            return NULL;
        }

        return result;
    }

    int flow_init() {
        initialize_python();
        PyObject *result = call_python_function("flow_init");
        if (!result) {
            return -1;
        }
        int ret = (int)PyLong_AsLong(result);
        Py_DECREF(result);
        return ret;
    }

    int flow_run() {
        initialize_python();
        PyObject *result = call_python_function("flow_run");
        if (!result) {
            return -1;
        }
        int ret = (int)PyLong_AsLong(result);
        Py_DECREF(result);
        return ret;
    }
    """,
	libraries=["flow", "python3.10"],
    library_dirs=[".", "/usr/lib", "/usr/lib/x86_64-linux-gnu", "/usr/local/lib"],
    include_dirs=["/usr/include/python3.10", "./include"],
    extra_link_args=[
		"-Wl,-rpath,/home/quirinpa/libmov",
        "-Wl,-rpath,/usr/lib/x86_64-linux-gnu",
        "-shared",
    ],

if __name__ == "__main__":
    ffi.compile(target="pyflow.so", verbose=True)
