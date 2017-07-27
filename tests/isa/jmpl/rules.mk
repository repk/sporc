ifeq ($(TESTS),1)
	TARGET = t-jmpl
	CROSSTARGET = jmpl.bin
endif

t-jmpl-OUTDIR = tests/isa
t-jmpl-CSRC = main.c
t-jmpl-DEPS = b-test-utils

jmpl.bin-OUTDIR = tests/binaries/isa
jmpl.bin-ASRC = jmpl.s
jmpl.bin-DEPS = b-test-tsparc-utils
