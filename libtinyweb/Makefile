PREFIX ?= /usr/local

src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)
name = tinyweb

CFLAGS = -pedantic -Wall -g $(pic)

sys = $(shell uname -s)

so_major = 0
so_minor = 1
alib = lib$(name).a

ifeq ($(sys), Darwin)
	solib = lib$(name).dylib
	shared = -dynamiclib
else
	ldname = lib$(name).so
	soname = lib$(name).so.$(so_major)
	solib = lib$(name).so.$(so_major).$(so_minor)
	shared = -shared -Wl,-soname=$(soname)
	pic = -fPIC
endif

.PHONY: all
all: $(alib) $(solib)

$(solib): $(obj)
	$(CC) -o $@ $(shared) $(obj) $(LDFLAGS)
	[ -n "$(soname)" ] && \
		rm -f $(soname) $(ldname) && \
		ln -s $(solib) $(soname) && \
		ln -s $(soname) $(ldname) || true

$(alib): $(obj)
	$(AR) rcs $@ $(obj)

-include $(dep)

%.d: %.c
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -f $(obj) $(alib) $(solib)

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: install
install: $(alib) $(solib)
	mkdir -p $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(PREFIX)/include
	cp src/tinyweb.h $(DESTDIR)$(PREFIX)/include/tinyweb.h
	cp $(alib) $(DESTDIR)$(PREFIX)/lib/$(alib)
	cp $(solib) $(DESTDIR)$(PREFIX)/lib/$(solib)
	[ -n "$(soname)" ] && \
		cd $(DESTDIR)$(PREFIX)/lib && \
		rm -f $(soname) $(ldname) && \
		ln -s $(solib) $(soname) && \
		ln -s $(soname) $(ldname) || \
		true

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/include/tinyweb.h
	rm -f $(DESTDIR)$(PREFIX)/lib/$(alib)
	rm -f $(DESTDIR)$(PREFIX)/lib/$(solib)
	[ -n "$(soname)" ] && \
		rm -f $(DESTDIR)$(PREFIX)/lib/$(soname) && \
		rm -f $(DESTDIR)$(PREFIX)/lib/$(ldname) || \
		true

