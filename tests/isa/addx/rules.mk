ifeq ($(TESTS),1)
	TARGET = t-addx
	CROSSTARGET = addx.bin
endif

t-addx-OUTDIR = tests/isa
t-addx-CSRC = main.c
t-addx-DEPS = b-test-utils

addx.bin-OUTDIR = tests/binaries/isa
addx.bin-ASRC = addx.s
addx.bin-DEPS = b-test-tsparc-utils
