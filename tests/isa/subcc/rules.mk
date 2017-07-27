ifeq ($(TESTS),1)
	TARGET = t-subcc
	CROSSTARGET = subcc.bin
endif

t-subcc-OUTDIR = tests/isa
t-subcc-CSRC = main.c
t-subcc-DEPS = b-test-utils

subcc.bin-OUTDIR = tests/binaries/isa
subcc.bin-ASRC = subcc.s
subcc.bin-DEPS = b-test-tsparc-utils
