ifeq ($(TESTS),1)
	TARGET = t-tleu
	CROSSTARGET = tleu.bin
endif

t-tleu-OUTDIR = tests/isa
t-tleu-CSRC = main.c
t-tleu-DEPS = b-test-utils

tleu.bin-OUTDIR = tests/binaries/isa
tleu.bin-ASRC = tleu.s
tleu.bin-DEPS = b-test-tsparc-utils
