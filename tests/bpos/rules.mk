ifeq ($(TESTS),1)
	TARGET = t-bpos
	CROSSTARGET = bpos.bin
endif

t-bpos-OUTDIR = tests
t-bpos-CSRC = main.c
t-bpos-DEPS = b-test-utils

bpos.bin-OUTDIR = tests/binaries
bpos.bin-ASRC = bpos.s
bpos.bin-DEPS = b-test-tsparc-utils
