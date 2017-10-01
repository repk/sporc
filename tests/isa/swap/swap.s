.section .text, "ax", @progbits

tmain:
	sethi %hi(mem), %g1
	or %g0, 0x12, %g2
	swap [%g1 + %lo(mem)], %g2
	ld [%g1 + %lo(mem)], %g1

.section .data, "aw", @progbits
mem:
	.byte 0xde,0xad,0xbe,0xef
