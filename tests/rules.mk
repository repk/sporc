BUNDLE = b-test-utils b-test-tsparc-utils

b-test-utils-CSRC = test-utils.c
b-test-utils-INCLUDE = .
b-test-utils-DEPS = b-sporc

b-test-tsparc-utils-LDSCRIPT = sparc.ld

ifeq ($(TESTS),1)
TARGET = b-tests.sh
endif

b-tests.sh-FILE = tests.sh
b-tests.sh-OUTDIR = tests
