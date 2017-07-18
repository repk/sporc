.section .text, "ax", @progbits

tmain:
	sethi %hi(0x0000caca), %g1
	or %g1, %lo(0x0000caca), %g1
	sethi %hi(0x80000000), %g2
	or %g2, %lo(0x80000000), %g2
	orcc %g1, %g2, %g1

	orcc %g0, %g0, %g1
