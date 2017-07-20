ifeq ($(TESTS),1)
	TARGET = t-bn
	CROSSTARGET = bn.bin
endif

t-bn-OUTDIR = tests
t-bn-CSRC = main.c
t-bn-DEPS = b-test-utils

bn.bin-OUTDIR = tests/binaries
bn.bin-ASRC = bn.s
bn.bin-DEPS = b-test-tsparc-utils
