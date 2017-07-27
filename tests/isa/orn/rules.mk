ifeq ($(TESTS),1)
	TARGET = t-orn
	CROSSTARGET = orn.bin
endif

t-orn-OUTDIR = tests/isa
t-orn-CSRC = main.c
t-orn-DEPS = b-test-utils

orn.bin-OUTDIR = tests/binaries/isa
orn.bin-ASRC = orn.s
orn.bin-DEPS = b-test-tsparc-utils
