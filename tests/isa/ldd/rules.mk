ifeq ($(TESTS),1)
	TARGET = t-ldd
	CROSSTARGET = ldd.bin
endif

t-ldd-OUTDIR = tests/isa
t-ldd-CSRC = main.c
t-ldd-DEPS = b-test-utils

ldd.bin-OUTDIR = tests/binaries/isa
ldd.bin-ASRC = ldd.s
ldd.bin-DEPS = b-test-tsparc-utils
