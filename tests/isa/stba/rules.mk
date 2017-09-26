ifeq ($(TESTS),1)
	TARGET = t-stba
	CROSSTARGET = stba.bin
endif

t-stba-OUTDIR = tests/isa
t-stba-CSRC = main.c
t-stba-DEPS = b-test-utils

stba.bin-OUTDIR = tests/binaries/isa
stba.bin-ASRC = stba.s
stba.bin-DEPS = b-test-tsparc-utils
