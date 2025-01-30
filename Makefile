npm-lib := @tty-pt/qhash @tty-pt/ndc
npm-root != npm root
npm-root-dir != dirname ${npm-root}
npm-both := ${npm-root} ${npm-root-dir}/../..
npm-both-lib := ${npm-both:%=-L%} ${npm-both:%=-Wl,-rpath,%}

all: libflow.a number_gen.so number_print.so numbers_print.so flow1.so flow2.so pyflow.so libflowd.so flowd docs/man/man3/flow.h.3

.SUFFIXES: .so .o .c

.c.so:
	${CC} -shared -g -o $@ -fPIC ${@:%.so=%.c} -lflow -L. -I./include

libflow.o: libflow.c
	        ${CC} -g -c -o $@ -fPIC ${CFLAGS} libflow.c

libflowd.so: libflowd.c
	${CC} -shared -g -o $@ -fPIC ${@:%.so=%.c} -lpython3.10 -lqhash \
		${npm-both-lib:%=%/@tty-pt/qhash} \
		${npm-both:%=-I%/@tty-pt/qhash/include} \
		-I./include -I/usr/include/python3.10

libflow.a: libflow.o
	        ar rcs libflow.a libflow.o

pyflow.so: pyflow.py
	python3 pyflow.py

flowd: flowd.c
	${CC} -g -o $@ flowd.c -lflowd -lndc -L. -Wl,-rpath,. \
		${npm-both-lib:%=%/@tty-pt/ndc} \
		${npm-both:%=-I%/@tty-pt/ndc/include} \
		-I./include -I./node_modules/@tty-pt/ndc/include

docs/man/man3/flow.h.3: include/flow.h
	doxygen Doxyfile
