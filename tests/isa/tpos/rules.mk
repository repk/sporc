ifeq ($(TESTS),1)
	TARGET = t-tpos
	CROSSTARGET = tpos.bin
endif

t-tpos-OUTDIR = tests/isa
t-tpos-CSRC = main.c
t-tpos-DEPS = b-test-utils

tpos.bin-OUTDIR = tests/binaries/isa
tpos.bin-ASRC = tpos.s
tpos.bin-DEPS = b-test-tsparc-utils
