ifeq ($(TESTS),1)
	TARGET = t-bgu
	CROSSTARGET = bgu.bin
endif

t-bgu-OUTDIR = tests
t-bgu-CSRC = main.c
t-bgu-DEPS = b-test-utils

bgu.bin-OUTDIR = tests/binaries
bgu.bin-ASRC = bgu.s
bgu.bin-DEPS = b-test-tsparc-utils
