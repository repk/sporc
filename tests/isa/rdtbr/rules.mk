ifeq ($(TESTS),1)
	TARGET = t-rdtbr
	CROSSTARGET = rdtbr.bin
endif

t-rdtbr-OUTDIR = tests/isa
t-rdtbr-CSRC = main.c
t-rdtbr-DEPS = b-test-utils

rdtbr.bin-OUTDIR = tests/binaries/isa
rdtbr.bin-ASRC = rdtbr.s
rdtbr.bin-DEPS = b-test-tsparc-utils
