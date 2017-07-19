ifeq ($(TESTS),1)
	TARGET = t-jmpl
	CROSSTARGET = jmpl.bin
endif

t-jmpl-OUTDIR = tests
t-jmpl-CSRC = main.c
t-jmpl-DEPS = b-test-utils

jmpl.bin-OUTDIR = tests/binaries
jmpl.bin-ASRC = jmpl.s
jmpl.bin-DEPS = b-test-tsparc-utils
