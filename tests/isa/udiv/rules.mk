ifeq ($(TESTS),1)
	TARGET = t-udiv
	CROSSTARGET = udiv.bin
endif

t-udiv-OUTDIR = tests/isa
t-udiv-CSRC = main.c
t-udiv-DEPS = b-test-utils

udiv.bin-OUTDIR = tests/binaries/isa
udiv.bin-ASRC = udiv.s
udiv.bin-DEPS = b-test-tsparc-utils
