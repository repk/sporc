ifeq ($(TESTS),1)
	TARGET = t-sth
	CROSSTARGET = sth.bin
endif

t-sth-OUTDIR = tests/isa
t-sth-CSRC = main.c
t-sth-DEPS = b-test-utils

sth.bin-OUTDIR = tests/binaries/isa
sth.bin-ASRC = sth.s
sth.bin-DEPS = b-test-tsparc-utils
