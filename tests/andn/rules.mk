ifeq ($(TESTS),1)
	TARGET = t-andn
	CROSSTARGET = andn.bin
endif

t-andn-OUTDIR = tests
t-andn-CSRC = main.c
t-andn-DEPS = b-test-utils

andn.bin-OUTDIR = tests/binaries
andn.bin-ASRC = andn.s
andn.bin-DEPS = b-test-tsparc-utils
