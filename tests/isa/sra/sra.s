.section .text, "ax", @progbits

tmain:
	sethi %hi(0xfff00000), %g1
	or %g0, 4, %g2
	sra %g1, %g2, %g1
	sethi %hi(0x7ff00000), %g2
	sra %g2, 4, %g2
