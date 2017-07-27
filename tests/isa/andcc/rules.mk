ifeq ($(TESTS),1)
	TARGET = t-andcc
	CROSSTARGET = andcc.bin
endif

t-andcc-OUTDIR = tests/isa
t-andcc-CSRC = main.c
t-andcc-DEPS = b-test-utils

andcc.bin-OUTDIR = tests/binaries/isa
andcc.bin-ASRC = andcc.s
andcc.bin-DEPS = b-test-tsparc-utils
