ifeq ($(TESTS),1)
	TARGET = t-lduh
	CROSSTARGET = lduh.bin
endif

t-lduh-OUTDIR = tests/isa
t-lduh-CSRC = main.c
t-lduh-DEPS = b-test-utils

lduh.bin-OUTDIR = tests/binaries/isa
lduh.bin-ASRC = lduh.s
lduh.bin-DEPS = b-test-tsparc-utils
