prefix ?= /usr

.PHONY: all install uninstall clean
all: vtable-dumper

install: vtable-dumper
	mkdir -p $(DESTDIR)$(prefix)/bin/
	install vtable-dumper $(DESTDIR)$(prefix)/bin/

vtable-dumper: dump-vtable.c dump-vtable.h
	$(CXX) $(CFLAGS) $(LDFLAGS) -o vtable-dumper dump-vtable.c -ldl -lelf

uninstall:
	rm -f $(DESTDIR)$(prefix)/bin/vtable-dumper

clean:
	rm -f vtable-dumper
