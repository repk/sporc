ifeq ($(TESTS),1)
	TARGET = t-lduh
	CROSSTARGET = lduh.bin
endif

t-lduh-OUTDIR = tests
t-lduh-CSRC = main.c
t-lduh-DEPS = b-test-utils

lduh.bin-OUTDIR = tests/binaries
lduh.bin-ASRC = lduh.s
lduh.bin-DEPS = b-test-tsparc-utils
