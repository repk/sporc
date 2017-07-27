ifeq ($(TESTS),1)
	TARGET = t-g0
	CROSSTARGET = g0.bin
endif

t-g0-OUTDIR = tests
t-g0-CSRC = main.c
t-g0-DEPS = b-test-utils

g0.bin-OUTDIR = tests/binaries
g0.bin-ASRC = g0.s
g0.bin-DEPS = b-test-tsparc-utils
