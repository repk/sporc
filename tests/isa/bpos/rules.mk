ifeq ($(TESTS),1)
	TARGET = t-bpos
	CROSSTARGET = bpos.bin
endif

t-bpos-OUTDIR = tests/isa
t-bpos-CSRC = main.c
t-bpos-DEPS = b-test-utils

bpos.bin-OUTDIR = tests/binaries/isa
bpos.bin-ASRC = bpos.s
bpos.bin-DEPS = b-test-tsparc-utils
