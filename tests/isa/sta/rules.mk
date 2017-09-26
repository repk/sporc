ifeq ($(TESTS),1)
	TARGET = t-sta
	CROSSTARGET = sta.bin
endif

t-sta-OUTDIR = tests/isa
t-sta-CSRC = main.c
t-sta-DEPS = b-test-utils

sta.bin-OUTDIR = tests/binaries/isa
sta.bin-ASRC = sta.s
sta.bin-DEPS = b-test-tsparc-utils
