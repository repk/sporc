CC=gcc
CFLAGS= -g -O0 -W -Wall
LDFLAGS=
LDSCRIPT=script.ld

SRC= \
	main.c
BIN=sporc
BUILDDIR=build
OBJS=$(SRC:%.c=$(BUILDDIR)/%.o)

all: sporc

$(BIN): $(OBJS) $(SCRIPT)
	$(CC) -T $(LDSCRIPT) -o $@ $(OBJS) $(LDFLAGS)

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -MMD -c $(CFLAGS) -o $@ $<

-include $(OBJS:.o=.d)

.PHONY: clean

clean:
	rm -f $(OBJS)
	rm -f $(OBJS:.o=.d)
	rm -rf $(BUILDDIR)
	rm -f $(BIN)
