ifeq ($(TESTS),1)
	TARGET = t-be
	CROSSTARGET = be.bin
endif

t-be-OUTDIR = tests/isa
t-be-CSRC = main.c
t-be-DEPS = b-test-utils

be.bin-OUTDIR = tests/binaries/isa
be.bin-ASRC = be.s
be.bin-DEPS = b-test-tsparc-utils
