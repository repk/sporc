ifeq ($(TESTS),1)
	TARGET = t-addcc
	CROSSTARGET = addcc.bin
endif

t-addcc-OUTDIR = tests
t-addcc-CSRC = main.c
t-addcc-DEPS = b-test-utils

addcc.bin-OUTDIR = tests/binaries
addcc.bin-ASRC = addcc.s
addcc.bin-DEPS = b-test-tsparc-utils
