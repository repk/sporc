ifeq ($(TESTS),1)
	TARGET = t-ldsb
	CROSSTARGET = ldsb.bin
endif

t-ldsb-OUTDIR = tests
t-ldsb-CSRC = main.c
t-ldsb-DEPS = b-test-utils

ldsb.bin-OUTDIR = tests/binaries
ldsb.bin-ASRC = ldsb.s
ldsb.bin-DEPS = b-test-tsparc-utils
