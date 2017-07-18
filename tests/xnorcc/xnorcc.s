.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
	xnorcc %g1, 0x1fff, %g1
	sethi %hi(0x21524110), %g2
	or %g2, %lo(0x21524110), %g2
	xnorcc %g1, %g2, %g1
