ifeq ($(TESTS),1)
	TARGET = t-sub
	CROSSTARGET = sub.bin
endif

t-sub-OUTDIR = tests
t-sub-CSRC = main.c
t-sub-DEPS = b-test-utils

sub.bin-OUTDIR = tests/binaries
sub.bin-ASRC = sub.s
sub.bin-DEPS = b-test-tsparc-utils
