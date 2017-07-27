ifeq ($(TESTS),1)
	TARGET = t-xnorcc
	CROSSTARGET = xnorcc.bin
endif

t-xnorcc-OUTDIR = tests/isa
t-xnorcc-CSRC = main.c
t-xnorcc-DEPS = b-test-utils

xnorcc.bin-OUTDIR = tests/binaries/isa
xnorcc.bin-ASRC = xnorcc.s
xnorcc.bin-DEPS = b-test-tsparc-utils
