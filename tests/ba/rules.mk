ifeq ($(TESTS),1)
	TARGET = t-ba
	CROSSTARGET = ba.bin
endif

t-ba-OUTDIR = tests
t-ba-CSRC = main.c
t-ba-DEPS = b-test-utils

ba.bin-OUTDIR = tests/binaries
ba.bin-ASRC = ba.s
ba.bin-DEPS = b-test-tsparc-utils
