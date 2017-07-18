ifeq ($(TESTS),1)
	TARGET = t-xnorcc
	CROSSTARGET = xnorcc.bin
endif

t-xnorcc-OUTDIR = tests
t-xnorcc-CSRC = main.c
t-xnorcc-DEPS = b-test-utils

xnorcc.bin-OUTDIR = tests/binaries
xnorcc.bin-ASRC = xnorcc.s
xnorcc.bin-DEPS = b-test-tsparc-utils
