.section .text, "ax", @progbits

tmain:
	/* Do not branch do not annul */
	sethi %hi(0x80000000), %g1
	subcc %g1, 0x1, %g2
	bvc 1f
	sethi %hi(0xdeadbeef), %g1


	/* Do not branch does annul */
	addcc %g2, 0x1, %g2
	bvc,a 1f
	sethi %hi(0xcacacaca), %g1

	/* Do branch + delay slot */
	subcc %g0, 0x1fff, %g2
	bvc 2f
	or %g1, %lo(0x0000be00), %g1
2:
	or %g1, %lo(0x000000ef), %g1

1:
	sethi %hi(0xd00d0000), %g1
