.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbee0), %g1
	or %g0, %lo(0xdeadbee0), %g3
	or %g0, 12, %g2
	swapa [%g1 + %g3] 0x12, %g2
