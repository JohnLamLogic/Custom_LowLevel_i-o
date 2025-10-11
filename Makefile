
SHELL=	/bin/sh

AR=	/usr/local/bin/ar
RANLIB=	/usr/local/bin/ranlib
CC=	/usr/local/bin/gcc
CPPFLAGS=	-MMD -MP
CFLAGS+=	-I.

SRCS=	mio.c
OBJS:=	${SRCS:%.c=%.o}
DEPS:=	${OBJS:%.o=%.d}

all: mio.a mtest

mio.a: ${OBJS}
	${AR} -rcs ${.TARGET} ${.ALLSRC}
	${RANLIB} ${.TARGET}

mtest: main.o mio.a
	${CC} ${CFLAGS} ${.ALLSRC} -o ${.TARGET}

.SUFFIXES: .c .o
.c.o:
	${CC} -c ${CPPFLAGS} ${CFLAGS} ${.IMPSRC} -o ${.TARGET}

.PHONY: all clean
clean: 
	rm -f *.o *.d *~ *core mio.a mtest out1.txt out2.txt

-include ${DEPS}


