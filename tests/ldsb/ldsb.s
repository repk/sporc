.section .text, "ax", @progbits

tmain:
	or %g0, %lo(0xf4), %g1
	stb %g1, [%g0 + 0x10]
	ldsb [%g0 + 0x10], %g2
