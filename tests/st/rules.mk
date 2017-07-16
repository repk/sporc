ifeq ($(TESTS),1)
	TARGET = t-st
	CROSSTARGET = st.bin
endif

t-st-OUTDIR = tests
t-st-CSRC = main.c
t-st-DEPS = b-test-utils

st.bin-OUTDIR = tests/binaries
st.bin-ASRC = st.s
st.bin-DEPS = b-test-tsparc-utils
