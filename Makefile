src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)
bin = tinywebd

CFLAGS = -pedantic -Wall -g

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.d: %.c
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:.d=.o) >$@

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
