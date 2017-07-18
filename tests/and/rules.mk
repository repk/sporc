ifeq ($(TESTS),1)
	TARGET = t-and
	CROSSTARGET = and.bin
endif

t-and-OUTDIR = tests
t-and-CSRC = main.c
t-and-DEPS = b-test-utils

and.bin-OUTDIR = tests/binaries
and.bin-ASRC = and.s
and.bin-DEPS = b-test-tsparc-utils
