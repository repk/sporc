ifeq ($(TESTS),1)
	TARGET = t-tcc
	CROSSTARGET = tcc.bin
endif

t-tcc-OUTDIR = tests/isa
t-tcc-CSRC = main.c
t-tcc-DEPS = b-test-utils

tcc.bin-OUTDIR = tests/binaries/isa
tcc.bin-ASRC = tcc.s
tcc.bin-DEPS = b-test-tsparc-utils
