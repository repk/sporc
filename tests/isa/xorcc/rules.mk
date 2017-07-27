ifeq ($(TESTS),1)
	TARGET = t-xorcc
	CROSSTARGET = xorcc.bin
endif

t-xorcc-OUTDIR = tests/isa
t-xorcc-CSRC = main.c
t-xorcc-DEPS = b-test-utils

xorcc.bin-OUTDIR = tests/binaries/isa
xorcc.bin-ASRC = xorcc.s
xorcc.bin-DEPS = b-test-tsparc-utils
