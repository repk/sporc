ifeq ($(TESTS),1)
	TARGET = t-lduha
	CROSSTARGET = lduha.bin
endif

t-lduha-OUTDIR = tests/isa
t-lduha-CSRC = main.c
t-lduha-DEPS = b-test-utils

lduha.bin-OUTDIR = tests/binaries/isa
lduha.bin-ASRC = lduha.s
lduha.bin-DEPS = b-test-tsparc-utils
