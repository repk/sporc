ifeq ($(TESTS),1)
	TARGET = t-ble
	CROSSTARGET = ble.bin
endif

t-ble-OUTDIR = tests/isa
t-ble-CSRC = main.c
t-ble-DEPS = b-test-utils

ble.bin-OUTDIR = tests/binaries/isa
ble.bin-ASRC = ble.s
ble.bin-DEPS = b-test-tsparc-utils
