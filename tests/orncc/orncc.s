.section .text, "ax", @progbits

tmain:
	sethi %hi(0x7fffffff), %g1
	or %g1, %lo(0x7fffffff), %g1
	orncc %g0, %g1, %g1
	sethi %hi(0xffffffff), %g1
	or %g1, %lo(0xffffffff), %g1
	orncc %g0, %g1, %g1
