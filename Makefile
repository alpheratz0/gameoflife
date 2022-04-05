VERSION = 0.1.0
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
INCS = -I. -I/usr/include
LIBS = -lglut -lGL
CFLAGS = -std=c99 -pedantic -Wall -Os ${INCS} -DVERSION=\"${VERSION}\"
LDFLAGS = -s ${LIBS}
CC = cc

SRC = src/gameoflife.c \
	  src/debug.c \
	  src/window.c \
	  src/board.c

OBJ = ${SRC:.c=.o}

all: gameoflife

${OBJ}: src/keys.h \
		src/debug.h \
		src/window.h \
		src/numdef.h \
		src/buttons.h \
		src/board.h

gameoflife: ${OBJ}
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@rm -f gameoflife gameoflife-${VERSION}.tar.gz ${OBJ}

install: all
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f gameoflife ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/gameoflife
	@mkdir -p ${DESTDIR}${MANPREFIX}/man6
	@cp -f man/gameoflife.6 ${DESTDIR}${MANPREFIX}/man6/gameoflife.6
	@chmod 644 ${DESTDIR}${MANPREFIX}/man6/gameoflife.6

dist: clean
	@mkdir -p gameoflife-${VERSION}
	@cp -R LICENSE Makefile README man src gameoflife-${VERSION}
	@tar -cf gameoflife-${VERSION}.tar gameoflife-${VERSION}
	@gzip gameoflife-${VERSION}.tar
	@rm -rf gameoflife-${VERSION}

uninstall:
	@rm -f ${DESTDIR}${PREFIX}/bin/gameoflife
	@rm -f ${DESTDIR}${MANPREFIX}/man6/gameoflife.6

.PHONY: all clean install uninstall dist
