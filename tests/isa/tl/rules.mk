ifeq ($(TESTS),1)
	TARGET = t-tl
	CROSSTARGET = tl.bin
endif

t-tl-OUTDIR = tests/isa
t-tl-CSRC = main.c
t-tl-DEPS = b-test-utils

tl.bin-OUTDIR = tests/binaries/isa
tl.bin-ASRC = tl.s
tl.bin-DEPS = b-test-tsparc-utils
