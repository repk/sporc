ifeq ($(TESTS),1)
	TARGET = t-mmu
	CROSSTARGET = mmu.bin
endif

t-mmu-OUTDIR = tests/mmu
t-mmu-CSRC = main.c
t-mmu-DEPS = b-test-utils

mmu.bin-OUTDIR = tests/binaries/mmu
mmu.bin-ASRC = mmu.s
mmu.bin-DEPS = b-test-tsparc-utils
