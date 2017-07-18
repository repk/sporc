ifeq ($(TESTS),1)
	TARGET = t-andcc
	CROSSTARGET = andcc.bin
endif

t-andcc-OUTDIR = tests
t-andcc-CSRC = main.c
t-andcc-DEPS = b-test-utils

andcc.bin-OUTDIR = tests/binaries
andcc.bin-ASRC = andcc.s
andcc.bin-DEPS = b-test-tsparc-utils
