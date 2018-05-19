CC = gcc
CFLAGS += -W -Wall
LDFLAGS =

CROSS ?= sparc-linux-uclibc-
CROSSCC = $(CROSS)gcc
CROSSLDFLAGS = -nostdlib
CROSSCFLAGS += -W -Wall
CROSSOBJCOPY = $(CROSS)objcopy

BUILDDIR = build/
OUTDIR = out/
