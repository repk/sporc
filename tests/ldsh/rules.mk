ifeq ($(TESTS),1)
	TARGET = t-ldsh
	CROSSTARGET = ldsh.bin
endif

t-ldsh-OUTDIR = tests
t-ldsh-CSRC = main.c
t-ldsh-DEPS = b-test-utils

ldsh.bin-OUTDIR = tests/binaries
ldsh.bin-ASRC = ldsh.s
ldsh.bin-DEPS = b-test-tsparc-utils
