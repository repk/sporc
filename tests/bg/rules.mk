ifeq ($(TESTS),1)
	TARGET = t-bg
	CROSSTARGET = bg.bin
endif

t-bg-OUTDIR = tests
t-bg-CSRC = main.c
t-bg-DEPS = b-test-utils

bg.bin-OUTDIR = tests/binaries
bg.bin-ASRC = bg.s
bg.bin-DEPS = b-test-tsparc-utils
