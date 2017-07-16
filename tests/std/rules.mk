ifeq ($(TESTS),1)
	TARGET = t-std
	CROSSTARGET = std.bin
endif

t-std-OUTDIR = tests
t-std-CSRC = main.c
t-std-DEPS = b-test-utils

std.bin-OUTDIR = tests/binaries
std.bin-ASRC = std.s
std.bin-DEPS = b-test-tsparc-utils
