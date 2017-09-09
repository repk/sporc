ifeq ($(TESTS),1)
	TARGET = t-umul
	CROSSTARGET = umul.bin
endif

t-umul-OUTDIR = tests/isa
t-umul-CSRC = main.c
t-umul-DEPS = b-test-utils

umul.bin-OUTDIR = tests/binaries/isa
umul.bin-ASRC = umul.s
umul.bin-DEPS = b-test-tsparc-utils
