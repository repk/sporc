ifeq ($(TESTS),1)
	TARGET = t-ldsh
	CROSSTARGET = ldsh.bin
endif

t-ldsh-OUTDIR = tests/isa
t-ldsh-CSRC = main.c
t-ldsh-DEPS = b-test-utils

ldsh.bin-OUTDIR = tests/binaries/isa
ldsh.bin-ASRC = ldsh.s
ldsh.bin-DEPS = b-test-tsparc-utils
