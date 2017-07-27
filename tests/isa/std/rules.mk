ifeq ($(TESTS),1)
	TARGET = t-std
	CROSSTARGET = std.bin
endif

t-std-OUTDIR = tests/isa
t-std-CSRC = main.c
t-std-DEPS = b-test-utils

std.bin-OUTDIR = tests/binaries/isa
std.bin-ASRC = std.s
std.bin-DEPS = b-test-tsparc-utils
