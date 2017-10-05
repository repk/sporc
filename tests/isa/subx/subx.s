.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbef0), %g1
	or %g1, %lo(0xdeadbef0), %g1
	addcc %g1, %g1, %g2
	subx %g1, %g0, %g1
