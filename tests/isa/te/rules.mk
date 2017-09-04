ifeq ($(TESTS),1)
	TARGET = t-te
	CROSSTARGET = te.bin
endif

t-te-OUTDIR = tests/isa
t-te-CSRC = main.c
t-te-DEPS = b-test-utils

te.bin-OUTDIR = tests/binaries/isa
te.bin-ASRC = te.s
te.bin-DEPS = b-test-tsparc-utils
