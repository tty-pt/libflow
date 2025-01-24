npm-lib := @tty-pt/qhash @tty-pt/ndc
npm-root != npm root
npm-root-dir != dirname ${npm-root}
libdir := /usr/local/lib ${pwd} ${npm-lib:%=${npm-root}/%} \
	  ${npm-lib:%=${npm-root-dir}/../../%}
LDFLAGS	+= -lndc -lqhash -ldb ${libdir:%=-L%} ${libdir:%=-Wl,-rpath,%}

all: libflow.a number_gen.so number_print.so flow1.so flowd

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

flowd: flowd.c
	        ${CC} -g -o $@ flowd.c ${LDFLAGS} -I./include -I./node_modules/@tty-pt/ndc/include
