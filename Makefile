src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)
bin = tinywebd
weblib = libtinyweb/libtinyweb.so

CFLAGS = -pedantic -Wall -g -Ilibtinyweb/src
LDFLAGS = -Llibtinyweb -Wl,-rpath=libtinyweb -ltinyweb

$(bin): $(obj) $(weblib)
	$(CC) -o $@ $(obj) $(LDFLAGS)

$(weblib):
	$(MAKE) -C libtinyweb

-include $(dep)

%.d: %.c
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: install
install: $(bin)
	mkdir -p $(PREFIX)/bin
	cp $(bin) $(DESTDIR)$(PREFIX)/bin/$(bin)

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(bin)
