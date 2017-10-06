ifeq ($(TESTS),1)
	TARGET = t-tsubcc
	CROSSTARGET = tsubcc.bin
endif

t-tsubcc-OUTDIR = tests/isa
t-tsubcc-CSRC = main.c
t-tsubcc-DEPS = b-test-utils

tsubcc.bin-OUTDIR = tests/binaries/isa
tsubcc.bin-ASRC = tsubcc.s
tsubcc.bin-DEPS = b-test-tsparc-utils
