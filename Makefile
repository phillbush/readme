PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

CPPFLAGS = -D_POSIX_C_SOURCE=200809L
CFLAGS = -Wall -Wextra ${CPPFLAGS}

PROG = readme
OBJS = ${PROG:=.o}

all: ${PROG}

${PROG}: ${OBJS}
	${CC} -o $@ ${OBJS} ${LDFLAGS}

.c.o:
	${CC} ${CFLAGS} -c $<

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	install -m 755 ${PROG} ${DESTDIR}${PREFIX}/bin/${PROG}
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	install -m 644 ${PROG}.1 ${DESTDIR}${MANPREFIX}/man1/${PROG}.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/${PROG}
	rm -f ${DESTDIR}${MANPREFIX}/man1/${PROG}.1

clean:
	-rm ${OBJS} ${PROG}

.PHONY: all clean install uninstall
