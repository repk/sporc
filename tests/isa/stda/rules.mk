ifeq ($(TESTS),1)
	TARGET = t-stda
	CROSSTARGET = stda.bin
endif

t-stda-OUTDIR = tests/isa
t-stda-CSRC = main.c
t-stda-DEPS = b-test-utils

stda.bin-OUTDIR = tests/binaries/isa
stda.bin-ASRC = stda.s
stda.bin-DEPS = b-test-tsparc-utils
