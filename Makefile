npm-lib := @tty-pt/qhash
npm-root != npm root
npm-root-dir != dirname ${npm-root}
libdir := /usr/local/lib ${pwd} ${npm-lib:%=${npm-root}/%} \
	  ${npm-lib:%=${npm-root-dir}/../../%}
LDFLAGS	+= -lqhash -ldb ${libdir:%=-L%} ${libdir:%=-Wl,-rpath,%}

all: libmov.a number_gen.so number_print.so flow1.so mov

number_gen.so: number_gen.c
	        ${CC} -shared -g -o $@ -fPIC number_gen.c -lmov -L. -I./include

number_print.so: number_print.c
	        ${CC} -shared -g -o $@ -fPIC number_print.c -lmov -L. -I./include

flow1.so: flow1.c
	        ${CC} -shared -g -o $@ -fPIC flow1.c -lmov -L. -I./include

libmov.o: libmov.c
	        ${CC} -g -c -o $@ -fPIC ${CFLAGS} libmov.c

libmov.a: libmov.o
	        ar rcs libmov.a libmov.o

mov: mov.c
	        ${CC} -g -o $@ mov.c ${LDFLAGS} -I./include
