CC ?= gcc
PKGCONFIG = $(shell which pkg-config)
CFLAGS = $(shell $(PKGCONFIG) --cflags gtk+-3.0 gio-2.0 gio-unix-2.0 x11 libpulse libnm)
LIBS = $(shell $(PKGCONFIG) --libs gtk+-3.0 gio-2.0 gio-unix-2.0 x11 libpulse libnm) -lpulse-mainloop-glib
PREFIX = /usr/local

SRC = sgbar.c metric.c tags.c x.c volume.c battery.c brightness.c network.c clock.c container.c seperator.c

OBJS = $(SRC:.c=.o)

all: sgbar

%.o: %.c
	$(CC) -c -o $(@F) $(CFLAGS) $<

sgbar: $(OBJS)
	$(CC) -o $(@F) $(OBJS) $(LIBS)

debug: CFLAGS += -DDEBUG -g
debug: sgbar

clean:
	rm -f $(OBJS)
	rm -f sgbar

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f sgbar ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/sgbar
	mkdir -p ${DESTDIR}${PREFIX}/share/sgbar
	cp -f style.css ${DESTDIR}${PREFIX}/share/sgbar
