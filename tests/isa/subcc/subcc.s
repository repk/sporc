.section .text, "ax", @progbits

tmain:
	sethi %hi(0x80000000), %g1
	or %g1, %lo(0x80000000), %g1
	subcc %g1, 0x1, %g1
	sethi %hi(0x80000000), %g2
	or %g2, %lo(0x80000000), %g2
	subcc %g1, %g2, %g1
