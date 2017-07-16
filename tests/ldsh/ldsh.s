.section .text, "ax", @progbits

tmain:
	sethi %hi(0xfff4), %g1
	or %g1, %lo(0xfff4), %g1
	sth %g1, [%g0 + 0x20]
	ldsh [%g0 + 0x20], %g2
