ifeq ($(TESTS),1)
	TARGET = t-or
	CROSSTARGET = or.bin
endif

t-or-OUTDIR = tests
t-or-CSRC = main.c
t-or-DEPS = b-test-utils

or.bin-OUTDIR = tests/binaries
or.bin-ASRC = or.s
or.bin-DEPS = b-test-tsparc-utils
