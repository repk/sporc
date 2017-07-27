ifeq ($(TESTS),1)
	TARGET = t-bvc
	CROSSTARGET = bvc.bin
endif

t-bvc-OUTDIR = tests/isa
t-bvc-CSRC = main.c
t-bvc-DEPS = b-test-utils

bvc.bin-OUTDIR = tests/binaries/isa
bvc.bin-ASRC = bvc.s
bvc.bin-DEPS = b-test-tsparc-utils
