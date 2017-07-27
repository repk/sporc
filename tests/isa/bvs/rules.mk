ifeq ($(TESTS),1)
	TARGET = t-bvs
	CROSSTARGET = bvs.bin
endif

t-bvs-OUTDIR = tests/isa
t-bvs-CSRC = main.c
t-bvs-DEPS = b-test-utils

bvs.bin-OUTDIR = tests/binaries/isa
bvs.bin-ASRC = bvs.s
bvs.bin-DEPS = b-test-tsparc-utils
