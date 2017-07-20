ifeq ($(TESTS),1)
	TARGET = t-bcc
	CROSSTARGET = bcc.bin
endif

t-bcc-OUTDIR = tests
t-bcc-CSRC = main.c
t-bcc-DEPS = b-test-utils

bcc.bin-OUTDIR = tests/binaries
bcc.bin-ASRC = bcc.s
bcc.bin-DEPS = b-test-tsparc-utils
