ifeq ($(TESTS),1)
	TARGET = t-mulscc
	CROSSTARGET = mulscc.bin
endif

t-mulscc-OUTDIR = tests/isa
t-mulscc-CSRC = main.c
t-mulscc-DEPS = b-test-utils

mulscc.bin-OUTDIR = tests/binaries/isa
mulscc.bin-ASRC = mulscc.s
mulscc.bin-DEPS = b-test-tsparc-utils
