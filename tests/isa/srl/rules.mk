ifeq ($(TESTS),1)
	TARGET = t-srl
	CROSSTARGET = srl.bin
endif

t-srl-OUTDIR = tests/isa
t-srl-CSRC = main.c
t-srl-DEPS = b-test-utils

srl.bin-OUTDIR = tests/binaries/isa
srl.bin-ASRC = srl.s
srl.bin-DEPS = b-test-tsparc-utils
