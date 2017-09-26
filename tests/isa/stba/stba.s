.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbeef), %g1
	or %g0, %lo(0xdeadbeef), %g3
	or %g0, %lo(12), %g2
	stba %g2, [%g1 + %g3] 0x12
