.section .text, "ax", @progbits

tmain:
	sethi %hi(0xb16b00b5), %g2
	or %g2, %lo(0xb16b00b5), %g2
	sethi %hi(0xdeadbeef), %g3
	or %g3, %lo(0xdeadbeef), %g3
	std %g2, [%g0 + 0x20]
	ldd [%g0 + 0x20], %g4

