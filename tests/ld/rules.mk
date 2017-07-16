ifeq ($(TESTS),1)
	TARGET = t-ld
	CROSSTARGET = ld.bin
endif

t-ld-OUTDIR = tests
t-ld-CSRC = main.c
t-ld-DEPS = b-test-utils

ld.bin-OUTDIR = tests/binaries
ld.bin-ASRC = ld.s
ld.bin-DEPS = b-test-tsparc-utils
