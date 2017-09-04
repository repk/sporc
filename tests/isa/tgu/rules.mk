ifeq ($(TESTS),1)
	TARGET = t-tgu
	CROSSTARGET = tgu.bin
endif

t-tgu-OUTDIR = tests/isa
t-tgu-CSRC = main.c
t-tgu-DEPS = b-test-utils

tgu.bin-OUTDIR = tests/binaries/isa
tgu.bin-ASRC = tgu.s
tgu.bin-DEPS = b-test-tsparc-utils
