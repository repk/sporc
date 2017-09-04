ifeq ($(TESTS),1)
	TARGET = t-tvs
	CROSSTARGET = tvs.bin
endif

t-tvs-OUTDIR = tests/isa
t-tvs-CSRC = main.c
t-tvs-DEPS = b-test-utils

tvs.bin-OUTDIR = tests/binaries/isa
tvs.bin-ASRC = tvs.s
tvs.bin-DEPS = b-test-tsparc-utils
