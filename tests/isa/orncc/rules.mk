ifeq ($(TESTS),1)
	TARGET = t-orncc
	CROSSTARGET = orncc.bin
endif

t-orncc-OUTDIR = tests/isa
t-orncc-CSRC = main.c
t-orncc-DEPS = b-test-utils

orncc.bin-OUTDIR = tests/binaries/isa
orncc.bin-ASRC = orncc.s
orncc.bin-DEPS = b-test-tsparc-utils
