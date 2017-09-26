ifeq ($(TESTS),1)
	TARGET = t-ldda
	CROSSTARGET = ldda.bin
endif

t-ldda-OUTDIR = tests/isa
t-ldda-CSRC = main.c
t-ldda-DEPS = b-test-utils

ldda.bin-OUTDIR = tests/binaries/isa
ldda.bin-ASRC = ldda.s
ldda.bin-DEPS = b-test-tsparc-utils
