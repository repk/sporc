ifeq ($(TESTS),1)
	TARGET = t-bge
	CROSSTARGET = bge.bin
endif

t-bge-OUTDIR = tests
t-bge-CSRC = main.c
t-bge-DEPS = b-test-utils

bge.bin-OUTDIR = tests/binaries
bge.bin-ASRC = bge.s
bge.bin-DEPS = b-test-tsparc-utils
