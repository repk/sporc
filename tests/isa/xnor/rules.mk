ifeq ($(TESTS),1)
	TARGET = t-xnor
	CROSSTARGET = xnor.bin
endif

t-xnor-OUTDIR = tests/isa
t-xnor-CSRC = main.c
t-xnor-DEPS = b-test-utils

xnor.bin-OUTDIR = tests/binaries/isa
xnor.bin-ASRC = xnor.s
xnor.bin-DEPS = b-test-tsparc-utils
