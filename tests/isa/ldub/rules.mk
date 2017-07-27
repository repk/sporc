ifeq ($(TESTS),1)
	TARGET = t-ldub
	CROSSTARGET = ldub.bin
endif

t-ldub-OUTDIR = tests/isa
t-ldub-CSRC = main.c
t-ldub-DEPS = b-test-utils

ldub.bin-OUTDIR = tests/binaries/isa
ldub.bin-ASRC = ldub.s
ldub.bin-DEPS = b-test-tsparc-utils

