.section .text, "ax", @progbits

tmain:
	or %g0, %lo(0x0c), %g1
	stb %g1, [%g0 + 0x10]
