ifeq ($(TESTS),1)
	TARGET = t-ldsba
	CROSSTARGET = ldsba.bin
endif

t-ldsba-OUTDIR = tests/isa
t-ldsba-CSRC = main.c
t-ldsba-DEPS = b-test-utils

ldsba.bin-OUTDIR = tests/binaries/isa
ldsba.bin-ASRC = ldsba.s
ldsba.bin-DEPS = b-test-tsparc-utils
