ifeq ($(TESTS),1)
	TARGET = t-ldstuba
	CROSSTARGET = ldstuba.bin
endif

t-ldstuba-OUTDIR = tests/isa
t-ldstuba-CSRC = main.c
t-ldstuba-DEPS = b-test-utils

ldstuba.bin-OUTDIR = tests/binaries/isa
ldstuba.bin-ASRC = ldstuba.s
ldstuba.bin-DEPS = b-test-tsparc-utils
