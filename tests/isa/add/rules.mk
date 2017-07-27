ifeq ($(TESTS),1)
	TARGET = t-add
	CROSSTARGET = add.bin
endif

t-add-OUTDIR = tests/isa
t-add-CSRC = main.c
t-add-DEPS = b-test-utils

add.bin-OUTDIR = tests/binaries/isa
add.bin-ASRC = add.s
add.bin-DEPS = b-test-tsparc-utils
