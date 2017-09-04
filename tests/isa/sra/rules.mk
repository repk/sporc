ifeq ($(TESTS),1)
	TARGET = t-sra
	CROSSTARGET = sra.bin
endif

t-sra-OUTDIR = tests/isa
t-sra-CSRC = main.c
t-sra-DEPS = b-test-utils

sra.bin-OUTDIR = tests/binaries/isa
sra.bin-ASRC = sra.s
sra.bin-DEPS = b-test-tsparc-utils
