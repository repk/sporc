ifeq ($(TESTS),1)
	TARGET = t-tge
	CROSSTARGET = tge.bin
endif

t-tge-OUTDIR = tests/isa
t-tge-CSRC = main.c
t-tge-DEPS = b-test-utils

tge.bin-OUTDIR = tests/binaries/isa
tge.bin-ASRC = tge.s
tge.bin-DEPS = b-test-tsparc-utils
