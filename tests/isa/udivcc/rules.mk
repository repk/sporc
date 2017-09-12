ifeq ($(TESTS),1)
	TARGET = t-udivcc
	CROSSTARGET = udivcc.bin
endif

t-udivcc-OUTDIR = tests/isa
t-udivcc-CSRC = main.c
t-udivcc-DEPS = b-test-utils

udivcc.bin-OUTDIR = tests/binaries/isa
udivcc.bin-ASRC = udivcc.s
udivcc.bin-DEPS = b-test-tsparc-utils
