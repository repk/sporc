ifeq ($(TESTS),1)
	TARGET = t-sdivcc
	CROSSTARGET = sdivcc.bin
endif

t-sdivcc-OUTDIR = tests/isa
t-sdivcc-CSRC = main.c
t-sdivcc-DEPS = b-test-utils

sdivcc.bin-OUTDIR = tests/binaries/isa
sdivcc.bin-ASRC = sdivcc.s
sdivcc.bin-DEPS = b-test-tsparc-utils
