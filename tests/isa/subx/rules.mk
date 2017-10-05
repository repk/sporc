ifeq ($(TESTS),1)
	TARGET = t-subx
	CROSSTARGET = subx.bin
endif

t-subx-OUTDIR = tests/isa
t-subx-CSRC = main.c
t-subx-DEPS = b-test-utils

subx.bin-OUTDIR = tests/binaries/isa
subx.bin-ASRC = subx.s
subx.bin-DEPS = b-test-tsparc-utils
