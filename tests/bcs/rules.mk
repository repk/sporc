ifeq ($(TESTS),1)
	TARGET = t-bcs
	CROSSTARGET = bcs.bin
endif

t-bcs-OUTDIR = tests
t-bcs-CSRC = main.c
t-bcs-DEPS = b-test-utils

bcs.bin-OUTDIR = tests/binaries
bcs.bin-ASRC = bcs.s
bcs.bin-DEPS = b-test-tsparc-utils
