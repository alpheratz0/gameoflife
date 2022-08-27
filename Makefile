.POSIX:
.PHONY: all clean install uninstall dist

include config.mk

all: gameoflife

gameoflife: gameoflife.o
	$(CC) $(LDFLAGS) -o gameoflife gameoflife.o $(LDLIBS)

clean:
	rm -f gameoflife gameoflife.o gameoflife-$(VERSION).tar.gz

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f gameoflife $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/gameoflife
	mkdir -p $(DESTDIR)$(MANPREFIX)/man6
	cp -f gameoflife.6 $(DESTDIR)$(MANPREFIX)/man6
	chmod 644 $(DESTDIR)$(MANPREFIX)/man6/gameoflife.6

dist: clean
	mkdir -p gameoflife-$(VERSION)
	cp -R COPYING config.mk Makefile README gameoflife.6 gameoflife.c gameoflife-$(VERSION)
	tar -cf gameoflife-$(VERSION).tar gameoflife-$(VERSION)
	gzip gameoflife-$(VERSION).tar
	rm -rf gameoflife-$(VERSION)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/gameoflife
	rm -f $(DESTDIR)$(MANPREFIX)/man6/gameoflife.6
