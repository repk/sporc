ifeq ($(TESTS),1)
	TARGET = t-stb
	CROSSTARGET = stb.bin
endif

t-stb-OUTDIR = tests
t-stb-CSRC = main.c
t-stb-DEPS = b-test-utils

stb.bin-OUTDIR = tests/binaries
stb.bin-ASRC = stb.s
stb.bin-DEPS = b-test-tsparc-utils
