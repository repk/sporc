ifeq ($(TESTS),1)
	TARGET = t-wrwim
	CROSSTARGET = wrwim.bin
endif

t-wrwim-OUTDIR = tests/isa
t-wrwim-CSRC = main.c
t-wrwim-DEPS = b-test-utils

wrwim.bin-OUTDIR = tests/binaries/isa
wrwim.bin-ASRC = wrwim.s
wrwim.bin-DEPS = b-test-tsparc-utils
