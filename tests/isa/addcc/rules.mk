ifeq ($(TESTS),1)
	TARGET = t-addcc
	CROSSTARGET = addcc.bin
endif

t-addcc-OUTDIR = tests/isa
t-addcc-CSRC = main.c
t-addcc-DEPS = b-test-utils

addcc.bin-OUTDIR = tests/binaries/isa
addcc.bin-ASRC = addcc.s
addcc.bin-DEPS = b-test-tsparc-utils
