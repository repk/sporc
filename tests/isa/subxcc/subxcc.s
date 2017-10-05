.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbef0), %g1
	or %g1, %lo(0xdeadbef0), %g1
	orcc %g0, 1, %g0 /* Clear flags */
	subxcc %g1, %g1, %g2
	subxcc %g1, %g2, %g1
	subx %g1, %g0, %g1
