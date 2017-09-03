ifeq ($(TESTS),1)
	TARGET = t-tn
	CROSSTARGET = tn.bin
endif

t-tn-OUTDIR = tests/isa
t-tn-CSRC = main.c
t-tn-DEPS = b-test-utils

tn.bin-OUTDIR = tests/binaries/isa
tn.bin-ASRC = tn.s
tn.bin-DEPS = b-test-tsparc-utils
