ifeq ($(TESTS),1)
	TARGET = t-ldsb
	CROSSTARGET = ldsb.bin
endif

t-ldsb-OUTDIR = tests/isa
t-ldsb-CSRC = main.c
t-ldsb-DEPS = b-test-utils

ldsb.bin-OUTDIR = tests/binaries/isa
ldsb.bin-ASRC = ldsb.s
ldsb.bin-DEPS = b-test-tsparc-utils
