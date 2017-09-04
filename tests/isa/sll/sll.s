.section .text, "ax", @progbits

tmain:
	sethi %hi(0xfff00000), %g1
	or %g0, 4, %g2
	sll %g1, 4, %g1
	sll %g1, %g2, %g1

