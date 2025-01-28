npm-lib := @tty-pt/qhash @tty-pt/ndc
npm-root != npm root
npm-root-dir != dirname ${npm-root}
libdir := /usr/lib/x86_64-linux-gnu /usr/local/lib ${pwd} ${npm-lib:%=${npm-root}/%} \
	  ${npm-lib:%=${npm-root-dir}/../../%}

LDFLAGS	:=  -Wl,--no-as-needed -lpython3.10 -lndc -lqhash -ldb ${libdir:%=-L%} ${libdir:%=-Wl,-rpath,%}

all: libflow.a number_gen.so number_print.so flow1.so pyflow.so flowd docs/man/man3/flow.h.3

number_gen.so: number_gen.c
	        ${CC} -shared -g -o $@ -fPIC number_gen.c -lflow -L. -I./include

number_print.so: number_print.c
	        ${CC} -shared -g -o $@ -fPIC number_print.c -lflow -L. -I./include

flow1.so: flow1.c
	        ${CC} -shared -g -o $@ -fPIC flow1.c -lflow -L. -I./include

libflow.o: libflow.c
	        ${CC} -g -c -o $@ -fPIC ${CFLAGS} libflow.c

libflow.a: libflow.o
	        ar rcs libflow.a libflow.o

libflow.so: libflow.a
	gcc -shared -o libflow.so -Wl,--whole-archive libflow.a -Wl,--no-whole-archive

pyflow.so: pyflow.py
	python3 pyflow.py

flowd: flowd.c
	        ${CC} -g -o $@ flowd.c ${LDFLAGS} -I./include -I./node_modules/@tty-pt/ndc/include -I/usr/include/python3.10

docs/man/man3/flow.h.3: include/flow.h
	doxygen Doxyfile
