ifeq ($(TESTS),1)
	TARGET = t-bne
	CROSSTARGET = bne.bin
endif

t-bne-OUTDIR = tests/isa
t-bne-CSRC = main.c
t-bne-DEPS = b-test-utils

bne.bin-OUTDIR = tests/binaries/isa
bne.bin-ASRC = bne.s
bne.bin-DEPS = b-test-tsparc-utils
