ifeq ($(TESTS),1)
	TARGET = t-taddcc
	CROSSTARGET = taddcc.bin
endif

t-taddcc-OUTDIR = tests/isa
t-taddcc-CSRC = main.c
t-taddcc-DEPS = b-test-utils

taddcc.bin-OUTDIR = tests/binaries/isa
taddcc.bin-ASRC = taddcc.s
taddcc.bin-DEPS = b-test-tsparc-utils
