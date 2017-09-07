ifeq ($(TESTS),1)
	TARGET = t-rdasr
	CROSSTARGET = rdasr.bin
endif

t-rdasr-OUTDIR = tests/isa
t-rdasr-CSRC = main.c
t-rdasr-DEPS = b-test-utils

rdasr.bin-OUTDIR = tests/binaries/isa
rdasr.bin-ASRC = rdasr.s
rdasr.bin-DEPS = b-test-tsparc-utils
