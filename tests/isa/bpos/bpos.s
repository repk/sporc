.section .text, "ax", @progbits

tmain:
	/* Do not branch do not annul */
	subcc %g0, 0x1, %g1
	bcs 1f
	sethi %hi(0xdeadbeef), %g1


	/* Do not branch does annul */
	subcc %g0, 0x1, %g2
	bcs,a 1f
	sethi %hi(0xcacacaca), %g1

	/* Do branch + delay slot */
	addcc %g0, 0x1, %g2
	bcs 2f
	or %g1, %lo(0x0000be00), %g1
2:
	or %g1, %lo(0x000000ef), %g1

1:
	sethi %hi(0xd00d0000), %g1
