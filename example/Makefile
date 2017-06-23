CROSS = sparc-linux-uclibc-
CC = $(CROSS)gcc
LD = $(CROSS)ld
CFLAGS = -g -O0 -W -Wall
LDFLAGS =
LDSCRIPT = script.ld
OBJCOPY = $(CROSS)objcopy

ASRC = \
	example.s

BIN = example.bin
ELF = $(BUILDDIR)/example.elf
BUILDDIR = build
AOBJS = $(ASRC:%.s=$(BUILDDIR)/%.s.o)
COBJS = $(CSRC:%.c=$(BUILDDIR)/%.c.o)
CFLAGS += -nostdinc

all: $(BIN)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(ELF): $(COBJS) $(AOBJS) $(LDSCRIPT)
	$(LD) -T $(LDSCRIPT) -o $@ $(COBJS) $(AOBJS) $(LDFLAGS)

$(BUILDDIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -MMD -c $(CFLAGS) -o $@ $<

$(BUILDDIR)/%.s.o: %.s
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) -o $@ $<

-include $(COBJS:.o=.d)

.PHONY: clean

clean:
	rm -f $(AOBJS)
	rm -f $(COBJS)
	rm -f $(COBJS:.o=.d)
	rm -rf $(BUILDDIR)
	rm -f $(BIN)
