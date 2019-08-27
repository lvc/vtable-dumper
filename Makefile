prefix ?= /usr

.PHONY: all install uninstall clean
all: vtable-dumper

install: vtable-dumper
	mkdir -p $(DESTDIR)$(prefix)/bin/
	install vtable-dumper $(DESTDIR)$(prefix)/bin/

OS=$(shell uname -s)
ifeq ($(OS), Linux)
LIBDL=-ldl
else  ifeq ($(OS), FreeBSD)
LIBDL=
else
LIBDL=UNKNOWN
endif

vtable-dumper: dump-vtable.c dump-vtable.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o vtable-dumper dump-vtable.c \
		$(LIBDL) -lelf -lstdc++

uninstall:
	rm -f $(DESTDIR)$(prefix)/bin/vtable-dumper

clean:
	rm -f vtable-dumper
