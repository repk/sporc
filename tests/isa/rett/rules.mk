ifeq ($(TESTS),1)
	TARGET = t-rett
	CROSSTARGET = rett.bin
endif

t-rett-OUTDIR = tests/isa
t-rett-CSRC = main.c
t-rett-DEPS = b-test-utils

rett.bin-OUTDIR = tests/binaries/isa
rett.bin-ASRC = rett.s
rett.bin-DEPS = b-test-tsparc-utils
