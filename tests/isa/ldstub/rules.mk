ifeq ($(TESTS),1)
	TARGET = t-ldstub
	CROSSTARGET = ldstub.bin
endif

t-ldstub-OUTDIR = tests/isa
t-ldstub-CSRC = main.c
t-ldstub-DEPS = b-test-utils

ldstub.bin-OUTDIR = tests/binaries/isa
ldstub.bin-ASRC = ldstub.s
ldstub.bin-DEPS = b-test-tsparc-utils

