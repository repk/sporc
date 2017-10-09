ifeq ($(TESTS),1)
	TARGET = t-unimp
	CROSSTARGET = unimp.bin
endif

t-unimp-OUTDIR = tests/isa
t-unimp-CSRC = main.c
t-unimp-DEPS = b-test-utils

unimp.bin-OUTDIR = tests/binaries/isa
unimp.bin-ASRC = unimp.s
unimp.bin-DEPS = b-test-tsparc-utils
