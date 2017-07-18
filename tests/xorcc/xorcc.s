.section .text, "ax", @progbits

tmain:
	sethi %hi(0x21524110), %g1
	or %g1, %lo(0x21524110), %g1
	xorcc %g1, 0x1fff, %g1
	xorcc %g1, %g1, %g1
