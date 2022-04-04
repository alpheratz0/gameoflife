PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
INCS = -I. -I/usr/include
LIBS = -lglut -lGL
CFLAGS = -std=c99 -pedantic -Wall -Os ${INCS}
LDFLAGS = -s ${LIBS}
CC = cc

SRC = frame.c gameoflife.c
OBJ = ${SRC:.c=.o}

all: gameoflife

${OBJ}: frame.h keyconfig.h

gameoflife: ${OBJ}
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@rm -f gameoflife ${OBJ}

install: all
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f gameoflife ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/gameoflife
	@mkdir -p ${DESTDIR}${MANPREFIX}/man6
	@cp -f gameoflife.6 ${DESTDIR}${MANPREFIX}/man6/gameoflife.6
	@chmod 644 ${DESTDIR}${MANPREFIX}/man6/gameoflife.6

uninstall:
	@rm -f ${DESTDIR}${PREFIX}/bin/gameoflife
	@rm -f ${DESTDIR}${MANPREFIX}/man6/gameoflife.6

.PHONY: all clean install uninstall
