ifeq ($(TESTS),1)
	TARGET = t-swapa
	CROSSTARGET = swapa.bin
endif

t-swapa-OUTDIR = tests/isa
t-swapa-CSRC = main.c
t-swapa-DEPS = b-test-utils

swapa.bin-OUTDIR = tests/binaries/isa
swapa.bin-ASRC = swapa.s
swapa.bin-DEPS = b-test-tsparc-utils
