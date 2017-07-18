.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
	xnor %g1, 0x1fff, %g1
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
	sethi %hi(0xffffffff), %g2
	or %g2, %lo(0xffffffff), %g2
	xnor %g1, %g2, %g1
