.section .text, "ax", @progbits

tmain:
	/* Set all psr bits to 1 but reserve field */
	sethi %hi(0xfff03fff), %g2
	wr %g2, %lo(0xfff03fff), %psr
	rd %psr, %g1
