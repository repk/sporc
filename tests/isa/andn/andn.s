.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
	andn %g1, 0x110, %g1
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
	sethi %hi(0x21524110), %g2
	or %g2, %lo(0x21524110), %g2
	andn %g1, %g2, %g1
