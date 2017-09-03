.section .text, "ax", @progbits

tmain:
	sethi %hi(0xfffffff0), %g2
	or %g2, %lo(0xfffffff0), %g2
	wr %g2, 0, %tbr
	rd %tbr, %g1
