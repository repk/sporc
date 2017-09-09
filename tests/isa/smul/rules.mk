ifeq ($(TESTS),1)
	TARGET = t-smul
	CROSSTARGET = smul.bin
endif

t-smul-OUTDIR = tests/isa
t-smul-CSRC = main.c
t-smul-DEPS = b-test-utils

smul.bin-OUTDIR = tests/binaries/isa
smul.bin-ASRC = smul.s
smul.bin-DEPS = b-test-tsparc-utils
