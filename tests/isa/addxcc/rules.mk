ifeq ($(TESTS),1)
	TARGET = t-addxcc
	CROSSTARGET = addxcc.bin
endif

t-addxcc-OUTDIR = tests/isa
t-addxcc-CSRC = main.c
t-addxcc-DEPS = b-test-utils

addxcc.bin-OUTDIR = tests/binaries/isa
addxcc.bin-ASRC = addxcc.s
addxcc.bin-DEPS = b-test-tsparc-utils
