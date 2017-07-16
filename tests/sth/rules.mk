ifeq ($(TESTS),1)
	TARGET = t-sth
	CROSSTARGET = sth.bin
endif

t-sth-OUTDIR = tests
t-sth-CSRC = main.c
t-sth-DEPS = b-test-utils

sth.bin-OUTDIR = tests/binaries
sth.bin-ASRC = sth.s
sth.bin-DEPS = b-test-tsparc-utils
