ifeq ($(TESTS),1)
	TARGET = t-xor
	CROSSTARGET = xor.bin
endif

t-xor-OUTDIR = tests/isa
t-xor-CSRC = main.c
t-xor-DEPS = b-test-utils

xor.bin-OUTDIR = tests/binaries/isa
xor.bin-ASRC = xor.s
xor.bin-DEPS = b-test-tsparc-utils
