ifeq ($(TESTS),1)
	TARGET = t-ta
	CROSSTARGET = ta.bin
endif

t-ta-OUTDIR = tests/isa
t-ta-CSRC = main.c
t-ta-DEPS = b-test-utils

ta.bin-OUTDIR = tests/binaries/isa
ta.bin-ASRC = ta.s
ta.bin-DEPS = b-test-tsparc-utils
