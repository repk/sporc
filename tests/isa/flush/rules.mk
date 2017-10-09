ifeq ($(TESTS),1)
	TARGET = t-flush
	CROSSTARGET = flush.bin
endif

t-flush-OUTDIR = tests/isa
t-flush-CSRC = main.c
t-flush-DEPS = b-test-utils

flush.bin-OUTDIR = tests/binaries/isa
flush.bin-ASRC = flush.s
flush.bin-DEPS = b-test-tsparc-utils
