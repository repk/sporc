ifeq ($(TESTS),1)
	TARGET = t-lda
	CROSSTARGET = lda.bin
endif

t-lda-OUTDIR = tests/isa
t-lda-CSRC = main.c
t-lda-DEPS = b-test-utils

lda.bin-OUTDIR = tests/binaries/isa
lda.bin-ASRC = lda.s
lda.bin-DEPS = b-test-tsparc-utils
