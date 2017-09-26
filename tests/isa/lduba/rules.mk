ifeq ($(TESTS),1)
	TARGET = t-lduba
	CROSSTARGET = lduba.bin
endif

t-lduba-OUTDIR = tests/isa
t-lduba-CSRC = main.c
t-lduba-DEPS = b-test-utils

lduba.bin-OUTDIR = tests/binaries/isa
lduba.bin-ASRC = lduba.s
lduba.bin-DEPS = b-test-tsparc-utils
