ifeq ($(TESTS),1)
	TARGET = t-tneg
	CROSSTARGET = tneg.bin
endif

t-tneg-OUTDIR = tests/isa
t-tneg-CSRC = main.c
t-tneg-DEPS = b-test-utils

tneg.bin-OUTDIR = tests/binaries/isa
tneg.bin-ASRC = tneg.s
tneg.bin-DEPS = b-test-tsparc-utils
