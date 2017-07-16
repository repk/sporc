.section .text, "ax", @progbits

tmain:
	sethi %hi(0xb16b00b5), %g1
	or %g1, %lo(0xb16b00b5), %g1
	sethi %hi(0xdeadbeef), %g2
	or %g2, %lo(0xdeadbeef), %g2
	std %g1, [%g0 + 0x20]
