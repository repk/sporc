ifeq ($(TESTS),1)
	TARGET = t-bvc
	CROSSTARGET = bvc.bin
endif

t-bvc-OUTDIR = tests
t-bvc-CSRC = main.c
t-bvc-DEPS = b-test-utils

bvc.bin-OUTDIR = tests/binaries
bvc.bin-ASRC = bvc.s
bvc.bin-DEPS = b-test-tsparc-utils
