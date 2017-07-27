ifeq ($(TESTS),1)
	TARGET = t-save-restore
	CROSSTARGET = save-restore.bin
endif

t-save-restore-OUTDIR = tests/isa
t-save-restore-CSRC = main.c
t-save-restore-DEPS = b-test-utils

save-restore.bin-OUTDIR = tests/binaries/isa
save-restore.bin-ASRC = save-restore.s
save-restore.bin-DEPS = b-test-tsparc-utils
