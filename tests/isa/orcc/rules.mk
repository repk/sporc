ifeq ($(TESTS),1)
	TARGET = t-orcc
	CROSSTARGET = orcc.bin
endif

t-orcc-OUTDIR = tests/isa
t-orcc-CSRC = main.c
t-orcc-DEPS = b-test-utils

orcc.bin-OUTDIR = tests/binaries/isa
orcc.bin-ASRC = orcc.s
orcc.bin-DEPS = b-test-tsparc-utils
