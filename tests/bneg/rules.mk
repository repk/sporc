ifeq ($(TESTS),1)
	TARGET = t-bneg
	CROSSTARGET = bneg.bin
endif

t-bneg-OUTDIR = tests
t-bneg-CSRC = main.c
t-bneg-DEPS = b-test-utils

bneg.bin-OUTDIR = tests/binaries
bneg.bin-ASRC = bneg.s
bneg.bin-DEPS = b-test-tsparc-utils
