ifeq ($(TESTS),1)
	TARGET = t-sethi
	CROSSTARGET = sethi.bin
endif

t-sethi-OUTDIR = tests/isa
t-sethi-CSRC = main.c
t-sethi-DEPS = b-test-utils

sethi.bin-OUTDIR = tests/binaries/isa
sethi.bin-ASRC = sethi.s
sethi.bin-DEPS = b-test-tsparc-utils
