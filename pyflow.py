from cffi import FFI
import cffi
import os

ffi = FFI()
print(cffi.__version__)

ffi.dlopen(None)

ffi.cdef("""
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
    #include <flow.h>
    """,
	libraries=["flow", "python3.10"],
    library_dirs=[".", "/usr/lib", "/usr/lib/x86_64-linux-gnu", "/usr/local/lib"],
    include_dirs=["/usr/include/python3.10", "./include"],
    extra_link_args=[
		"-Wl,-rpath,/home/quirinpa/libmov",
        "-Wl,-rpath,/usr/lib/x86_64-linux-gnu",
        "-shared",
    ],
    )

if __name__ == "__main__":
    ffi.compile(target="pyflow.so", verbose=True)
