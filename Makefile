prefix ?= /usr

all:
	gcc dump-vtable.c -o vtable-dumper -ldl -lelf -lstdc++ -Wall

install: all
	mkdir -p $(prefix)/bin/
	install vtable-dumper $(prefix)/bin/

uninstall:
	rm -f $(prefix)/bin/vtable-dumper

clean:
	rm -f vtable-dumper