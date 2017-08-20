ifeq ($(TESTS),1)
	TARGET = t-wrpsr
	CROSSTARGET = wrpsr.bin
endif

t-wrpsr-OUTDIR = tests/isa
t-wrpsr-CSRC = main.c
t-wrpsr-DEPS = b-test-utils

wrpsr.bin-OUTDIR = tests/binaries/isa
wrpsr.bin-ASRC = wrpsr.s
wrpsr.bin-DEPS = b-test-tsparc-utils
