ifeq ($(TESTS),1)
	TARGET = t-ldd
	CROSSTARGET = ldd.bin
endif

t-ldd-OUTDIR = tests
t-ldd-CSRC = main.c
t-ldd-DEPS = b-test-utils

ldd.bin-OUTDIR = tests/binaries
ldd.bin-ASRC = ldd.s
ldd.bin-DEPS = b-test-tsparc-utils
