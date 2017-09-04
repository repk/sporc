ifeq ($(TESTS),1)
	TARGET = t-tne
	CROSSTARGET = tne.bin
endif

t-tne-OUTDIR = tests/isa
t-tne-CSRC = main.c
t-tne-DEPS = b-test-utils

tne.bin-OUTDIR = tests/binaries/isa
tne.bin-ASRC = tne.s
tne.bin-DEPS = b-test-tsparc-utils
