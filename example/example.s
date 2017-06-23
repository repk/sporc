.section .text.boot, "ax", @progbits
boot:
	sethi %hi(canary), %g2
	or %g0, 12, %g1
	or %g0, %g1, %g1
	st %g1, [%g2 + %lo(canary)]
	ld [%g2 + %lo(canary)], %g3

.section .text, "ax", @progbits

.section .bss, "aw", @nobits
canary:
	.space 4, 0
