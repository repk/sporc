ifeq ($(TESTS),1)
	TARGET = t-tvc
	CROSSTARGET = tvc.bin
endif

t-tvc-OUTDIR = tests/isa
t-tvc-CSRC = main.c
t-tvc-DEPS = b-test-utils

tvc.bin-OUTDIR = tests/binaries/isa
tvc.bin-ASRC = tvc.s
tvc.bin-DEPS = b-test-tsparc-utils
