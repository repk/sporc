ifeq ($(TESTS),1)
	TARGET = t-be
	CROSSTARGET = be.bin
endif

t-be-OUTDIR = tests
t-be-CSRC = main.c
t-be-DEPS = b-test-utils

be.bin-OUTDIR = tests/binaries
be.bin-ASRC = be.s
be.bin-DEPS = b-test-tsparc-utils
