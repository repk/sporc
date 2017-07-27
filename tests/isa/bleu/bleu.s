.section .text, "ax", @progbits

tmain:
	/* Do not branch do not annul */
	addcc %g0, 0x1fff, %g2
	bleu 1f
	sethi %hi(0xdeadbeef), %g1


	/* Do not branch does annul */
	addcc %g0, 0x1, %g2
	bleu,a 1f
	sethi %hi(0xcacacaca), %g1

	/* Do branch + delay slot */
	addcc %g1, 0x1fff, %g2
	bleu 2f
	or %g1, %lo(0x0000be00), %g1
2:
	or %g1, %lo(0x000000ef), %g1

1:
	sethi %hi(0xd00d0000), %g1
