ifeq ($(TESTS),1)
	TARGET = t-stha
	CROSSTARGET = stha.bin
endif

t-stha-OUTDIR = tests/isa
t-stha-CSRC = main.c
t-stha-DEPS = b-test-utils

stha.bin-OUTDIR = tests/binaries/isa
stha.bin-ASRC = stha.s
stha.bin-DEPS = b-test-tsparc-utils
