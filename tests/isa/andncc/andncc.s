.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
	andncc %g1, 0x110, %g1
	sethi %hi(0xffffffff), %g1
	or %g1, %lo(0xffffffff), %g1
	andncc %g1, 0x1fff, %g1
