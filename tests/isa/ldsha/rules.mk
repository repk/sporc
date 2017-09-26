ifeq ($(TESTS),1)
	TARGET = t-ldsha
	CROSSTARGET = ldsha.bin
endif

t-ldsha-OUTDIR = tests/isa
t-ldsha-CSRC = main.c
t-ldsha-DEPS = b-test-utils

ldsha.bin-OUTDIR = tests/binaries/isa
ldsha.bin-ASRC = ldsha.s
ldsha.bin-DEPS = b-test-tsparc-utils
