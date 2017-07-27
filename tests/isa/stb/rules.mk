ifeq ($(TESTS),1)
	TARGET = t-stb
	CROSSTARGET = stb.bin
endif

t-stb-OUTDIR = tests/isa
t-stb-CSRC = main.c
t-stb-DEPS = b-test-utils

stb.bin-OUTDIR = tests/binaries/isa
stb.bin-ASRC = stb.s
stb.bin-DEPS = b-test-tsparc-utils
