ifeq ($(TESTS),1)
	TARGET = t-sll
	CROSSTARGET = sll.bin
endif

t-sll-OUTDIR = tests/isa
t-sll-CSRC = main.c
t-sll-DEPS = b-test-utils

sll.bin-OUTDIR = tests/binaries/isa
sll.bin-ASRC = sll.s
sll.bin-DEPS = b-test-tsparc-utils
