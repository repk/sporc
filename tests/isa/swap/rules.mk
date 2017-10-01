ifeq ($(TESTS),1)
	TARGET = t-swap
	CROSSTARGET = swap.bin
endif

t-swap-OUTDIR = tests/isa
t-swap-CSRC = main.c
t-swap-DEPS = b-test-utils

swap.bin-OUTDIR = tests/binaries/isa
swap.bin-ASRC = swap.s
swap.bin-DEPS = b-test-tsparc-utils

