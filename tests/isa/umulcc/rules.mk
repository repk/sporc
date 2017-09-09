ifeq ($(TESTS),1)
	TARGET = t-umulcc
	CROSSTARGET = umulcc.bin
endif

t-umulcc-OUTDIR = tests/isa
t-umulcc-CSRC = main.c
t-umulcc-DEPS = b-test-utils

umulcc.bin-OUTDIR = tests/binaries/isa
umulcc.bin-ASRC = umulcc.s
umulcc.bin-DEPS = b-test-tsparc-utils
