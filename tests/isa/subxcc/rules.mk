ifeq ($(TESTS),1)
	TARGET = t-subxcc
	CROSSTARGET = subxcc.bin
endif

t-subxcc-OUTDIR = tests/isa
t-subxcc-CSRC = main.c
t-subxcc-DEPS = b-test-utils

subxcc.bin-OUTDIR = tests/binaries/isa
subxcc.bin-ASRC = subxcc.s
subxcc.bin-DEPS = b-test-tsparc-utils
