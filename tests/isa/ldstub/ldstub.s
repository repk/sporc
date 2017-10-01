.section .text, "ax", @progbits

tmain:
	sethi %hi(mem), %g1
	ldstub [%g1 + %lo(mem)], %g2
	ldub [%g1 + %lo(mem)], %g1

.section .data, "aw", @progbits
mem:
	.space 1, 0x12
