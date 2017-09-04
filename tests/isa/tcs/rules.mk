ifeq ($(TESTS),1)
	TARGET = t-tcs
	CROSSTARGET = tcs.bin
endif

t-tcs-OUTDIR = tests/isa
t-tcs-CSRC = main.c
t-tcs-DEPS = b-test-utils

tcs.bin-OUTDIR = tests/binaries/isa
tcs.bin-ASRC = tcs.s
tcs.bin-DEPS = b-test-tsparc-utils
