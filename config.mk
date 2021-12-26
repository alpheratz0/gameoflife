VERSION = 1.0
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
INCS = -I. -I/usr/include
LIBS = -lglut -lGL
CFLAGS = -std=c99 -pedantic -Wall -Os ${INCS}
LDFLAGS = -s ${LIBS}
CC = cc
