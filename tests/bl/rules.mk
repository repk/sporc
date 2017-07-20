ifeq ($(TESTS),1)
	TARGET = t-bl
	CROSSTARGET = bl.bin
endif

t-bl-OUTDIR = tests
t-bl-CSRC = main.c
t-bl-DEPS = b-test-utils

bl.bin-OUTDIR = tests/binaries
bl.bin-ASRC = bl.s
bl.bin-DEPS = b-test-tsparc-utils
