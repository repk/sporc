.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbeee), %g1
	or %g1, %lo(0xdeadbeee), %g1
	sethi %hi(0x21524112), %g2
	or %g2, %lo(0x21524112), %g2
	orcc %g0, 1, %g0 /* Clear flags */
	addxcc %g1, %g2, %g2
	addxcc %g2, %g1, %g1
	addx %g0, %g1, %g1
