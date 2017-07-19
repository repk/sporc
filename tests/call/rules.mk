ifeq ($(TESTS),1)
	TARGET = t-call
	CROSSTARGET = call.bin
endif

t-call-OUTDIR = tests
t-call-CSRC = main.c
t-call-DEPS = b-test-utils

call.bin-OUTDIR = tests/binaries
call.bin-ASRC = call.s
call.bin-DEPS = b-test-tsparc-utils
