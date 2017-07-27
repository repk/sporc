.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbeee), %g1
	or %g1, %lo(0xdeadbeee), %g1
	sub %g1, 0x1fff, %g1
	sethi %hi(0xdeae89b9), %g1
	or %g1, %lo(0xdeae89b9), %g1
	sethi %hi(0x0000caca), %g2
	or %g2, %lo(0x0000caca), %g2
	sub %g1, %g2, %g1
