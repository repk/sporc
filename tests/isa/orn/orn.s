.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbeef), %g1
	or %g1, %lo(0xdeadbeef), %g1
	orn %g1, 0x1fff, %g1
	sethi %hi(0x21524110), %g1
	or %g1, %lo(0x21524110), %g1
	orn %g0, %g1, %g1
