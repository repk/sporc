ifeq ($(TESTS),1)
	TARGET = t-wrtbr
	CROSSTARGET = wrtbr.bin
endif

t-wrtbr-OUTDIR = tests/isa
t-wrtbr-CSRC = main.c
t-wrtbr-DEPS = b-test-utils

wrtbr.bin-OUTDIR = tests/binaries/isa
wrtbr.bin-ASRC = wrtbr.s
wrtbr.bin-DEPS = b-test-tsparc-utils
