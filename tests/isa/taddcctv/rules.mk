ifeq ($(TESTS),1)
	TARGET = t-taddcctv
	CROSSTARGET = taddcctv.bin
endif

t-taddcctv-OUTDIR = tests/isa
t-taddcctv-CSRC = main.c
t-taddcctv-DEPS = b-test-utils

taddcctv.bin-OUTDIR = tests/binaries/isa
taddcctv.bin-ASRC = taddcctv.s
taddcctv.bin-DEPS = b-test-tsparc-utils
