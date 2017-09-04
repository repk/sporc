ifeq ($(TESTS),1)
	TARGET = t-tg
	CROSSTARGET = tg.bin
endif

t-tg-OUTDIR = tests/isa
t-tg-CSRC = main.c
t-tg-DEPS = b-test-utils

tg.bin-OUTDIR = tests/binaries/isa
tg.bin-ASRC = tg.s
tg.bin-DEPS = b-test-tsparc-utils
