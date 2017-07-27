.section .text, "ax", @progbits

tmain:
	sethi %hi(0xdeadbef0), %g1
	or %g1, %lo(0xdeadbef0), %g1
	add %g1, 0x1fff, %g1
	sethi %hi(0xdeacf425), %g1
	or %g1, %lo(0xdeacf425), %g1
	sethi %hi(0x0000caca), %g2
	or %g2, %lo(0x0000caca), %g2
	add %g1, %g2, %g1
