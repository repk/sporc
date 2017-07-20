ifeq ($(TESTS),1)
	TARGET = t-bleu
	CROSSTARGET = bleu.bin
endif

t-bleu-OUTDIR = tests
t-bleu-CSRC = main.c
t-bleu-DEPS = b-test-utils

bleu.bin-OUTDIR = tests/binaries
bleu.bin-ASRC = bleu.s
bleu.bin-DEPS = b-test-tsparc-utils
