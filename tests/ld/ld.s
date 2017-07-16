.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
	st %g1, [%g0 + 0x20]
	ld [%g0 + 0x20], %g2

