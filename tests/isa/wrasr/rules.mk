ifeq ($(TESTS),1)
	TARGET = t-wrasr
	CROSSTARGET = wrasr.bin
endif

t-wrasr-OUTDIR = tests/isa
t-wrasr-CSRC = main.c
t-wrasr-DEPS = b-test-utils

wrasr.bin-OUTDIR = tests/binaries/isa
wrasr.bin-ASRC = wrasr.s
wrasr.bin-DEPS = b-test-tsparc-utils
