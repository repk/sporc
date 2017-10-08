ifeq ($(TESTS),1)
	TARGET = t-stbar
	CROSSTARGET = stbar.bin
endif

t-stbar-OUTDIR = tests/isa
t-stbar-CSRC = main.c
t-stbar-DEPS = b-test-utils

stbar.bin-OUTDIR = tests/binaries/isa
stbar.bin-ASRC = stbar.s
stbar.bin-DEPS = b-test-tsparc-utils
