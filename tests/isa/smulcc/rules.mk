ifeq ($(TESTS),1)
	TARGET = t-smulcc
	CROSSTARGET = smulcc.bin
endif

t-smulcc-OUTDIR = tests/isa
t-smulcc-CSRC = main.c
t-smulcc-DEPS = b-test-utils

smulcc.bin-OUTDIR = tests/binaries/isa
smulcc.bin-ASRC = smulcc.s
smulcc.bin-DEPS = b-test-tsparc-utils
