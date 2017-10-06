ifeq ($(TESTS),1)
	TARGET = t-tsubcctv
	CROSSTARGET = tsubcctv.bin
endif

t-tsubcctv-OUTDIR = tests/isa
t-tsubcctv-CSRC = main.c
t-tsubcctv-DEPS = b-test-utils

tsubcctv.bin-OUTDIR = tests/binaries/isa
tsubcctv.bin-ASRC = tsubcctv.s
tsubcctv.bin-DEPS = b-test-tsparc-utils
