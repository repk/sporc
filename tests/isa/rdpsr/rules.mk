ifeq ($(TESTS),1)
	TARGET = t-rdpsr
	CROSSTARGET = rdpsr.bin
endif

t-rdpsr-OUTDIR = tests/isa
t-rdpsr-CSRC = main.c
t-rdpsr-DEPS = b-test-utils

rdpsr.bin-OUTDIR = tests/binaries/isa
rdpsr.bin-ASRC = rdpsr.s
rdpsr.bin-DEPS = b-test-tsparc-utils
