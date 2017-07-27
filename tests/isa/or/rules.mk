ifeq ($(TESTS),1)
	TARGET = t-or
	CROSSTARGET = or.bin
endif

t-or-OUTDIR = tests/isa
t-or-CSRC = main.c
t-or-DEPS = b-test-utils

or.bin-OUTDIR = tests/binaries/isa
or.bin-ASRC = or.s
or.bin-DEPS = b-test-tsparc-utils
