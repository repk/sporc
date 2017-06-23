CC = gcc
CFLAGS = -g -O0 -W -Wall
LDFLAGS =
LDSCRIPT = script.ld
MAKE = make

INC = include
SRC = \
	main.c \
	memory/memory.c \
	memory/file.c

BIN = sporc
BUILDDIR = build
OBJS = $(SRC:%.c=$(BUILDDIR)/%.o)
CFLAGS += $(INC:%=-I %)

all: $(BIN)

$(BIN): $(OBJS) $(LDSCRIPT)
	$(CC) -T $(LDSCRIPT) -o $@ $(OBJS) $(LDFLAGS)

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -MMD -c $(CFLAGS) -I include -o $@ $<

-include $(OBJS:.o=.d)

.PHONY: clean example-clean example

example:
	$(MAKE) -C example

example-clean:
	$(MAKE) -C example clean

clean:
	rm -f $(OBJS)
	rm -f $(OBJS:.o=.d)
	rm -rf $(BUILDDIR)
	rm -f $(BIN)
