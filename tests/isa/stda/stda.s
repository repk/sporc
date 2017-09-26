.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbee0), %g1
	or %g0, %lo(0xdeadbee0), %g4
	or %g0, %lo(12), %g2
	or %g0, %lo(0x12), %g3
	stda %g2, [%g1 + %g4] 0x12
