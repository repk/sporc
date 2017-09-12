ifeq ($(TESTS),1)
	TARGET = t-sdiv
	CROSSTARGET = sdiv.bin
endif

t-sdiv-OUTDIR = tests/isa
t-sdiv-CSRC = main.c
t-sdiv-DEPS = b-test-utils

sdiv.bin-OUTDIR = tests/binaries/isa
sdiv.bin-ASRC = sdiv.s
sdiv.bin-DEPS = b-test-tsparc-utils
