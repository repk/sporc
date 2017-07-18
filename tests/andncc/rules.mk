ifeq ($(TESTS),1)
	TARGET = t-andncc
	CROSSTARGET = andncc.bin
endif

t-andncc-OUTDIR = tests
t-andncc-CSRC = main.c
t-andncc-DEPS = b-test-utils

andncc.bin-OUTDIR = tests/binaries
andncc.bin-ASRC = andncc.s
andncc.bin-DEPS = b-test-tsparc-utils
