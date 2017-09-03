.section .text, "ax", @progbits

tmain:
	sub %g0, 1, %g2
	wr %g2, 0, %wim
	rd %wim, %g1
