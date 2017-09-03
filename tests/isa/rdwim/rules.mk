ifeq ($(TESTS),1)
	TARGET = t-rdwim
	CROSSTARGET = rdwim.bin
endif

t-rdwim-OUTDIR = tests/isa
t-rdwim-CSRC = main.c
t-rdwim-DEPS = b-test-utils

rdwim.bin-OUTDIR = tests/binaries/isa
rdwim.bin-ASRC = rdwim.s
rdwim.bin-DEPS = b-test-tsparc-utils
