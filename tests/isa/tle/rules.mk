ifeq ($(TESTS),1)
	TARGET = t-tle
	CROSSTARGET = tle.bin
endif

t-tle-OUTDIR = tests/isa
t-tle-CSRC = main.c
t-tle-DEPS = b-test-utils

tle.bin-OUTDIR = tests/binaries/isa
tle.bin-ASRC = tle.s
tle.bin-DEPS = b-test-tsparc-utils
