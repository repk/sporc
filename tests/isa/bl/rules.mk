ifeq ($(TESTS),1)
	TARGET = t-bl
	CROSSTARGET = bl.bin
endif

t-bl-OUTDIR = tests/isa
t-bl-CSRC = main.c
t-bl-DEPS = b-test-utils

bl.bin-OUTDIR = tests/binaries/isa
bl.bin-ASRC = bl.s
bl.bin-DEPS = b-test-tsparc-utils
