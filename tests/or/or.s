.section .text, "ax", @progbits

tmain:
	sethi %hi(0xff000000), %g3
	or %g0, 0xff, %g2
	or %g2, %g3, %g2

