CC ?= gcc
PKGCONFIG = $(shell which pkg-config)
CFLAGS = $(shell $(PKGCONFIG) --cflags gtk+-3.0 gio-2.0 gio-unix-2.0 x11 libpulse)
LIBS = $(shell $(PKGCONFIG) --libs gtk+-3.0 gio-2.0 gio-unix-2.0 x11 libpulse) -lpulse-mainloop-glib

SRC = sgbar.c config.c widgets.c x.c volume.c battery.c brightness.c

OBJS = $(SRC:.c=.o)

all: sgbar

%.o: %.c
	$(CC) -c -o $(@F) $(CFLAGS) $<

sgbar: $(OBJS)
	$(CC) -o $(@F) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS)
	rm -f sgbar
