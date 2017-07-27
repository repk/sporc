.section .text, "ax", @progbits

tmain:
	sethi %hi(0x21524110), %g1
	or %g1, %lo(0x21524110), %g1
	andn %g1, 0x1fff, %g1
	sethi %hi(0x21524110), %g1
	or %g1, %lo(0x2152411f), %g1
	sethi %hi(0xfffffff0), %g2
	or %g2, %lo(0xfffffff0), %g2
	andn %g1, %g2, %g1
